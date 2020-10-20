#ifndef _FF_SIMPLE_FILTER_H_
#define _FF_SIMPLE_FILTER_H_

#include "../MediaCore/MediaBaseFilter.h"

extern "C"
{
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libavutil/opt.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
};

class CFFSimpleFilter : public CMediaBaseFilter
{
public:
	CFFSimpleFilter();
	CFFSimpleFilter(const std::string &name);
	virtual ~CFFSimpleFilter();
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
	MediaType m_type;
	//for video
	int m_srcWidth;
	int m_srcHeight;
	int m_srcPixfmt;
	int m_srcfps;
	int m_dstWidth;
	int m_dstHeight;
	int m_dstPixfmt;
	int m_dstfps;

	//for audio
	int m_srcSampleRate;
	int m_srcChannels;
	int m_srcSamplefmt;
	int m_samplecount;
	int m_dstSampleRate;
	int m_dstChannels;
	int m_dstSamplefmt;

	AVFilterGraph *m_pFilterGraph;
	AVFilterContext *m_pBufferSrcCtx;
	AVFilterContext *m_pBufferSinkCtx;
	AVFrame *m_srcFrame;
	AVFrame *m_dstFrame;
};



class CFFSimpleFilterFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFSimpleFilter();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFSimpleFilter(name);
	}
};


#endif  //_FF_SIMPLE_FILTER_H_