#include "GDIVideoCapture.h"

#include "../MediaBuffer.h"
#include "../../Common/Util.h"
#include "../../Common/LogManager.h"

#include <assert.h>
#include <sstream>

#ifndef SYSTEM_WIN32
#include <unistd.h>
#endif

#define LOG_FILTER	"GDIVideoCapture"

short CGDIVideoCapture::m_ID = 0;

CGDIVideoCapture::CGDIVideoCapture()
	: m_hScrDC(0), m_hMemDC(0), m_hbm(0), m_hWnd(0),
	m_winWidth(0), m_winHeight(0), m_bpp(16), m_pixels(NULL), m_frameSize(0), m_capSize(0), m_fps(30)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_ID;
	std::string name = "GDIVideoCapture" + ss.str();	

	Init(name);
}

CGDIVideoCapture::CGDIVideoCapture(const std::string &name)
	: m_hScrDC(0), m_hMemDC(0), m_hbm(0), m_hWnd(0),
	m_winWidth(0), m_winHeight(0), m_bpp(16), m_pixels(NULL), m_frameSize(0), m_capSize(0), m_fps(30)
{
	Init(name);
}

CGDIVideoCapture::~CGDIVideoCapture()
{
	m_ID--;
	assert(m_ID >= 0);
}

void CGDIVideoCapture::Init(const std::string &name)
{
	m_ID++;


	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	m_desc.SetExternData(MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING));

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

int CGDIVideoCapture::Open()
{
	size_t size;
	LPBITMAPINFO info;

	/* Find out the format of the screen */
	size = sizeof(BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD);
	info = (LPBITMAPINFO)malloc(size);
	if (!info) 
	{
		return MEDIA_ERR_INVALIDE_PARAME;
	}


	if(m_hWnd)
	{
		m_hScrDC = GetWindowDC(m_hWnd);
	}
	else
	{
		m_hScrDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	}


	/* The second call to GetDIBits() fills in the bitfields */
	m_hbm = CreateCompatibleBitmap(m_hScrDC, 1, 1);
	GetDIBits(m_hScrDC, m_hbm, 0, 0, NULL, info, DIB_RGB_COLORS);
	GetDIBits(m_hScrDC, m_hbm, 0, 0, NULL, info, DIB_RGB_COLORS);
	DeleteObject(m_hbm);

	{
		/* Create a new one */
		memset(info, 0, size);
		info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info->bmiHeader.biPlanes = 1;
		info->bmiHeader.biBitCount = m_bpp;
		info->bmiHeader.biCompression = BI_RGB;
	}

	/* Fill in the size information */
	int pitch = m_winWidth * (m_bpp>>3);
	info->bmiHeader.biWidth = m_winWidth;
	info->bmiHeader.biHeight = -m_winHeight;  /* negative for topdown bitmap */
	info->bmiHeader.biSizeImage = m_winHeight * pitch;
	m_frameSize = info->bmiHeader.biSizeImage;

	m_hMemDC = CreateCompatibleDC(m_hScrDC);
	m_hbm = CreateDIBSection(m_hScrDC, info, DIB_RGB_COLORS, &m_pixels, NULL, 0);
	free(info);

	if (!m_hbm) 
	{
		goto error;
	}
	SelectObject(m_hMemDC, m_hbm);
	return MEDIA_ERR_NONE;

error:
	if (m_hMemDC) 
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}

	if (m_hScrDC) 
	{
		DeleteDC(m_hScrDC);
		m_hScrDC = NULL;
	}
	return MEDIA_ERR_INVALIDE_PARAME;
}

void CGDIVideoCapture::Close()
{
	if (m_hbm) 
	{
		DeleteObject(m_hbm);
		m_hbm = NULL;
	}

	if (m_hMemDC) 
	{
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
	}

	if (m_hScrDC) 
	{
		DeleteDC(m_hScrDC);
		m_hScrDC = NULL;
	}
}

void CGDIVideoCapture::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		m_outPorts[0]->SetProperty(MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING));

		MetaData data;

		if(m_desc.GetMetaData(META_KEY_FILE_PATH, data) == MEDIA_ERR_NONE)
		{
			m_devName = data.mValue;
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_WINDOW, data) == MEDIA_ERR_NONE)
		{
			long hwnd = 0x00000000;
			hwnd = CUtil::convert<long, std::string>(data.mValue);

			m_hWnd = (HWND)hwnd;
		}

		if(m_hWnd)
		{
			RECT rect;
			GetWindowRect(m_hWnd, &rect);
			m_winWidth = rect.right - rect.left;
			m_winHeight = rect.bottom - rect.top;
		}
		else
		{
			m_winWidth = GetSystemMetrics(SM_CXSCREEN);
			m_winHeight = GetSystemMetrics(SM_CYSCREEN);
		}

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			int pixfmt = CUtil::convert<int, std::string>(data.mValue);
			switch(pixfmt)
			{
			case VIDEO_PIX_FMT_RGB555:
			case VIDEO_PIX_FMT_BGR555:
			case VIDEO_PIX_FMT_RGB565:
			case VIDEO_PIX_FMT_BGR565:
				m_bpp = 16;
				break;
			case VIDEO_PIX_FMT_RGB24:
			case VIDEO_PIX_FMT_BGR24:
				m_bpp = 24;
				break;
			case VIDEO_PIX_FMT_ARGB:
			case VIDEO_PIX_FMT_RGBA:
			case VIDEO_PIX_FMT_ABGR:
			case VIDEO_PIX_FMT_BGRA:
				m_bpp = 32;
				break;
			default:
				m_bpp = 24;
				break;
			}

			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			data = MetaData(META_KEY_PIX_FORMAT, CUtil::convert<std::string, int>(VIDEO_PIX_FMT_RGB565), META_DATA_VAL_TYPE_INT);
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
		
		int pitch = m_capRect.width * (m_bpp >> 3);
		m_capSize = m_capRect.height * pitch;

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

int CGDIVideoCapture::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	memset(m_pixels, 0x00, m_frameSize);
	
	if(BitBlt(m_hMemDC, m_capRect.x, m_capRect.y, m_capRect.width, m_capRect.height, 
		m_hScrDC, m_capRect.x, m_capRect.y, SRCCOPY))
	{
		long long ts = CUtil::GetTimeStamp();
		*buffer = new CMediaBuffer((unsigned char*)m_pixels, m_capSize, ts, ts, 1e+6/m_fps);

		CSThread::Sleep(1000/m_fps);

		return MEDIA_ERR_NONE;
	}
	else
	{
		return MEDIA_ERR_READ_FAILED;
	}
}