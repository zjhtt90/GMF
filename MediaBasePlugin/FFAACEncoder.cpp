#include "FFAACEncoder.h"

#include "../Common/LogManager.h"
#include "../MediaCore/MediaBuffer.h"
#include "common.h"

#include <sstream>

#include <assert.h>

#include "libavutil/error.h"

#define LOG_FILTER	"FFAACEncoder"

short CFFAACEncoder::m_encoderID = 0;

CFFAACEncoder::CFFAACEncoder() : m_pCodecCtx(NULL), m_pCodec(NULL)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_encoderID;
	std::string name = "FFAACEncoder" + ss.str();	

	Init(name);
}

CFFAACEncoder::CFFAACEncoder(const std::string &name) : m_pCodecCtx(NULL), m_pCodec(NULL)
{
	Init(name);
}

CFFAACEncoder::~CFFAACEncoder()
{
	m_encoderID--;
	assert(m_encoderID >= 0);

	if(m_pCodecCtx != NULL)
	{
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
}

void CFFAACEncoder::Init(const std::string &name)
{
	m_encoderID++;

	m_codecID = LocalCodecType(AV_CODEC_ID_AAC);

	m_desc.SetElementName(name);

	m_pCodecCtx = avcodec_alloc_context3(NULL);
	if(m_pCodecCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
	}

}

int CFFAACEncoder::Open()
{
	m_pCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if(m_pCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return MEDIA_ERR_NOT_FOUND;
	}

	avcodec_get_context_defaults3(m_pCodecCtx, m_pCodec);

	m_pCodecCtx->codec_type = (AVMediaType)MEDIA_TYPE_AUDIO;

	// 设置编码器参数
	m_pCodecCtx->sample_fmt = (AVSampleFormat)m_sampleFmt; // 采样格式
	m_pCodecCtx->sample_rate = m_sampleRate; // 采样率
	m_pCodecCtx->channels = m_channel; // 声道个数
	m_pCodecCtx->channel_layout = av_get_default_channel_layout(m_channel); // 声道布局
	m_pCodecCtx->bit_rate = m_channel * av_get_bytes_per_sample((AVSampleFormat)m_sampleFmt) * m_sampleRate; // 码率

	if(m_pCodecCtx != NULL && m_pCodec != NULL)
	{
		int res = 0;
		res = avcodec_open2(m_pCodecCtx, m_pCodec, NULL);
		if(res != 0)
		{
			char reson[512] = {0};
			av_strerror(res, reson, 512);
			LOG_ERR("Open codec error: %d,,,%s", res, reson);
			return res;
		}

		//MediaMessage msg(MEDIA_MESSAGE_NOTIFY, MESSAGE_SUB_TYPE_CODEC_INFO, this, NULL);
		//msg.m_content.msgFileds.push_back();
	}

	return MEDIA_ERR_NONE;
}

void CFFAACEncoder::Close()
{
	if(m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		m_pCodec = NULL;
	}
}

void CFFAACEncoder::SetState(MediaElementState state)
{
	CAudioCodec::SetState(state);

	if (state == MEDIA_ELEMENT_STATE_READY)
	{
		GenerateSdp();
	}


}

void CFFAACEncoder::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer != NULL)
	{
		CMediaBuffer *dst = NULL;
		if(ProcessFrame(buffer, &dst) == MEDIA_ERR_NONE && dst != NULL)
		{
			m_outPorts[0]->PushBufferToDownStream(dst);
			delete dst;
		}
	}
}

int CFFAACEncoder::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

