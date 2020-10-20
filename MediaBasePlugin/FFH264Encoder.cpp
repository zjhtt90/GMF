#include "FFH264Encoder.h"
#include "common.h"
#include "../Common/LogManager.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"
#include "libavutil/base64.h"
};

#include <sstream>

#include <assert.h>

#define LOG_FILTER	"FFH264Encoder"

short CFFH264Encoder::m_eleID = 0;

CFFH264Encoder::CFFH264Encoder() : m_pCodecCtx(NULL), m_pCodec(NULL),
	m_qmin(0), m_qmax(0), m_preset(Fast), m_profile(BaselineProfile), m_IFrameInterval(30)
	, m_bForceKey(false)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_eleID;
	std::string name = "FFH264Encoder" + ss.str();	

	Init(name);
}

CFFH264Encoder::CFFH264Encoder(const std::string &name) : m_pCodecCtx(NULL), m_pCodec(NULL),
	m_qmin(0), m_qmax(0), m_preset(Fast), m_profile(BaselineProfile), m_IFrameInterval(30)
	, m_bForceKey(false)
{
	Init(name);
}

CFFH264Encoder::~CFFH264Encoder()
{
	m_eleID--;
	assert(m_eleID >= 0);

	if(m_pCodecCtx != NULL)
	{
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
}

void CFFH264Encoder::Init(const std::string &name)
{
	m_eleID++;

	m_codecID = LocalCodecType(AV_CODEC_ID_H264);

	m_desc.SetElementName(name);

	m_pCodecCtx = avcodec_alloc_context3(NULL);
	if(m_pCodecCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
	}

}

void CFFH264Encoder::SetConfig(struct BitrateControl bitrate, int qmin, int qmax, Preset preset, Profile profile, int interval)
{
	m_bitrate = bitrate;
	m_qmin = qmin;
	m_qmax = qmax;
	m_preset = preset;
	m_profile = profile;
	m_IFrameInterval = interval;
}

void CFFH264Encoder::InitParam(AVCodecContext *ctx)
{
	//��ʼ�����������Ĳ���
	ctx->bit_rate = m_bitrate.cur_bitrate * 1000; // ������
	ctx->rc_max_rate = m_bitrate.max_bitrate * 1000; // ������
	ctx->rc_buffer_size = m_bitrate.max_bitrate * 1000;
	ctx->rc_min_rate = m_bitrate.min_bitrate * 1000; // ������

	ctx->width = m_width;
	ctx->height = m_height;
	ctx->qmax = m_qmax;
	ctx->qmin = m_qmin;
	ctx->time_base.num = 1;
	ctx->time_base.den = m_dstFps; // ֡��/֡����ʱ��/֡����ʱ��
	ctx->gop_size = m_IFrameInterval; // emit one intra frame every gop_size frames
	ctx->max_b_frames = 0; // ��ֹB֡
	ctx->slices = 1; // ���д����Ƭ��
	m_pixformat = FFPixFormat((VideoPixformat)m_pixformat);
	ctx->pix_fmt = (AVPixelFormat)m_pixformat;


	// ���ñ���������˽��ѡ��
	if(!m_licPath.empty()) 
	{
		if(av_opt_set(ctx->priv_data, "lic_path", m_licPath.c_str(), 0) < 0)
			LOG_WARN("set option lic_path failed");
	}

	if(m_bitrate.type == BitrateControl::CQP && m_bitrate.quality > 0)
	{
		if(av_opt_set(ctx->priv_data, "qp", CUtil::convert<std::string, unsigned int>(m_bitrate.quality).c_str(), 0) < 0)
			LOG_WARN("set option qp failed");
	}

	// ����Ԥ��ֵ
	if(m_preset == Fast) 
	{
		if(av_opt_set(ctx->priv_data, "preset", "fast", 0) < 0)
		{
			LOG_WARN("set option preset=fast failed");
		}
	}
	else if (m_preset == Medium) 
	{
		if(av_opt_set(ctx->priv_data, "preset", "superfast", 0) < 0)
		{
			LOG_WARN("set option preset=superfast failed");
		}
	}
	else if (m_preset == UltraFast) 
	{
		if(av_opt_set(ctx->priv_data, "preset", "ultrafast", 0) < 0)
		{
			LOG_WARN("set option preset=ultrafast failed");
		}
	}
	else
	{
		LOG_WARN("invalid preset value: %d" ,m_preset);
	}

	if(av_opt_set(ctx->priv_data, "tune", "zerolatency", 0) < 0)
	{
		LOG_WARN("set option tune failed");
	}

	// ����profile
	if(m_profile == BaselineProfile) 
	{
		if(av_opt_set(ctx->priv_data, "profile", "baseline", 0) < 0)
		{
			LOG_WARN("set option profile=baseline failed");
		}
	}
	else if(m_profile == MainProfile) 
	{
		if(av_opt_set(ctx->priv_data, "profile", "main", 0) < 0)
		{
			LOG_WARN("set option profile=main failed");
		}
	}
	else 
	{
		if (m_pixformat == AV_PIX_FMT_YUV444P) 
		{
			if(av_opt_set(ctx->priv_data, "profile", "high444", 0) < 0)
				LOG_WARN("set option profile=high44 failed");
		}
		else if (m_pixformat == AV_PIX_FMT_YUV422P) 
		{
			if(av_opt_set(ctx->priv_data, "profile", "high422", 0) < 0)
				LOG_WARN("set option profile=high422 failed");
		}
		else 
		{
			if(av_opt_set(ctx->priv_data, "profile", "high", 0) < 0)
				LOG_WARN("set option profile=high failed");
		}
	}

}

int CFFH264Encoder::Open()
{
	LOG_INFO("Start H264Encoder");
	m_pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if(m_pCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return MEDIA_ERR_NOT_FOUND;
	}

	avcodec_get_context_defaults3(m_pCodecCtx, m_pCodec);

	m_pCodecCtx->codec_type = (AVMediaType)MEDIA_TYPE_VIDEO;

	InitParam(m_pCodecCtx);

	if(m_pCodecCtx != NULL && m_pCodec != NULL)
	{
		int res = 0;
		res = avcodec_open2(m_pCodecCtx, m_pCodec, NULL);
		if(res != 0)
		{
			LOG_ERR("Open codec error: %d", res);
			return res;
		}
	}

	CSThread::Start();

	return MEDIA_ERR_NONE;
}

void CFFH264Encoder::Close()
{
	LOG_INFO("Stop H264Encoder");

	m_pauseEvent.post();

	CSThread::Kill();

	m_bufLock.Lock();
	while(!m_inBufs.empty())
	{
		CMediaBuffer *buf = m_inBufs.front();
		delete buf;
		m_inBufs.pop();
	}
	m_bufLock.UnLock();

	if(m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		m_pCodec = NULL;
	}
}


void CFFH264Encoder::SetState(MediaElementState state)
{
	CVideoCodec::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if(m_desc.GetMetaData(META_KEY_MIN_QVALUE, data) == MEDIA_ERR_NONE)
		{
			m_qmin = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_MAX_QVALUE, data) == MEDIA_ERR_NONE)
		{
			m_qmax = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_CODEC_PRESET, data) == MEDIA_ERR_NONE)
		{
			m_preset = (Preset)CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_CODEC_PROFILE, data) == MEDIA_ERR_NONE)
		{
			m_profile = (Profile)CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_I_FRAME_INTERVAL, data) == MEDIA_ERR_NONE)
		{
			m_IFrameInterval = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_BITRATE_TYPE, data) == MEDIA_ERR_NONE)
		{
			m_bitrate.type = (BitrateControl::BitrateType)CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_CURRENT_BITRATE, data) == MEDIA_ERR_NONE)
		{
			m_bitrate.cur_bitrate = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_MIN_BITRATE, data) == MEDIA_ERR_NONE)
		{
			m_bitrate.min_bitrate = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_MAX_BITRATE, data) == MEDIA_ERR_NONE)
		{
			m_bitrate.max_bitrate = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_CODEC_QUALITY, data) == MEDIA_ERR_NONE)
		{
			m_bitrate.quality = CUtil::convert<unsigned int, std::string>(data.mValue);
		}


		GenerateSdp();
	}

	
}

