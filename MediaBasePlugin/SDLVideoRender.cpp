#include "SDLVideoRender.h"

#include "../MediaCore/MediaBuffer.h"
#include "../Common/LogManager.h"
#include "../Common/Util.h"

#include <string.h>

CSDLVideoRender::CSDLVideoRender()
	: m_pWindow(NULL), m_pRender(NULL), m_pTexture(NULL)
{
	memset(&m_winRect, 0, sizeof(VideoRect));
	memset(&m_dstVideoRect, 0, sizeof(VideoRect));
}

CSDLVideoRender::CSDLVideoRender(const std::string &name)
	:CVideoRender(name), m_pWindow(NULL), m_pRender(NULL), m_pTexture(NULL)
{
	memset(&m_winRect, 0, sizeof(VideoRect));
	memset(&m_dstVideoRect, 0, sizeof(VideoRect));
}

CSDLVideoRender::~CSDLVideoRender()
{
}

int CSDLVideoRender::Open()
{
	SDL_SetHint(SDL_HINT_WINDOWS_ENABLE_MESSAGELOOP, "0");

	if(m_pHwnd != NULL)
	{
		m_pWindow = SDL_CreateWindowFrom(m_pHwnd);

		SDL_GetWindowPosition(m_pWindow, &m_winRect.x, &m_winRect.y);
		SDL_GetWindowSize(m_pWindow, &m_winRect.width, &m_winRect.height);
		SDL_ShowWindow(m_pWindow);
	}
	else
	{
		if(m_width == 0 || m_height == 0)
			return MEDIA_ERR_INVALIDE_PARAME;

		MetaData data;
		if(m_desc.GetMetaData(META_KEY_WINDOW_X_POS, data) == MEDIA_ERR_NONE)
		{
			m_winRect.x = CUtil::convert<int, std::string>(data.mValue);
		}
		if(m_desc.GetMetaData(META_KEY_WINDOW_Y_POS, data) == MEDIA_ERR_NONE)
		{
			m_winRect.y = CUtil::convert<int, std::string>(data.mValue);
		}

		m_dstVideoRect.width = m_winRect.width = m_width;
		m_dstVideoRect.height = m_winRect.height = m_height;
		m_pWindow = SDL_CreateWindow(GetName().c_str(), /*SDL_WINDOWPOS_UNDEFINED*/m_winRect.x, /*SDL_WINDOWPOS_UNDEFINED*/m_winRect.y,
			m_winRect.width, m_winRect.height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	}
	if(m_pWindow == NULL)
	{
		LOG_ERR("Create window faild!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");


	m_pRender = SDL_CreateRenderer(m_pWindow, 0, SDL_RENDERER_ACCELERATED);
	if(m_pRender == NULL)
	{
		LOG_ERR("Create render faild!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	m_pTexture = SDL_CreateTexture(m_pRender, SDL_PIXELFORMAT_IYUV/*m_pixFmt*/, SDL_TEXTUREACCESS_STREAMING, m_dstVideoRect.width, m_dstVideoRect.height);
	if(m_pTexture == NULL)
	{
		LOG_ERR("Create texture faild!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	return MEDIA_ERR_NONE;
}

void CSDLVideoRender::Close()
{
	m_textureLock.Lock();
	if(m_pTexture != NULL)
	{
		SDL_DestroyTexture(m_pTexture);
		m_pTexture = NULL;
	}
	m_textureLock.UnLock();

	if(m_pRender != NULL)
	{
		SDL_DestroyRenderer(m_pRender);
		m_pRender = NULL;
	}

	if(m_pWindow != NULL)
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = NULL;
	}
}

void CSDLVideoRender::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	uint8_t *yuv = NULL;
	int pitch;

	m_textureLock.Lock();
	if(m_pTexture != NULL)
	{
		SDL_LockTexture(m_pTexture, NULL,(void**)&yuv, &pitch);
		memcpy(yuv, buffer->GetData(), buffer->GetDataSize());
		SDL_UnlockTexture(m_pTexture);
		SDL_RenderCopy(m_pRender,m_pTexture, NULL, NULL);
		SDL_RenderPresent(m_pRender);
	}
	m_textureLock.UnLock();
}

void CSDLVideoRender::PortSettingChanged(TRACKID index, const std::string &key)
{
	CVideoRender::PortSettingChanged(index, key);

	MediaElementState state = GetCurrentState();
	if(state >= MEDIA_ELEMENT_STATE_OPEN && state < MEDIA_ELEMENT_STATE_STOP)
	{
		Close();

		Open();
	}
}