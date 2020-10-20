#include "FFRTSPServer.h"
#include "common.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"
#include <assert.h>

#define LOG_FILTER	"FFRTSPServer"

CFFRTSPServer::CFFRTSPServer() : m_iPortCnt(0), m_pOFmtCtx(NULL)
{
	Init("FFRTSPServer");
}

CFFRTSPServer::CFFRTSPServer(const std::string &name) : m_iPortCnt(0), m_pOFmtCtx(NULL)
{
	Init(name);
}

CFFRTSPServer::~CFFRTSPServer()
{

}

void CFFRTSPServer::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);

	for(int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_iMediaInfo[i] = NULL;
	}


}

void CFFRTSPServer::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if(data.mKey == META_KEY_INPORT_COUNT)
	{
		m_iPortCnt = CUtil::convert<int, std::string>(data.mValue);		
	}
	for(int i = 0; i < m_iPortCnt; i++)
	{
		m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
	}


	m_desc.SetElementPortCount(m_iPortCnt, 0);
}

void CFFRTSPServer::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		{
			Open();
		}
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		{
			Close();
		}
		break;
	default:
		break;
	}
}

void CFFRTSPServer::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	AVStream* out_stream = m_pOFmtCtx->streams[m_iMediaInfo[id]->mStreamID];
	AVPacket pkt;
	av_new_packet(&pkt, buffer->GetDataSize());
	memcpy(pkt.data, buffer->GetData(), buffer->GetDataSize());
	pkt.stream_index = m_iMediaInfo[id]->mStreamID;
	pkt.pts = buffer->GetPts();
	pkt.dts = buffer->GetDts();

	if (av_interleaved_write_frame(m_pOFmtCtx, &pkt) < 0)
	{
		LOG_ERR("Error muxing packet!");
	}
	av_packet_unref(&pkt);
}

void CFFRTSPServer::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);

	if(key == META_KEY_MEDIA)
	{
		MediaType type = MediaStrToType(data.mValue);
		if(m_iMediaInfo[index] == NULL)
		{
			if(type == MEDIA_TYPE_VIDEO)
			{
				m_iMediaInfo[index] = new VMetaInfo();
			}
			else if(type == MEDIA_TYPE_AUDIO)
			{
				m_iMediaInfo[index] = new AMetaInfo();
			}
		}
	}
	else if (key == META_KEY_CODEC_ID)
	{
		if (m_iMediaInfo[index] != NULL)
		{
			m_iMediaInfo[index]->mCodecType = (MediaCodecType)CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_VIDEO_WIDTH)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mWidth = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_VIDEO_HEIGHT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mHeight = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_PIX_FORMAT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mPixfmt = FFPixFormat((VideoPixformat)CUtil::convert<int, std::string>(data.mValue));
		}
	}
	else if(key == META_KEY_VIDEO_FPS)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mfps = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_SAMPLE_RATE)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplerate = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_SAMPLE_FORMAT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_CHANNEL_NUM)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mChannel = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_FRAME_SAMPLE_COUNT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplecnt = CUtil::convert<int, std::string>(data.mValue);
		}
	}
}

void CFFRTSPServer::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		break;
	case MEDIA_MESSAGE_REPLY:
		break;
	case MEDIA_MESSAGE_NOTIFY:
		break;
	default:
		break;
	}
}