void CFFH264Encoder::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	CMediaBuffer *buf = new CMediaBuffer(*buffer);
	m_bufLock.Lock();
	m_inBufs.push(buf);
	//LOG_DEBUG("In Buf Size: %d", m_inBufs.size());
	m_bufLock.UnLock();

	if(m_curState == MEDIA_ELEMENT_STATE_RUNNING && m_inBufs.size() == 1)
	{
		m_pauseEvent.post();
	}
}

int CFFH264Encoder::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

int CFFH264Encoder::ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf)
{
	AVFrame *frameBuf = NULL;
	AVPacket packet = {0};

	frameBuf = av_frame_alloc();

	frameBuf->format = m_pixformat;
	frameBuf->width  = m_width;
	frameBuf->height = m_height;
	frameBuf->pkt_pts = srcBuf->GetPts();
	frameBuf->pkt_dts = srcBuf->GetDts();
	frameBuf->pkt_duration = srcBuf->GetDuration();
	if (m_bForceKey)
	{
		LOG_INFO("force key frame!");
		frameBuf->pict_type = AV_PICTURE_TYPE_I;
		m_bForceKey = false;
	}
	else
	{
		frameBuf->pict_type = AV_PICTURE_TYPE_NONE;
	}
	
	unsigned char *tmp = (unsigned char*)av_memdup(srcBuf->GetData(), srcBuf->GetDataSize());
	av_image_fill_arrays(frameBuf->data, frameBuf->linesize,
		tmp, (AVPixelFormat)m_pixformat, m_width, m_height, 1);

	//av_new_packet(&packet, );
	av_init_packet(&packet);

	// ����
	int got_packet = 0;
	int ret = avcodec_encode_video2(m_pCodecCtx, &packet, frameBuf, &got_packet);
	// �������������
	av_free(tmp);
	av_frame_free(&frameBuf);

	if (ret >=0) 
	{
		// ����ɹ�, ����һ�������ݰ�
		if(got_packet && packet.size > 0) 
		{
			*dstBuf = new CVideoBuffer(packet.data, packet.size, srcBuf->GetPts(), srcBuf->GetDts(), 
				srcBuf->GetDuration(), av_get_picture_type_char(m_pCodecCtx->coded_frame->pict_type));
			av_packet_unref(&packet);
		}
	}
	else 
	{
		LOG_ERR("H264 Encode failed.");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	return MEDIA_ERR_NONE;
}


void CFFH264Encoder::Run()
{
	while(1)
	{
		//LOG_INFO("current state %d", m_curState);
		if(m_curState == MEDIA_ELEMENT_STATE_RUNNING)
		{
			CMediaBuffer *inBuf = NULL;
			CMediaBuffer *outBuf = NULL;
			m_bufLock.Lock();
			if(!m_inBufs.empty())
			{
				inBuf = m_inBufs.front();
				m_inBufs.pop();
			}
			m_bufLock.UnLock();

			if(inBuf != NULL)
			{
				ProcessFrame(inBuf, &outBuf);
				delete inBuf;
				if(outBuf != NULL)
				{
					m_outPorts[0]->PushBufferToDownStream(outBuf);
					delete outBuf;
				}

				CSThread::Sleep(1);
			}
			else
			{
				//LOG_INFO("No buffer, Will Pause");
				m_pauseEvent.wait();		
			}
		}
		else if(m_curState == MEDIA_ELEMENT_STATE_PAUSED)
		{
			m_pauseEvent.wait();
		}
		else if(m_curState == MEDIA_ELEMENT_STATE_STOPPED)
		{
			break;
		}
	}
}


void CFFH264Encoder::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		{
			MediaMessage msgRes(msg);
			msgRes.m_type = MEDIA_MESSAGE_REPLY;
			msgRes.m_src = this;
			msgRes.m_dst = msg.m_src;

			if(msg.m_subType == MESSAGE_SUB_TYPE_SDP)
			{
				std::ostringstream ostr;
				ostr<<"m=video 0 RTP/AVP 96\n"
					<<"c=IN IP4 0.0.0.0\n"
					<<"b=AS:12000\n"
					<<"a=rtpmap:96 "<<"H264"<<"/90000\n"
					<<"a=fmtp:96 packetization-mode=1;"<<m_sdpSPS<<"\n"
					<<"a=control:track"<<1<<"\n";

				msgRes.m_content.msgFileds.push_back(MetaData("VideoSDP", ostr.str(), META_DATA_VAL_TYPE_STRING));	
			}
			else if(msg.m_subType == MESSAGE_SUB_TYPE_CODEC_INFO)
			{
				ExportCodecInfo(msgRes.m_content.msgFileds);
			}
			else if (msg.m_subType == MESSAGE_SUB_TYPE_FORCE_KEY)
			{
				m_bForceKey = true;
			}

			if(m_bus != NULL)
			{
				m_bus->PushMessage(msgRes);
			}
		}
		break;
	case MEDIA_MESSAGE_REPLY:
		break;
	case MEDIA_MESSAGE_NOTIFY:
		break;
	default:
		break;
	}
}

