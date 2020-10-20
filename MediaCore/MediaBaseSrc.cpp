#include "MediaBaseSrc.h"

#include <assert.h>

short CMediaBaseSrc::m_srcID = 0;

CMediaBaseSrc::CMediaBaseSrc()
{
	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName("BaseSrc");

	m_srcID++;

}


CMediaBaseSrc::~CMediaBaseSrc()
{
	m_srcID--;
	assert(m_srcID >= 0);

}

int CMediaBaseSrc::Open()
{
	return MEDIA_ERR_NONE;
}

int CMediaBaseSrc::Open(const std::string &uri)
{
	return MEDIA_ERR_NONE;
}

void CMediaBaseSrc::Close()
{

}

unsigned long CMediaBaseSrc::GetSize()
{
	return 0;
}

bool CMediaBaseSrc::IsSeekable()
{
	return false;
}

int CMediaBaseSrc::Seek(unsigned long milisecond)
{
	return MEDIA_ERR_NONE;
}

int CMediaBaseSrc::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}