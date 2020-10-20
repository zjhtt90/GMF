#ifndef _MEDIA_BASE_FILTER_H_
#define _MEDIA_BASE_FILTER_H_

#include "MediaElement.h"

class API_EXPORT CMediaBaseFilter : public CMediaElement
{
public:
	CMediaBaseFilter();
	virtual ~CMediaBaseFilter();

	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

private:
	static short m_filterID;
};

#endif  //_MEDIA_BASE_FILTER_H_