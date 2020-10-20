#include "RTSPSink.h"
#include "../MediaCore/MediaBuffer.h"

#include "../Common/LogManager.h"
#include "../Common/Util.h"

extern "C"
{
#include "libavutil/common.h"
};


#define LOG_FILTER	"RTSPSink"


// 数据块头
#define CHUNK_HEADER_FOURCC MKTAG('C', 'H', 'U', 'H')
struct SChunkHeader{
	unsigned uFourCC;       // 识别标志 CHUNK_HEADER_FOURCC
	unsigned uHeaderSize;   // 帧头长度 sizeof(SChunkHeader)
	unsigned uMediaType;    // 帧类型
	unsigned uChunkCount;   // 流水序号
	unsigned uDataSize;     // 数据长度(不含头结构)
	unsigned uTimeStamp;    // 帧时间戳

	SChunkHeader() : uFourCC(CHUNK_HEADER_FOURCC), uHeaderSize(sizeof(struct SChunkHeader)),
		uMediaType(0), uChunkCount(0), uDataSize(0), uTimeStamp(0)
	{}
};



CRTSPSink::CRTSPSink()
{
	Init("RTSPSink");
}

CRTSPSink::CRTSPSink(const std::string &name)
{
	Init(name);
}

CRTSPSink::~CRTSPSink()
{
	UNINIT_NETWORK;
}

void CRTSPSink::Init(const std::string &name)
{
	m_channelCnt = 0;
	m_vCount = 0;
	m_aCount = 0;
	m_nDataType = 172;
	m_sdpMediaCnt = 0;

	m_sampleRate = 44100;

	for(int i=0; i<MEDIA_SEVICE_CHANNELS; i++) 
	{
			m_srcno[i] = -1;
	}

	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);

	INIT_NETWORK;

}

void CRTSPSink::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if(data.mKey == META_KEY_INPORT_COUNT)
	{
		int inPortCnt = CUtil::convert<int, std::string>(data.mValue);

		for(int i = 0; i < inPortCnt; i++)
		{
			m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
		}

		m_channelCnt = inPortCnt;
		m_desc.SetElementPortCount(inPortCnt, 0);
	}
}

void CRTSPSink::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{
			
			LOG_DEBUG("-----");
		}
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		Open();
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		Close();
		break;
	default:
		break;
	}
}

int CRTSPSink::Open()
{
	LOG_INFO("Start RTSPSink");
	MS_CFG ms_cfg;
	GetMediaServerCfg(ms_cfg);
	m_eMdieaServerAlive = xt_init_server(ms_cfg);
	if(m_eMdieaServerAlive) 
	{
		LOG_ERR("xt_init_server failed");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	CreateSrc();
	SetupHeartbeatCheck();

	SetSdp();

	// 设置丢包重传
	if(xt_update_resend_flag(m_cfg.is_resend) < 0)
	{
		LOG_WARN("Set resend to %d failed", m_cfg.is_resend);
	}

	return MEDIA_ERR_NONE;
}

void CRTSPSink::Close()
{
	LOG_INFO("Stop RTSPSink");
	for(int i=0; i<MEDIA_SEVICE_CHANNELS; i++) {
		if(m_srcno[i] >= 0)
			xt_destroy_src(m_srcno[i]);
		m_srcno[i] = -1;
	}
	xt_uninit_server();
}

void CRTSPSink::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	MediaType type = m_inPorts[id]->GetMediaType();
	if(type == MEDIA_TYPE_VIDEO)
	{
		CVideoBuffer *vBuffer = static_cast<CVideoBuffer*>(buffer);

		enum EFrameType frametype;
		switch(vBuffer->m_picType) 
		{
		case 'I': frametype = OV_VIDEO_I; break;
		case 'B': frametype = OV_VIDEO_B; break;
		case 'P': frametype = OV_VIDEO_P; break;
		default:  frametype = OV_VIDEO_I; break;
		}

		unsigned int len = sizeof(struct SChunkHeader) + buffer->GetDataSize();
		unsigned char *vBuf = new unsigned char[len]; 
		if(vBuf == NULL)
		{
			LOG_ERR("Can not alloc memory");
		}
		
		struct SChunkHeader *header = reinterpret_cast<struct SChunkHeader *>(vBuf);
		header->uFourCC = CHUNK_HEADER_FOURCC;
		header->uDataSize = sizeof(struct SChunkHeader);
		header->uTimeStamp = buffer->GetPts()/1000.0/1000.0*90000;
		header->uMediaType = frametype;
		header->uDataSize = buffer->GetDataSize();
		header->uChunkCount = m_vCount++;
		memcpy(vBuf+sizeof(struct SChunkHeader), buffer->GetData(), buffer->GetDataSize());	

		xt_send_data_in_stamp(m_srcno[0], 1, (char*)vBuf, len, frametype, m_nDataType, true, header->uTimeStamp);
		//xt_send_data(m_srcno[0], 1, (char*)vBuf, len, frametype, m_nDataType);
		if(vBuf != NULL)
		{
			delete vBuf;
			vBuf = NULL;
		}
	}
	else if(type == MEDIA_TYPE_AUDIO)
	{
		CAudioBuffer *aBuffer = static_cast<CAudioBuffer*>(buffer);

		unsigned int len = sizeof(struct SChunkHeader)+buffer->GetDataSize();
		unsigned char *aBuf = new unsigned char[len]; 
		if(aBuf == NULL)
		{
			LOG_ERR("Can not alloc memory");
		}
		struct SChunkHeader *header = reinterpret_cast<struct SChunkHeader *>(aBuf);
		header->uTimeStamp = buffer->GetPts()/1000.0/1000.0*m_sampleRate;
		header->uMediaType = aBuffer->m_type;
		header->uDataSize = buffer->GetDataSize();
		header->uChunkCount = m_aCount++;
		memcpy(aBuf+sizeof(struct SChunkHeader), buffer->GetData(), buffer->GetDataSize());	

		xt_send_data_in_stamp(m_srcno[0], 2, (char*)aBuf, len, aBuffer->m_type, m_nDataType, true, header->uTimeStamp);
		//xt_send_data(m_srcno[0], 2, (char*)aBuf, len, aBuffer->m_type, m_nDataType);
		if(aBuf != NULL)
		{
			delete aBuf;
			aBuf = NULL;
		}
	}
}

