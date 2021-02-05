#include "FFScreenCapture.h"
#include "common.h"
#include "Util.h"
#include "LogManager.h"
#include "MediaBuffer.h"
#include <assert.h>
#include "libavutil/pixfmt.h"
#ifndef SYSTEM_WIN32
#include<X11/Xlib.h>
#endif

#define LOG_FILTER	"FFScreenCapture"

short CFFScreenCapture::m_ID = 0;

CFFScreenCapture::CFFScreenCapture()
	: m_hWnd(0), m_winWidth(0), m_winHeight(0), m_fps(30), m_pFmtCtx(NULL), m_dataHeadLen(0)
{
}

CFFScreenCapture::CFFScreenCapture(const std::string &name)
	: m_hWnd(0),m_winWidth(0), m_winHeight(0), m_fps(30), m_pFmtCtx(NULL), m_dataHeadLen(0)
{
	Init(name);
}

CFFScreenCapture::~CFFScreenCapture()
{
	m_ID--;
	assert(m_ID >= 0);
}

void CFFScreenCapture::Init(const std::string &name)
{
	m_ID++;

	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	MetaData media(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(media);
	m_outPorts[0]->SetProperty(media);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

int CFFScreenCapture::Open()
{
	LOG_DEBUG("Open ffmpeg Screen Capture");
	int res = 0;

#ifdef _WIN32
	AVInputFormat *ifmt = av_find_input_format("gdigrab");
#else
	AVInputFormat *ifmt = av_find_input_format("x11grab");
#endif
	if(ifmt == NULL)
	{
		LOG_ERR_S("Can not find  grab!");

		return MEDIA_ERR_NOT_FOUND;
	}

	m_pFmtCtx = avformat_alloc_context();
	m_pFmtCtx->iformat = ifmt;


	AVDictionary* options = NULL;

	char resolution[64] = {0};
	char framerate[64] = {0};
	snprintf(resolution , sizeof(resolution), "%dx%d", m_capRect.width, m_capRect.height);
	snprintf(framerate,sizeof(framerate),"%d", m_fps);	
	av_dict_set(&options,"framerate",framerate,0);
	av_dict_set(&options,"video_size",resolution,0);
	av_dict_set(&options,"draw_mouse","1",0);

#ifdef _WIN32
	char posx[8] = {0};
	char posy[8] = {0};
	snprintf(posx,sizeof(posx),"%d", m_capRect.x);
	snprintf(posy,sizeof(posy),"%d", m_capRect.y);
	av_dict_set(&options,"offset_x",posx,0);
	av_dict_set(&options,"offset_y",posy,0);
	std::string dev = "desktop";
	if(m_hWnd != NULL)
	{
		char title[1024] = {0};
		GetWindowText((HWND)m_hWnd, title, 1024);
		dev = "title=" + std::string(title);
	}

	LOG_DEBUG("Capture %s Rect(%d,%d,%d,%d), fps(%d)", dev.c_str(), m_capRect.x, m_capRect.y, m_capRect.width, m_capRect.height, m_fps);

	res = avformat_open_input(&m_pFmtCtx, dev.c_str(), m_pFmtCtx->iformat, &options);
#else
	char dev[128] = {0};
	snprintf(dev,sizeof(dev),":0.0+%d+%d",m_capRect.x,m_capRect.y);
	res = avformat_open_input(&m_pFmtCtx,dev,m_pFmtCtx->iformat,&options);
#endif
	if(res < 0)
	{
		LOG_ERR("Open error, %d!", res);

		return MEDIA_ERR_INVALIDE_PARAME;
	}

#ifdef _WIN32
	m_dataHeadLen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
#endif

	LOG_DEBUG("Open ffmpeg Screen Capture end");
	return MEDIA_ERR_NONE;
}

void CFFScreenCapture::Close()
{
	LOG_DEBUG("Close ffmpeg Screen Capture");
	if(m_pFmtCtx != NULL)
	{
		avformat_close_input(&m_pFmtCtx);
		m_pFmtCtx = NULL;
	}
}

void CFFScreenCapture::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if(m_desc.GetMetaData(META_KEY_VIDEO_WINDOW, data) == MEDIA_ERR_NONE)
		{
			long hwnd = 0x00000000;
			hwnd = CUtil::convert<long, std::string>(data.mValue);

			m_hWnd = (void*)hwnd;
		}

#ifdef _WIN32
		if(m_hWnd)
		{
			RECT rect;
			GetWindowRect((HWND)m_hWnd, &rect);
			m_winWidth = rect.right - rect.left;
			m_winHeight = rect.bottom - rect.top;
		}
		else
		{
			m_winWidth = GetSystemMetrics(SM_CXSCREEN);
			m_winHeight = GetSystemMetrics(SM_CYSCREEN);
		}
#else
#endif

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			data = MetaData(META_KEY_PIX_FORMAT, CUtil::convert<std::string, int>(VIDEO_PIX_FMT_BGRA), META_DATA_VAL_TYPE_INT);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_VIEW_RECT, data) == MEDIA_ERR_NONE)
		{
			sscanf(data.mValue.c_str(), "(%d,%d,%d,%d)", m_capRect.x, m_capRect.y, m_capRect.width, m_capRect.height);
		}
		else
		{
			m_capRect.x = 0;
			m_capRect.y = 0;
			m_capRect.width = m_winWidth;
			m_capRect.height = m_winHeight;
		}

		data = MetaData(META_KEY_VIDEO_WIDTH, CUtil::convert<std::string, int>(m_capRect.width), META_DATA_VAL_TYPE_INT);
		m_outPorts[0]->SetProperty(data);

		data = MetaData(META_KEY_VIDEO_HEIGHT, CUtil::convert<std::string, int>(m_capRect.height), META_DATA_VAL_TYPE_INT);
		m_outPorts[0]->SetProperty(data);


		if(m_desc.GetMetaData(META_KEY_VIDEO_FPS, data) == MEDIA_ERR_NONE)
		{
			m_fps = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			data = MetaData(META_KEY_VIDEO_FPS, CUtil::convert<std::string, int>(m_fps), META_DATA_VAL_TYPE_INT);
			m_outPorts[0]->SetProperty(data);
		}


	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(Open() == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->StartTask();
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();

		Close();
	}
}

int CFFScreenCapture::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	AVPacket packet;
	av_init_packet(&packet);

	if(av_read_frame(m_pFmtCtx, &packet)>=0)
	{
		*buffer = new CMediaBuffer(packet.data+m_dataHeadLen, packet.size-m_dataHeadLen, packet.pts, packet.dts, packet.duration);

		av_free_packet(&packet);

		//CSThread::Sleep(1000/m_fps);

		return MEDIA_ERR_NONE;
	}
	else
	{
		LOG_ERR("Read Packet Error");
		return MEDIA_ERR_READ_FAILED;
	}
}