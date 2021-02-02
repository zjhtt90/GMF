#include "VideoRender.h"
#include "LogManager.h"
#include "MediaBuffer.h"
#include "Util.h"

#include <assert.h>

#define LOG_FILTER	"VideoRender"

short CVideoRender::m_renderID = 0;

CVideoRender::CVideoRender() 
	: m_pHwnd(NULL), m_width(0), m_height(0), m_pixFmt(0)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_renderID;
	std::string name = "VideoRender" + ss.str();	

	Init(name);
}

CVideoRender::CVideoRender(const std::string &name)
	: m_pHwnd(NULL), m_width(0), m_height(0), m_pixFmt(0)
{
	Init(name);
}


CVideoRender::~CVideoRender()
{
	m_renderID--;
	assert(m_renderID >= 0);
}

void CVideoRender::Init(const std::string &name)
{
	m_renderID++;


	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);
	
	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);

	this->SetPrivateData(data);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_inPorts[0]->SetProperty(data);
	/*
	data = MetaData(META_KEY_VIDEO_WIDTH, "640", META_DATA_VAL_TYPE_INT);
	m_inPorts[0]->SetProperty(data);
	data = MetaData(META_KEY_VIDEO_HEIGHT, "480", META_DATA_VAL_TYPE_INT);
	m_inPorts[0]->SetProperty(data);
	data = MetaData(META_KEY_PIX_FORMAT, "100", META_DATA_VAL_TYPE_INT);
	m_inPorts[0]->SetProperty(data);
	*/
	m_desc.SetElementPortCount(1, 0);
}

void CVideoRender::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{
			int cnt = 0;
			MetaData data;
			if(m_desc.GetMetaData(META_KEY_SUBTITLE, data) == MEDIA_ERR_NONE)
			{
				cnt = CUtil::convert<int, std::string>(data.mValue);
				if(cnt > 0)
				{
					for(int i = 1; i <= cnt; i++)
					{
						m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
						m_inPorts[i]->SetProperty(MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_SUBTITLE), META_DATA_VAL_TYPE_STRING));
					}
				}
			}

			m_desc.SetElementPortCount(cnt+1, 0);


			if(m_desc.GetMetaData(META_KEY_VIDEO_WINDOW, data) == MEDIA_ERR_NONE)
			{
				long hwnd = 0x00000000;
				hwnd = CUtil::convert<long, std::string>(data.mValue);
				//sscanf(data.mValue.c_str(), "%ld", hwnd);
				SetVideoWindow((void*)hwnd);
			}

			int w = 0;
			int h = 0;
			int fmt = 0;
			if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
			{
				w = CUtil::convert<int, std::string>(data.mValue);
			}
			if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
			{
				h = CUtil::convert<int, std::string>(data.mValue);
			}
			if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
			{
				fmt = CUtil::convert<int, std::string>(data.mValue);
			}

			SetImageInfo(w, h, fmt);

			m_desc.Print();
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

int CVideoRender::Open()
{
	return MEDIA_ERR_NONE;
}

void CVideoRender::Close()
{

}


void CVideoRender::SetVideoWindow(void* hwnd)
{
	if(m_pHwnd == hwnd)
		return;

	m_pHwnd = hwnd;

	std::string win = CUtil::convert<std::string, long>((long)hwnd);

	MetaData data(META_KEY_VIDEO_WINDOW, win, META_DATA_VAL_TYPE_PTR);
	if(m_desc.UpdateMetaData(data) == MEDIA_ERR_NOT_FOUND)
	{
		m_desc.SetExternData(data);
	}
}

void CVideoRender::SetImageInfo(int width, int height, int pixFormat)
{
	if(width <= 0 || height <= 0 || pixFormat <= 0)
		return;

	if(m_width == width &&
		m_height == height &&
		m_pixFmt == pixFormat)
		return;

	m_width = width;
	m_height = height;
	m_pixFmt = pixFormat;

	MetaData data(META_KEY_VIDEO_WIDTH, CUtil::convert<std::string, int>(width), META_DATA_VAL_TYPE_INT);
	if(m_desc.UpdateMetaData(data) == MEDIA_ERR_NOT_FOUND)
	{
		m_desc.SetExternData(data);
	}

	data = MetaData(META_KEY_VIDEO_HEIGHT, CUtil::convert<std::string, int>(height), META_DATA_VAL_TYPE_INT);
	if(m_desc.UpdateMetaData(data) == MEDIA_ERR_NOT_FOUND)
	{
		m_desc.SetExternData(data);
	}

	data = MetaData(META_KEY_PIX_FORMAT, CUtil::convert<std::string, int>(pixFormat), META_DATA_VAL_TYPE_INT);
	if(m_desc.UpdateMetaData(data) == MEDIA_ERR_NOT_FOUND)
	{
		m_desc.SetExternData(data);
	}
}


void CVideoRender::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	if(index == 0)
	{
		m_inPorts[0]->GetProperty(META_KEY_VIDEO_WIDTH, data);
		m_desc.UpdateMetaData(data);
		m_width = CUtil::convert<int, std::string>(data.mValue);


		m_inPorts[0]->GetProperty(META_KEY_VIDEO_HEIGHT, data);
		m_desc.UpdateMetaData(data);
		m_height = CUtil::convert<int, std::string>(data.mValue);
		

		m_inPorts[0]->GetProperty(META_KEY_PIX_FORMAT, data);
		m_desc.UpdateMetaData(data);
		m_pixFmt = CUtil::convert<int, std::string>(data.mValue);	
	}
}