void CRTSPSink::GetMediaServerCfg(MS_CFG &ms_cfg)
{

	ms_cfg.num_chan = m_channelCnt;
	strcpy(ms_cfg.ip, "0.0.0.0");
	ms_cfg.snd_start_port = m_cfg.snd_port_start;
	ms_cfg.demux = m_cfg.is_demux;
	strcpy(ms_cfg.mul_start_ip, m_cfg.mul_ip_start.c_str());
	ms_cfg.msg_liten_port = m_cfg.msg_listen_port;
	ms_cfg.rtsp_listen_port = m_cfg.rtsp_listen_port;
	ms_cfg.tcp_listen_port = m_cfg.tcp_listen_port;
	ms_cfg.udp_listen_port = m_cfg.udp_listen_port;
	ms_cfg.snd_std_rtp = m_cfg.is_std_rtp;
	ms_cfg.sink_single = m_cfg.is_single_sink;
	ms_cfg.use_traffic_shaping = m_cfg.use_traffic_shaping;
	

	ms_cfg.xt_media_server_log_cb = &CRTSPSink::WriteMediaServerLog;
	ms_cfg.xt_link_state_event = &CRTSPSink::WriteLinkStateEvent;
	ms_cfg.rtsp_play_cb = &CRTSPSink::WriteRtspPlay;
	ms_cfg.rtsp_pause_cb = &CRTSPSink::WriteRtspPause;
	ms_cfg.tcp_play_cb = &CRTSPSink::WriteTcpPlay;
	ms_cfg.tcp_pause_cb = &CRTSPSink::WriteTcpPause;
	ms_cfg.rtcp_force_iframe_cb = &CRTSPSink::RtcpForceIframe;
}

void CRTSPSink::CreateSrc()
{
	struct src_track_info_t trackinfo = {0};
	trackinfo.tracknum = 2;
	trackinfo.trackids[0] = 1;
	trackinfo.trackids[1] = 2;
	char *trackname[8]={"video", "video", "audio" ,"video","video","audio"};
	xt_create_src(trackinfo.tracknum, trackinfo.trackids, trackname, m_srcno[0], 0);
	xt_create_src(trackinfo.tracknum, trackinfo.trackids, trackname, m_srcno[1], 1);
	trackinfo.tracknum = 1;
	xt_create_src(trackinfo.tracknum, trackinfo.trackids, trackname, m_srcno[2], 2);

#ifdef _USE_RTP_SEND_CONTROLLER
	// 注册网络变化回调事件
	int ret;
	for(int i=0; i<=1; i++) {
		ret = xt_register_network_changed_callback(m_srcno[i], 1, &CRTSPSink::NetworkChanged, &m_network_cost[i]);
		if(ret < 0)
			WriteLog("PcDvs", "register network changed failed srcno=%d, trackid=1", m_srcno[i]);
	}
#endif
}