bool CFFH264Encoder::GenerateSdp()
{
	AVCodec	*tmpCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if(tmpCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return false;
	}

	AVCodecContext *tmpCtx = avcodec_alloc_context3(NULL);
	if(tmpCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
		return false;
	}

	// ǿ��x264���ɹ�����SDPͷ��Ϣ��extra data��
	tmpCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;


	InitParam(tmpCtx);

	if(tmpCtx != NULL && tmpCodec != NULL)
	{
		int res = 0;
		res = avcodec_open2(tmpCtx, tmpCodec, NULL);
		if(res != 0)
		{
			LOG_ERR("Open codec error: %d", res);
			return false;
		}
	}

	MetaData data(META_KEY_CODEC_EXTRA_DATA, std::string((char*)tmpCtx->extradata, tmpCtx->extradata_size), META_DATA_VAL_TYPE_PTR);
	m_outPorts[0]->SetProperty(data);

	// ��extra dataת����SDP
	char *str = ExtraDataToPsets(tmpCtx);
	if(str) 
	{
		m_sdpSPS = str;
		//m_sdpSPS = "sprop-parameter-sets=Z0LAFtoCgPaEAAADAAQAAAMAUjxYuoA=,aM4NyA==; profile-level-id=42C016";
		free(str);
		LOG_INFO("SDP=%s", m_sdpSPS.c_str());
	}
	else
	{
		LOG_WARN("Can not get SDP from libx264");
	}

	avcodec_close(tmpCtx);
	avcodec_free_context(&tmpCtx);
	return str != NULL;
}