int CFFAACEncoder::ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf)
{
	AVFrame *frameBuf = NULL;
	AVPacket packet = {0};

	frameBuf = av_frame_alloc();

	frameBuf->nb_samples = m_pCodecCtx->frame_size; // 帧大小
	frameBuf->format = m_pCodecCtx->sample_fmt; // 采样格式
	frameBuf->channels = m_pCodecCtx->channels; // 声道数
	frameBuf->channel_layout = m_pCodecCtx->channel_layout; // 声道布局
	frameBuf->sample_rate = m_pCodecCtx->sample_rate; // 采样率
	// 填充frame中的data和linesize
	if(avcodec_fill_audio_frame(frameBuf, frameBuf->channels,
		static_cast<enum AVSampleFormat>(frameBuf->format),
		reinterpret_cast<const uint8_t*>(srcBuf->GetData()), srcBuf->GetDataSize(), 1) < 0) 
	{
			LOG_ERR("audio data size is wrong.");
			return MEDIA_ERR_INVALIDE_PARAME;
	}

	av_init_packet(&packet);

	// 编码
	int got_packet = 0;
	int ret = avcodec_encode_audio2(m_pCodecCtx, &packet, frameBuf, &got_packet);
	// 处理编码后的数据
	av_frame_free(&frameBuf);

	if (ret >=0) 
	{
		// 编码成功, 但不一定得数据包
		if(got_packet && packet.size > 0) 
		{
			*dstBuf = new CAudioBuffer(packet.data, packet.size, srcBuf->GetPts(), srcBuf->GetDts(), srcBuf->GetDuration(), OV_AAC);
			av_packet_unref(&packet);
		}
	}
	else 
	{
		LOG_ERR("AAC Encode failed.");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	return MEDIA_ERR_NONE;
}

bool CFFAACEncoder::GenerateSdp()
{
	AVCodec	*tmpCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (tmpCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return false;
	}

	AVCodecContext *tmpCtx = avcodec_alloc_context3(NULL);
	if (tmpCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
		return false;
	}

	tmpCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	tmpCtx->codec_type = (AVMediaType)MEDIA_TYPE_AUDIO;

	// 设置编码器参数
	tmpCtx->sample_fmt = (AVSampleFormat)m_sampleFmt; // 采样格式
	tmpCtx->sample_rate = m_sampleRate; // 采样率
	tmpCtx->channels = m_channel; // 声道个数
	tmpCtx->channel_layout = av_get_default_channel_layout(m_channel); // 声道布局
	tmpCtx->bit_rate = m_channel * av_get_bytes_per_sample((AVSampleFormat)m_sampleFmt) * m_sampleRate; // 码率

	if (tmpCtx != NULL && tmpCodec != NULL)
	{
		int res = 0;
		res = avcodec_open2(tmpCtx, tmpCodec, NULL);
		if (res != 0)
		{
			char reson[512] = { 0 };
			av_strerror(res, reson, 512);
			LOG_ERR("Open codec error: %d,,,%s", res, reson);
			return false;
		}
	}

	MetaData data(META_KEY_CODEC_EXTRA_DATA, std::string((char*)tmpCtx->extradata, tmpCtx->extradata_size), META_DATA_VAL_TYPE_PTR);
	m_outPorts[0]->SetProperty(data);

	avcodec_close(tmpCtx);
	avcodec_free_context(&tmpCtx);
	return true;
}

std::string CFFAACEncoder::GetAudioSdp(int trackid)
{
	int config = 0;
	switch(m_sampleRate) 
	{
	case 8000:
		if (m_channel== 1)
			config = 1588;
		else if (m_channel == 2)
			config = 1590;
		break;
	case 16000:
		if (m_channel == 1)
			config = 1408;
		else if (m_channel == 2)
			config = 1410;
		break;
	case 22050:
		if (m_channel == 1)
			config = 1388;
		else if (m_channel == 2)
			config = 1390;
		break;
	case 24000:
		if (m_channel == 1)
			config = 1308;
		else if (m_channel == 2)
			config = 1310;
		break;
	case 32000:
		if (m_channel == 1)
			config = 1288;
		else if (m_channel == 2)
			config = 1290;
		break;
	case 44100:
		if (m_channel == 1)
			config = 1208;
		else if (m_channel == 2)
			config = 1210;
		break;
	}
	std::ostringstream ostr;
	ostr<<"m=audio 0 RTP/AVP 96\n"
		<<"c=IN IP4 0.0.0.0\n"
		<<"b=AS:706\n"
		<<"a=rtpmap:96 MPEG4-GENERIC/"<<m_sampleRate<<"/"<<m_channel<<"\n"
		<<"a=fmtp:96 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config="<<config<<"\n"
		<<"a=control:track"<<trackid<<"\n";
	return ostr.str();
}

void CFFAACEncoder::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		{
			if(msg.m_subType == MESSAGE_SUB_TYPE_SDP)
			{
				MediaMessage msgRes(msg);
				msgRes.m_type = MEDIA_MESSAGE_REPLY;
				msgRes.m_src = this;
				msgRes.m_dst = msg.m_src;

				msgRes.m_content.msgFileds.push_back(MetaData("AudioSDP", GetAudioSdp(2), META_DATA_VAL_TYPE_STRING));

				if(m_bus != NULL)
				{
					m_bus->PushMessage(msgRes);
				}
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