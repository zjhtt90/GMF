#ifndef _VIDEO_RENDER_H_
#define _VIDEO_RENDER_H_

#include "MediaBaseSink.h"
class API_EXPORT CVideoRender : public CMediaBaseSink
{
public:
	CVideoRender();
	CVideoRender(const std::string &name);
	virtual ~CVideoRender();
	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	void SetVideoWindow(void* hwnd);
	void SetImageInfo(int width, int height, int pixFormat);
	virtual void PortSettingChanged(TRACKID index, const std::string &key);

private:
	void Init(const std::string &name);

protected:
	void *m_pHwnd;
	int m_width;
	int m_height;
	int m_pixFmt;

private:
	static short m_renderID;
};



#endif  //_VIDEO_RENDER_H_