char* CFFH264Encoder::ExtraDataToPsets(AVCodecContext *c)
{
	char *psets, *p;
	static const char pset_string[] = "sprop-parameter-sets=";
	static const char profile_string[] = "; profile-level-id=";
	uint8_t *extradata = c->extradata;
	int extradata_size = c->extradata_size;

	if (extradata_size > MAX_EXTRADATA_SIZE) 
	{
		LOG_ERR("AVCodecContext's extradata size(%d) is too large ", extradata_size);
		return NULL;
	}

	psets = static_cast<char *>(malloc(MAX_PSET_SIZE));
	if (!psets) 
	{
		LOG_ERR("Can not malloc size: %d", MAX_PSET_SIZE);
		return NULL;
	}
	memset(psets, 0, MAX_PSET_SIZE);
	memcpy(psets, pset_string, strlen(pset_string));
	p = psets + strlen(pset_string);
	bool firstp = true;

	const uint8_t *sps = NULL, *sps_end;
	const uint8_t *r = ff_avc_find_startcode(extradata, extradata + extradata_size);
	while (r < extradata + extradata_size) 
	{
		const uint8_t *r1;
		uint8_t nal_type;

		while (!*(r++)); // ����ǰ���3�ֽڻ�4�ֽڵ���ʼ��
		nal_type = *r & 0x1f; // ȡ��NAL unit����
		r1 = ff_avc_find_startcode(r, extradata + extradata_size);
		// ֻ���SPS��PPS����
		if (nal_type != NAL_SPS && nal_type != NAL_PPS) {
			r = r1;
			continue;
		}
		// ������ǵ�һ�����,��Ӷ��ŷָ�
		if(!firstp) 
		{
			*p = ',';
			p++;
			firstp = false;
		}
		// ����SPS������λ��
		if (nal_type == NAL_SPS) 
		{
			sps = r;
			sps_end = r1;
		}
		// ������ת����base64����
		if (!av_base64_encode(p, MAX_PSET_SIZE - (p - psets), r, r1 - r)) 
		{
			free(psets);
			return NULL;
		}
		p += strlen(p);
		r = r1;
	}
	if (sps && sps_end - sps >= 4) 
	{
		memcpy(p, profile_string, strlen(profile_string));
		p += strlen(p);
		// дprofile_idc, constraint_set��level_idc
		ff_data_to_hex(p, sps + 1, 3, 0);
		p[6] = '\0';
	}

	return psets;
}

