#ifndef _D3D_VIDEO_RENDER_H_
#define _D3D_VIDEO_RENDER_H_

#include "VideoRender.h"

#include <d3d9.h>

class CD3DVideoRender : public CVideoRender
{
public:
	CD3DVideoRender();
	CD3DVideoRender(const std::string &name);
	virtual ~CD3DVideoRender();
	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
private:
	int RenderFrame(BYTE *pData, size_t size);

private:
	IDirect3D9 *m_pDirect3D9;
	IDirect3DDevice9 *m_pDirecr3DDevice;
	IDirect3DSurface9 *m_pDirect3DSurface;

	HWND m_hWnd;
	RECT m_viewRect;
};


class CD3DVideoRenderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CD3DVideoRender();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CD3DVideoRender(name);
	}
};



#endif  //_D3D_VIDEO_RENDER_H_