#ifndef _SDL_VID_RENDER_H_
#define _SDL_VID_RENDER_H_

#include "../MediaCore/VideoRender.h"
#include "../Common/SMutex.h"

#include "SDL.h"

class CSDLVideoRender : public CVideoRender
{
public:
	CSDLVideoRender();
	CSDLVideoRender(const std::string &name);
	virtual ~CSDLVideoRender();

	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

private:
	SDL_Window *m_pWindow;
	SDL_Renderer *m_pRender;
	SDL_Texture *m_pTexture;
	CSMutex m_textureLock;

	VideoRect m_winRect;
	VideoRect m_dstVideoRect;
};



class CSDLVideoRenderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CSDLVideoRender();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CSDLVideoRender(name);
	}
};


#endif  //_SDL_VID_RENDER_H_