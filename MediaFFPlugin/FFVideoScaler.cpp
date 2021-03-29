#include "FFVideoScaler.h"
#include "common.h"
#include "MediaBuffer.h"
#include "Util.h"
#include "LogManager.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

#define LOG_FILTER	"FFVideoScaler"

CFFVideoScaler::CFFVideoScaler() : m_pSwsCtx(NULL)
{
	Init("FFVideoScaler");
}

CFFVideoScaler::CFFVideoScaler(const std::string &name) : m_pSwsCtx(NULL)
{
	Init(name);
}

CFFVideoScaler::~CFFVideoScaler()
{
}

void CFFVideoScaler::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_desc.SetElementPortCount(1, 1);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);
}

int CFFVideoScaler::Open()
{
	LOG_INFO("Start VideoScaler");
	if (m_dstWidth == m_srcWidth && m_dstHeight == m_srcHeight && m_dstPixFmt == m_srcPixFmt)
	{
		LOG_WARN("Don`t need scaler");
		return MEDIA_ERR_NONE;
	}
	m_pSwsCtx = sws_getCachedContext(m_pSwsCtx,m_srcWidth, m_srcHeight, 
		(AVPixelFormat)m_srcPixFmt,
		m_dstWidth, m_dstHeight,
		(AVPixelFormat)m_dstPixFmt, SWS_BILINEAR, NULL, NULL, NULL);

	if(m_pSwsCtx == NULL)
	{
		LOG_ERR("Create convert faild!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	return MEDIA_ERR_NONE;
}

void CFFVideoScaler::Close()
{
	LOG_INFO("Stop VideoScaler");
	if(m_pSwsCtx)
	{
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = NULL;
	}
}

void CFFVideoScaler::SetState(MediaElementState state)
{
	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
		{
			m_dstWidth = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstWidth = m_srcWidth;
			m_inPorts[0]->GetProperty(META_KEY_VIDEO_WIDTH, data);
			m_outPorts[0]->SetProperty(data);
		}
		//m_dstWidth = ALIGN(m_dstWidth,ALIGN_BY);

		if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
		{
			m_dstHeight = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstHeight = m_srcHeight;
			m_inPorts[0]->GetProperty(META_KEY_VIDEO_HEIGHT, data);
			m_outPorts[0]->SetProperty(data);
		}
		//m_dstHeight = ALIGN(m_dstHeight,ALIGN_BY);

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_dstPixFmt = CUtil::convert<int, std::string>(data.mValue);
			m_dstPixFmt = FFPixFormat((VideoPixformat)m_dstPixFmt);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstPixFmt = m_srcPixFmt;
			m_inPorts[0]->GetProperty(META_KEY_PIX_FORMAT, data);
			m_outPorts[0]->SetProperty(data);
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		Open();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		Close();
	}
}

void CFFVideoScaler::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if (m_dstWidth == m_srcWidth && m_dstHeight == m_srcHeight && m_dstPixFmt == m_srcPixFmt)
	{
		m_outPorts[0]->PushBufferToDownStream(buffer);
	}
	else
	{
		AVPicture srcPic = { 0x00 }, dstPic = { 0x00 };
		if (m_pSwsCtx == NULL)
		{
			return;
		}

		avpicture_fill(&srcPic, buffer->GetData(), (AVPixelFormat)m_srcPixFmt, m_srcWidth, m_srcHeight);

		avpicture_alloc(&dstPic, (AVPixelFormat)m_dstPixFmt, m_dstWidth, m_dstHeight);

		sws_scale(m_pSwsCtx, srcPic.data, srcPic.linesize, 0, m_srcHeight,
			dstPic.data, dstPic.linesize);


		int size = avpicture_get_size((AVPixelFormat)m_dstPixFmt, m_dstWidth, m_dstHeight);
		unsigned char *data = new unsigned char[size];
		avpicture_layout(&dstPic, (AVPixelFormat)m_dstPixFmt, m_dstWidth, m_dstHeight, data, size);
		avpicture_free(&dstPic);

		CMediaBuffer *sBuf = new CMediaBuffer(data, size, buffer->GetPts(), buffer->GetDts(), buffer->GetDuration());
		delete data;

		m_outPorts[0]->PushBufferToDownStream(sBuf);
		delete sBuf;
	}
}

int CFFVideoScaler::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

void CFFVideoScaler::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[0]->GetProperty(key, data);

	if(key == META_KEY_VIDEO_WIDTH)
	{
		m_srcWidth = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_VIDEO_HEIGHT)
	{
		m_srcHeight = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_PIX_FORMAT)
	{
		m_srcPixFmt = CUtil::convert<int, std::string>(data.mValue);
		m_srcPixFmt = FFPixFormat((VideoPixformat)m_srcPixFmt);
	}
	else
	{
		m_outPorts[0]->SetProperty(data);
	}
}