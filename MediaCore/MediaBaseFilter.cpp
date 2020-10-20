#include "MediaBaseFilter.h"
#include <assert.h>

short CMediaBaseFilter::m_filterID = 0;

CMediaBaseFilter::CMediaBaseFilter()
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName("BaseFilter");

	m_filterID++;
}


CMediaBaseFilter::~CMediaBaseFilter()
{
	m_filterID--;
	assert(m_filterID >= 0);
}

void CMediaBaseFilter::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
}

int CMediaBaseFilter::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}
