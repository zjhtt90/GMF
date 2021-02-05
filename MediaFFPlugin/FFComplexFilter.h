#ifndef _FF_COMPLEX_FILTER_H_
#define _FF_COMPLEX_FILTER_H_

#include "MediaBaseFilter.h"

extern "C"
{
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/opt.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/channel_layout.h"
#include "libavcodec/avcodec.h"
};

class CFFComplexFilter : public CMediaBaseFilter
{
public:
	CFFComplexFilter();
	CFFComplexFilter(const std::string &name);
	virtual ~CFFComplexFilter();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

private:
	void Init(const std::string &name);
	int InitFilterGraph();
	void UnInitFilterGraph();

protected:
	std::string m_filterDesc;
private:
	int m_iPortCnt;
	int m_oPortCnt;
	MediaMetaInfo *m_iMediaInfo[MAX_SIDE_PORT];
	MediaMetaInfo *m_oMediaInfo[MAX_SIDE_PORT];
	AVFilterGraph *m_pFilterGraph;
	AVFilterContext *m_pBufferSrcCtxs[MAX_SIDE_PORT];
	AVFilterContext *m_pBufferSinkCtxs[MAX_SIDE_PORT];
	AVFrame *m_srcFrame[MAX_SIDE_PORT];
	AVFrame *m_dstFrame[MAX_SIDE_PORT];
};



class CFFComplexFilterFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFComplexFilter();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFComplexFilter(name);
	}
};


#endif  //_FF_COMPLEX_FILTER_H_