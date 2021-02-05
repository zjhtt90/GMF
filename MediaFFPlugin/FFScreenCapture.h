#ifndef _FF_SCREEN_CAPTURE_H_
#define _FF_SCREEN_CAPTURE_H_

#include "MediaBaseSrc.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavutil/dict.h"
#include "libavutil/imgutils.h"
};

class CFFScreenCapture : public CMediaBaseSrc
{
public:
	CFFScreenCapture();
	CFFScreenCapture(const std::string &name);
	virtual ~CFFScreenCapture();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);

private:
	static short m_ID;
	
	void *m_hWnd;

	int m_winWidth;
	int m_winHeight;

	VideoRect m_capRect;
	int m_fps;

	AVFormatContext *m_pFmtCtx;
	unsigned m_dataHeadLen;
};


class CFFScreenCaptureFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFScreenCapture();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFScreenCapture(name);
	}
};




#endif  //_FF_SCREEN_CAPTURE_H_