// ����x264 NAL payload��ʼ��,���δ����x264��b_annexb����,����ʼ��Ϊ4�ֽڵ�payload��С,
// ����Ϊ0x000001,����Ϊ0x00000001, libx264��b_annexb����Ĭ��������.
// pΪx264 NAL payload����
// �ú�������ffmpeg��
const uint8_t *CFFH264Encoder::ff_avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
	// Ϊ���Ż�,��pת����4�ֽڵ��������бȽ�,���p����ƫ�Ƶ�4�ֽڶ���߽�
	// ���p��ַ����4�ֽڶ����,���ȱȽϼ���,ֱ��pƫ�Ƶ�4�ֽڱ߽�
	const uint8_t *a = p + 4 - (reinterpret_cast<intptr_t>(p) & 3);

	// ��ǰһ�αȽ�3���ַ�,ֱ��pƫ�Ƶ�4�ֽڱ߽�
	for (end -= 3; p < a && p < end; p++) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}

	// ��ǰһ�αȽ�6���ַ�,����ѭ������, ����4���ֽ�
	for (end -= 3; p < end; p += 4) {
		uint32_t x = *reinterpret_cast<const uint32_t*>(p);
		//      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
		//      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
		if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
			if (p[1] == 0) {
				if (p[0] == 0 && p[2] == 1)
					return p;
				if (p[2] == 0 && p[3] == 1)
					return p+1;
			}
			if (p[3] == 0) {
				if (p[2] == 0 && p[4] == 1)
					return p+2;
				if (p[4] == 0 && p[5] == 1)
					return p+3;
			}
		}
	}

	// �����δƥ��,�����buffer����6�ֽ�, �����3�ֽ�,���ֽڱȽ�
	for (end += 3; p < end; p++) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}

	return end + 3; // ����ʧ��,���ؽ�����ַ
}

// �ú�������ffmpeg��
const uint8_t *CFFH264Encoder::ff_avc_find_startcode(const uint8_t *p, const uint8_t *end)
{
	const uint8_t *out= ff_avc_find_startcode_internal(p, end);
	// �����ʼ����0x00 00 00 01,������0x00 00 01, �����1�ֽ�
	if(p<out && out<end && !out[-1]) out--;
	return out;
}

// �ú�������ffmpeg��
char *CFFH264Encoder::ff_data_to_hex(char *buff, const uint8_t *src, int s, int lowercase)
{
	int i;
	static const char hex_table_uc[16] = { '0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F' };
	static const char hex_table_lc[16] = { '0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'a', 'b',
		'c', 'd', 'e', 'f' };
	const char *hex_table = lowercase ? hex_table_lc : hex_table_uc;

	for (i = 0; i < s; i++) {
		buff[i * 2]     = hex_table[src[i] >> 4];
		buff[i * 2 + 1] = hex_table[src[i] & 0xF];
	}

	return buff;
}