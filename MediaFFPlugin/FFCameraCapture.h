#ifndef _FF_CAMERA_CAPTURE_H_
#define _FF_CAMERA_CAPTURE_H_

#include "MediaBaseSrc.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
};

class CFFCameraCapture : public CMediaBaseSrc
{
public:
	CFFCameraCapture();
	CFFCameraCapture(const std::string &name);
	virtual ~CFFCameraCapture();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);

private:
	AVFormatContext *m_fmtCtx;
	std::string m_devName;
	int m_width;
	int m_height;
	int m_pixFmt;
	int m_fps;
};



class CFFCameraCaptureFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFCameraCapture();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFCameraCapture(name);
	}
};




#endif  //_CAMERA_CAPTURE_H_