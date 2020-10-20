#include "FFVideoDecoder.h"
#include "common.h"
#include "LogManager.h"
#include "MediaBuffer.h"
#include "Util.h"

extern "C"
{
#include "libavutil/imgutils.h"
#include "libavutil/mem.h"
};

#include <sstream>

#include <assert.h>

#define LOG_FILTER	"FFVideoDecoder"

short FFVideoDecoder::m_eleID = 0;

FFVideoDecoder::FFVideoDecoder() 
	: m_pCodecCtx(NULL), m_pCodec(NULL), m_dstFrameBuf(NULL), m_bGetInfo(false)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_eleID;
	std::string name = "FFVideoDecoder" + ss.str();	

	Init(name);
}

FFVideoDecoder::FFVideoDecoder(const std::string &name) 
	: m_pCodecCtx(NULL), m_pCodec(NULL), m_dstFrameBuf(NULL), m_bGetInfo(false)
{
	Init(name);
}

FFVideoDecoder::~FFVideoDecoder()
{
	m_eleID--;
	assert(m_eleID >= 0);

	if(m_pCodecCtx != NULL)
	{
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
}

void FFVideoDecoder::Init(const std::string &name)
{
	m_eleID++;

	m_codecID = AV_CODEC_ID_H264;

	m_desc.SetElementName(name);

	m_pCodecCtx = avcodec_alloc_context3(NULL);
	if(m_pCodecCtx == NULL)
	{
		LOG_ERR("Can not alloc contex!");
	}

}

int FFVideoDecoder::Open()
{
	LOG_INFO("Start FFVideoDecoder");
	m_pCodec = avcodec_find_decoder((AVCodecID)m_codecID);
	if(m_pCodec == NULL)
	{
		LOG_ERR("Can not find Codec!");
		return MEDIA_ERR_NOT_FOUND;
	}

	avcodec_get_context_defaults3(m_pCodecCtx, m_pCodec);

	m_pCodecCtx->codec_type = (AVMediaType)MEDIA_TYPE_VIDEO;

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

	CSThread::Start();

	return MEDIA_ERR_NONE;
}

void FFVideoDecoder::Close()
{
	LOG_INFO("Stop FFVideoDecoder");

	m_pauseEvent.post();

	CSThread::Kill();

	if (m_dstFrameBuf)
	{
		av_frame_free(&m_dstFrameBuf);
		m_dstFrameBuf = NULL;
	}

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


void FFVideoDecoder::SetState(MediaElementState state)
{
	CVideoCodec::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if (m_desc.GetMetaData(META_KEY_CODEC_TYPE, data) == MEDIA_ERR_NONE)
		{
			if (data.mValue == "h264")
			{
				m_codecID = AV_CODEC_ID_H264;
			}
			else if (data.mValue == "h265")
			{
				m_codecID = AV_CODEC_ID_H265;
			}
			else if (data.mValue == "mpeg4")
			{
				m_codecID = AV_CODEC_ID_MPEG4;
			}
		}
	}
}

void FFVideoDecoder::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
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

int FFVideoDecoder::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

int FFVideoDecoder::ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf)
{
	int ret = MEDIA_ERR_NONE;
	AVPacket packet = {0};

	av_init_packet(&packet);

	av_packet_from_data(&packet, srcBuf->GetData(), srcBuf->GetDataSize());
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
					LOG_DEBUG("parse video info,w(%d),h(%d),fmt(%d)", m_dstFrameBuf->width, m_dstFrameBuf->height, m_dstFrameBuf->format);
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_VIDEO_WIDTH, 
						CUtil::convert<std::string, int>(m_dstFrameBuf->width), META_DATA_VAL_TYPE_INT));
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_VIDEO_HEIGHT,
						CUtil::convert<std::string, int>(m_dstFrameBuf->height), META_DATA_VAL_TYPE_INT));
					m_outPorts[0]->UpdateProperty(MetaData(META_KEY_PIX_FORMAT,
						CUtil::convert<std::string, int>(LocalPixFormat((AVPixelFormat)m_dstFrameBuf->format)), META_DATA_VAL_TYPE_INT));

					CMediaElement *pNext = m_nextElement;
					while (pNext)
					{
						pNext->SetState(MEDIA_ELEMENT_STATE_STOP);
						pNext->SetState(MEDIA_ELEMENT_STATE_OPEN);
						pNext = pNext->m_nextElement;
					}

					m_bGetInfo = true;
				}

				unsigned char* pTemp = NULL;
				int len = av_image_get_buffer_size((AVPixelFormat)m_dstFrameBuf->format, m_dstFrameBuf->width, m_dstFrameBuf->height, 1);
				pTemp = new unsigned char[len];
				av_image_copy_to_buffer(pTemp, len, m_dstFrameBuf->data, m_dstFrameBuf->linesize, (AVPixelFormat)m_dstFrameBuf->format, m_dstFrameBuf->width, m_dstFrameBuf->height, 1);
				*dstBuf = new CVideoBuffer(pTemp, len, srcBuf->GetPts(), srcBuf->GetDts(),
					srcBuf->GetDuration(), av_get_picture_type_char(m_dstFrameBuf->pict_type));
				delete pTemp;

				av_frame_unref(m_dstFrameBuf);
			}
		}
	}
	return ret;
}


void FFVideoDecoder::Run()
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
