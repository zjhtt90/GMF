#ifndef _FF_RTSP_SERVER_H_
#define _FF_RTSP_SERVER_H_

#include "MediaBaseSink.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
}

class CFFRTSPServer : public CMediaBaseSink
{
public:
	CFFRTSPServer();
	CFFRTSPServer(const std::string &name);
	virtual ~CFFRTSPServer();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);
	int Open();
	void Close();
	void AddStream(TRACKID index);
private:
	int m_iPortCnt;
	MediaMetaInfo *m_iMediaInfo[MAX_SIDE_PORT];
	AVFormatContext* m_pOFmtCtx;
};


class CFFRTSPServerFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFRTSPServer();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFRTSPServer(name);
	}
};


#endif //_FF_RTSP_SERVER_H_