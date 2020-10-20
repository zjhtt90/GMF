#ifndef _MEDIA_BASE_SINK_H_
#define _MEDIA_BASE_SINK_H_

#include "MediaelEment.h"

class API_EXPORT CMediaBaseSink : public CMediaElement
{
public:
	CMediaBaseSink();
	virtual ~CMediaBaseSink();

	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

private:
	static short m_sinkID;
};

#endif  //_MEDIA_BASE_SINK_H_