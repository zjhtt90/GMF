#ifndef _GDI_VIDEO_CAPTURE_H_
#define _GDI_VIDEO_CAPTURE_H_

#include "MediaBaseSrc.h"

#include <Windows.h>

class CGDIVideoCapture : public CMediaBaseSrc
{
public:
	CGDIVideoCapture();
	CGDIVideoCapture(const std::string &name);
	virtual ~CGDIVideoCapture();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);

private:
	static short m_ID;

	std::string m_devName;
	HDC m_hScrDC; 
	HDC	m_hMemDC;
	HBITMAP m_hbm;
	HWND m_hWnd;

	int m_winWidth;
	int m_winHeight;
	int m_bpp;
	void *m_pixels;
	size_t m_frameSize;

	VideoRect m_capRect;
	size_t m_capSize;

	int m_fps;
};


class CGDIVideoCaptureFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CGDIVideoCapture();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CGDIVideoCapture(name);
	}
};



#endif  //_GDI_VIDEO_CAPTURE_H_