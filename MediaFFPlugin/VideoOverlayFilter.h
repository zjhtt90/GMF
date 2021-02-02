#ifndef _VEDIO_OVERLAY_H_
#define _VEDIO_OVERLAY_H_

#include "../MediaCore/MediaBaseFilter.h"

extern "C"
{
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/opt.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
};

typedef struct _OverlayInfo
{
	int mStartX;
	int mStartY;
	int mImgWidth;
	int mImgHeight;
	int mImgFormat;
	int mFps;
}OverlayInfo_t;

class CVideoOverlayFilter : public CMediaBaseFilter
{
public:
	CVideoOverlayFilter();
	CVideoOverlayFilter(const std::string &name);
	virtual ~CVideoOverlayFilter();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);
private:
	void Init(const std::string &name);
	int InitFilterGraph();
	void UnInitFilterGraph();

private:
	OverlayInfo_t m_overInfos[MAX_SIDE_PORT];
	int m_outFormat;

	AVFilterGraph *m_pFilterGraph;
	AVFilterContext *m_pBufferSrcCtx[MAX_SIDE_PORT];
	AVFilterContext *m_pBufferSinkCtx;
};



class CVideoFrameFuseFilterFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CVideoOverlayFilter();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CVideoOverlayFilter(name);
	}
};




#endif  //_VEDIO_OVERLAY_H_