void CRTSPSink::SetupHeartbeatCheck()
{
	unsigned int interval = 0;
	unsigned int timeout = 0;

	MetaData data;
	if(m_desc.GetMetaData(META_KEY_HEART_INTERVAL, data) == MEDIA_ERR_NONE)
	{
		interval = CUtil::convert<unsigned int, std::string>(data.mValue);
	}
	if(m_desc.GetMetaData(META_KEY_HEART_TIMEOUT, data) == MEDIA_ERR_NONE)
	{
		timeout = CUtil::convert<unsigned int, std::string>(data.mValue);
	}

	if(xt_ms_set_rtsp_heartbit_time(interval, timeout) < 0)
	{
		LOG_ERR("Setup heartbeat to [interval=%d, timeout=%d] failed", interval, timeout);
	}
}

void CRTSPSink::SetSdp()
{
	MediaMessage msg(MEDIA_MESSAGE_QUERY, MESSAGE_SUB_TYPE_SDP, this, NULL);

	if(m_bus != NULL)
	{
		m_bus->PushMessage(msg);
	}


	// 生成SDP信息
	std::ostringstream ostr;
	ostr.str("");
	ostr<<"v=0\n"
		<<"o=- "<<GetTickCount()<<" 1 IN IP4 "<<CUtil::GetHotAddr(CUtil::GetHostName())<<"\n"
		<<"s=RTSP/RTP stream from IPNC\n"
		<<"i=2?videoCodecType="<<"H264"<<"\n"
		<<"t=0 0\n"
		<<"a=tool:LIVE555 Streaming Media v2010.07.29\n"
		<<"a=type:broadcast\n"
		<<"a=control:*\n"
		<<"a=range:npt=0-\n"
		<<"a=x-qt-text-nam:RTSP/RTP stream from IPNC\n"
		<<"a=x-qt-text-inf:2?videoCodecType="<<"H264"<<"\n";
	m_baseSDP = ostr.str();


	unsigned int i = 0, o = 0;
	GetElementPortCount(i, o);
	while(m_sdpMediaCnt < i);

	std::string sdp = m_baseSDP + m_vSDP + m_aSDP;
	xt_set_key(m_srcno[0], (char*)sdp.c_str(), sdp.size(), m_nDataType);
	
}

void CRTSPSink::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	if(m_inPorts[index]->GetProperty(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
	{
		m_sampleRate = CUtil::convert<int, std::string>(data.mValue);
	}
}

void CRTSPSink::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		break;
	case MEDIA_MESSAGE_REPLY:
		{
			if(msg.m_subType == MESSAGE_SUB_TYPE_SDP)
			{
				MetaData data = msg.m_content.msgFileds[0];
				if(data.mKey == "VideoSDP")
				{
					m_vSDP += data.mValue;
				}
				else if(data.mKey == "AudioSDP")
				{
					m_aSDP += data.mValue;
				}
				m_sdpMediaCnt++;
			}
		}
		break;
	case MEDIA_MESSAGE_NOTIFY:
		break;
	default:
		break;
	}
}



// XTMediaServer回调处理
void MEDIASERVER_STDCALL CRTSPSink::WriteMediaServerLog(char* logname, log_level_type level,
													char* log_ctx, uint32_t log_ctx_len)
{

}

int MEDIASERVER_STDCALL CRTSPSink::WriteLinkStateEvent(const LINK_STATE_EVENT evnt, const int srno)
{
	return 0;
}

int MEDIASERVER_STDCALL CRTSPSink::WriteRtspPlay(int srcno, int trackid, long chid,  double npt,
											 float scale, uint32_t *rtp_pkt_seq,  uint32_t *rtp_pkt_timestamp)
{
	return 0;
}

int MEDIASERVER_STDCALL CRTSPSink::WriteRtspPause(int srcno, int trackid, long chid)
{
	return 0;
}

int MEDIASERVER_STDCALL CRTSPSink::WriteTcpPlay(int srcno, long chid, double npt, float scale,
											uint32_t *rtp_pkt_timestamp)
{
	return 0;
}

int MEDIASERVER_STDCALL CRTSPSink::WriteTcpPause(int srcno, long chid)
{
	return 0;
}

void MEDIASERVER_STDCALL CRTSPSink::RtcpForceIframe(const int srcno)
{

}

#ifdef _USE_RTP_SEND_CONTROLLER
void MEDIASERVER_STDCALL CRTSPSink::NetworkChanged(void *ctx, uint32_t bitrate,
											   uint32_t fraction_lost, uint32_t rtt)
{

}
#endif