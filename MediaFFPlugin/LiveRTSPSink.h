#ifndef _RTSP_SINK_H_
#define _RTSP_SINK_H_

#include "LiveStreamIO.h"
#include "SThread.h"
#include "MediaBaseSink.h"

class CLiveRTSPSink : public CMediaBaseSink, public CSThread
{
public:
	CLiveRTSPSink();
	CLiveRTSPSink(const std::string &name);
	virtual ~CLiveRTSPSink();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);

	virtual void Run();

	static void LiveStreamVSrcCallbackFunc(int state, void* pObj);
	static void LiveStreamASrcCallbackFunc(int state, void* pObj);

private:
	LiveStreamIO* m_pSrc[MAX_SIDE_PORT];
	int m_channelCount;
	int m_sdpMediaCnt;
	std::string m_baseSDP;
	std::string m_aSDP;
	std::string m_vSDP;
	bool m_bGetKeyFrame;
};



class CLiveRTSPSinkFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CLiveRTSPSink();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CLiveRTSPSink(name);
	}
};


#endif  //_RTSP_SINK_H_