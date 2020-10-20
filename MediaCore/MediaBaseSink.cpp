#include "MediaBaseSink.h"
#include <assert.h>

short CMediaBaseSink::m_sinkID = 0;

CMediaBaseSink::CMediaBaseSink()
{
	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(std::string("BaseSink"));

	m_sinkID++;
}


CMediaBaseSink::~CMediaBaseSink()
{
	m_sinkID--;
	assert(m_sinkID >= 0);
}

void CMediaBaseSink::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
}