void CFFRTSPServer::AddStream(TRACKID index)
{
	AVStream *out_stream = NULL;
	out_stream = avformat_new_stream(m_pOFmtCtx, NULL);
	if (m_iMediaInfo[index]->mType == MEDIA_TYPE_VIDEO)
	{
		out_stream->time_base.num = 1;
		out_stream->time_base.den = ((VMetaInfo*)m_iMediaInfo[index])->mfps;
		out_stream->avg_frame_rate.num = ((VMetaInfo*)m_iMediaInfo[index])->mfps;
		out_stream->avg_frame_rate.den = 1;
		out_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		out_stream->codecpar->codec_id = FFCodecType(m_iMediaInfo[index]->mCodecType);
		out_stream->codecpar->width = ((VMetaInfo*)m_iMediaInfo[index])->mWidth;
		out_stream->codecpar->height = ((VMetaInfo*)m_iMediaInfo[index])->mHeight;
		out_stream->codecpar->format = 0;
	}
	else if (m_iMediaInfo[index]->mType == MEDIA_TYPE_AUDIO)
	{
		out_stream->time_base.num = 1;
		out_stream->time_base.den = ((AMetaInfo*)m_iMediaInfo[index])->mSamplerate;
		out_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
		AVCodecID codecID = FFCodecType(m_iMediaInfo[index]->mCodecType);
		if (m_iMediaInfo[index]->mCodecType == MediaCodec_PCM)
		{
			if (((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt == AUDIO_SAMPLE_FMT_U8)
			{
				codecID = AV_CODEC_ID_PCM_U8;
			}
			else if (((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt == AUDIO_SAMPLE_FMT_S16)
			{
				codecID = AV_CODEC_ID_PCM_S16LE;
			}
			else if (((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt == AUDIO_SAMPLE_FMT_S32)
			{
				codecID = AV_CODEC_ID_PCM_S32LE;
			}
		}
		m_iMediaInfo[index]->mStreamID = out_stream->index;

		out_stream->codecpar->codec_id = codecID;
		const AVCodecTag* tags = avformat_get_mov_audio_tags();
		out_stream->codecpar->codec_tag = av_codec_get_tag(&tags, out_stream->codecpar->codec_id);
		out_stream->codecpar->channels = ((AMetaInfo*)m_iMediaInfo[index])->mChannel;
		out_stream->codecpar->channel_layout = av_get_default_channel_layout(out_stream->codecpar->channels);
		out_stream->codecpar->sample_rate = ((AMetaInfo*)m_iMediaInfo[index])->mSamplerate;
		out_stream->codecpar->format = ((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt;
		out_stream->codecpar->frame_size = ((AMetaInfo*)m_iMediaInfo[index])->mSamplecnt;
	}

	MetaData data;
	if (m_inPorts[index]->GetProperty(META_KEY_CODEC_EXTRA_DATA, data) == MEDIA_ERR_NONE)
	{
		out_stream->codecpar->extradata_size = data.mValue.length();
		out_stream->codecpar->extradata = (uint8_t*)av_memdup(data.mValue.c_str(), data.mValue.length());
	}
}

int CFFRTSPServer::Open()
{
	int res = MEDIA_ERR_NONE;

	MetaData data;
	res = m_desc.GetMetaData(META_KEY_FILE_PATH, data);
	if (res != MEDIA_ERR_NONE)
	{
		LOG_ERR("Open error(%d)!", res);
		return res;
	}

	char errMsg[512] = { 0 };
	res = avformat_alloc_output_context2(&m_pOFmtCtx, NULL, "RTSP", data.mValue.c_str());
	if (res < 0)
	{
		av_strerror(res, errMsg, 512);
		LOG_ERR("Alloc output ctx error(%d), %s", res, errMsg);
		return res;
	}
	m_pOFmtCtx->oformat->flags |= AVFMT_GLOBALHEADER;
	m_pOFmtCtx->flags |= AVFMT_FLAG_NONBLOCK;

	//av_opt_set(m_pOFmtCtx->priv_data, "rtsp_transport", "tcp", 0);
	av_opt_set(m_pOFmtCtx->priv_data, "rtsp_flags", "listen", 0);
	av_opt_set(m_pOFmtCtx->priv_data, "timeout", "5000000", 0);

	for (int i = 0; i < m_iPortCnt; i++)
	{
		AddStream(i);
	}

	av_dump_format(m_pOFmtCtx, 0, m_pOFmtCtx->filename, 1);
	/*
	if (!(m_pOFmtCtx->flags & AVFMT_NOFILE)) 
	{
		res = avio_open(&m_pOFmtCtx->pb, m_pOFmtCtx->filename, AVIO_FLAG_WRITE);
		if (res < 0)
		{
			av_strerror(res, errMsg, 512);
			LOG_ERR("Could not open output URL '%s', err(%d):%s", m_pOFmtCtx->filename, res, errMsg);
			res = -1;
		}
	}
	*/
	res = avformat_write_header(m_pOFmtCtx, NULL);
	if (res < 0)
	{
		av_strerror(res, errMsg, 512);
		LOG_ERR("write header error(%d), %s", res, errMsg);
		res = -2;
	}


	return res;
}

void CFFRTSPServer::Close()
{
	if (m_pOFmtCtx)
	{
		av_write_trailer(m_pOFmtCtx);

		if (!(m_pOFmtCtx->flags & AVFMT_NOFILE))
		{
			avio_closep(&m_pOFmtCtx->pb);
		}
		avformat_free_context(m_pOFmtCtx);
		m_pOFmtCtx = NULL;
	}
	
}