#include "FFAudioDecoder.h"
#include "common.h"
#include "LogManager.h"
#include "MediaBuffer.h"
#include "Util.h"

extern "C"
{
#include "libavutil/samplefmt.h"
#include "libavutil/mem.h"
};

#include <sstream>

#include <assert.h>

#define LOG_FILTER	"FFAudioDecoder"

short FFAudioDecoder::m_eleID = 0;

FFAudioDecoder::FFAudioDecoder()
	: m_pCodecCtx(NULL), m_pCodec(NULL), m_dstFrameBuf(NULL), m_bGetInfo(false)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_eleID;
	std::string name = "FFAudioDecoder" + ss.str();	

	Init(name);
}

FFAudioDecoder::FFAudioDecoder(const std::string &name)
	: m_pCodecCtx(NULL), m_pCodec(NULL), m_dstFrameBuf(NULL), m_bGetInfo(false)
{
	Init(name);
}

FFAudioDecoder::~FFAudioDecoder()
{
	m_eleID--;
	assert(m_eleID >= 0);

	if(m_pCodecCtx != NULL)
	{
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
}

void FFAudioDecoder::Init(const std::string &name)
{
	m_eleID++;

	m_desc.SetElementName(name);

	m_pCodecCtx = avcodec_alloc_context3(NULL);
	if(m_pCodecCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
	}

}

int FFAudioDecoder::Open()
{
	LOG_INFO("Start FFAudioDecoder");
	m_pCodec = avcodec_find_decoder((AVCodecID)m_codecID);
	if(m_pCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return MEDIA_ERR_NOT_FOUND;
	}

	avcodec_get_context_defaults3(m_pCodecCtx, m_pCodec);

	m_pCodecCtx->codec_type = (AVMediaType)MEDIA_TYPE_AUDIO;

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

	m_dstFrameBuf = av_frame_alloc();

	return MEDIA_ERR_NONE;
}

void FFAudioDecoder::Close()
{
	LOG_INFO("Stop FFVideoDecoder");

	if (m_dstFrameBuf)
	{
		av_frame_free(&m_dstFrameBuf);
		m_dstFrameBuf = NULL;
	}

	if(m_pCodecCtx)
	{
		avcodec_close(m_pCodecCtx);
		m_pCodec = NULL;
	}
}


void FFAudioDecoder::SetState(MediaElementState state)
{
	CAudioCodec::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if (m_desc.GetMetaData(META_KEY_CODEC_TYPE, data) == MEDIA_ERR_NONE)
		{
			if (data.mValue == "pcma")
			{
				m_codecID = AV_CODEC_ID_PCM_ALAW;
			}
			else if (data.mValue == "pcmu")
			{
				m_codecID = AV_CODEC_ID_PCM_MULAW;
			}
			else if (data.mValue == "aac")
			{
				m_codecID = AV_CODEC_ID_AAC;
			}
			else if (data.mValue == "mp3")
			{
				m_codecID = AV_CODEC_ID_MP3;
			}
		}
	}
}

void FFAudioDecoder::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	int ret = MEDIA_ERR_NONE;
	AVPacket packet = { 0 };

	av_init_packet(&packet);

	av_packet_from_data(&packet, buffer->GetData(), buffer->GetDataSize());
	ret = avcodec_send_packet(m_pCodecCtx, &packet);
	if (ret == 0)
	{
		while (ret >= 0)
		{
			ret = avcodec_receive_frame(m_pCodecCtx, m_dstFrameBuf);
			if (ret < 0)
			{
				//LOG_ERR("Error during decoding\n");
				continue;
			}
			else
			{
				if (!m_bGetInfo)
				{
					LOG_DEBUG("parse audio info,samplerate(%d),channel(%d),fmt(%d)",
						m_dstFrameBuf->sample_rate, m_dstFrameBuf->channels, m_dstFrameBuf->format);
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_SAMPLE_RATE,
						CUtil::convert<std::string, int>(m_dstFrameBuf->sample_rate), META_DATA_VAL_TYPE_INT));
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_CHANNEL_NUM,
						CUtil::convert<std::string, int>(m_dstFrameBuf->channels), META_DATA_VAL_TYPE_INT));
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_SAMPLE_FORMAT,
						CUtil::convert<std::string, int>(m_dstFrameBuf->format), META_DATA_VAL_TYPE_INT));

					CMediaElement *pNext = m_nextElement;
					while (pNext)
					{
						pNext->SetState(MEDIA_ELEMENT_STATE_STOP);
						pNext->SetState(MEDIA_ELEMENT_STATE_OPEN);
						pNext = pNext->m_nextElement;
					}

					m_bGetInfo = true;
				}

				CMediaBuffer *dstBuf = new CVideoBuffer(m_dstFrameBuf->data[0], m_dstFrameBuf->linesize[0], buffer->GetPts(), buffer->GetDts(),
					buffer->GetDuration(), av_get_picture_type_char(m_dstFrameBuf->pict_type));
				m_outPorts[0]->PushBufferToDownStream(dstBuf);
				delete dstBuf;

				av_frame_unref(m_dstFrameBuf);
			}
		}
	}
}

int FFAudioDecoder::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}
