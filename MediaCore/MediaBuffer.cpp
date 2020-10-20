#include "MediaBuffer.h"

#include "../Common/LogManager.h"

#define LOG_FILTER	"MediaBuffer"

CMediaBuffer::CMediaBuffer() : m_pData(NULL), m_size(0), m_pts(0), m_dts(0), m_duration(0)
{
}

CMediaBuffer::CMediaBuffer(const unsigned char *buf, int size, unsigned long long pts, unsigned long long dts, unsigned long long duration)
	: m_size(size), m_pts(pts), m_dts(dts), m_duration(duration)
{
	if(size > 0)
	{
		m_pData = new unsigned char[size];
		if(buf != NULL)
		{
			memcpy(m_pData, buf, size);
		}
	}
	else
	{
		m_pData = NULL;
	}
}

CMediaBuffer::CMediaBuffer(const CMediaBuffer &other)
{
	m_pts = other.m_pts;
	m_dts = other.m_dts;
	m_duration = other.m_duration;
	m_size = other.m_size;

	if(m_size > 0)
	{
		m_pData = new unsigned char[m_size];
		if(other.m_pData != NULL)
		{
			memcpy(m_pData, other.m_pData, m_size);
		}
	}
}

CMediaBuffer::~CMediaBuffer()
{
	if(m_pData != NULL)
	{
		delete []m_pData;
		m_pData = NULL;
	}
}

unsigned char* CMediaBuffer::GetData() const
{
	return m_pData;
}

size_t CMediaBuffer::GetDataSize() const
{
	return m_size;
}

unsigned long long CMediaBuffer::GetPts() const
{
	return m_pts;
}

unsigned long long CMediaBuffer::GetDts() const
{
	return m_dts;
}

unsigned long long CMediaBuffer::GetDuration() const
{
	return m_duration;
}
