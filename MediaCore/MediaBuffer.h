#ifndef _MEDIA_BUFFER_H_
#define _MEDIA_BUFFER_H_

#include "../config.h"

#include "MediaDefs.h"

#include <string.h>

class API_EXPORT CMediaBuffer
{
public:

	CMediaBuffer();
	CMediaBuffer(const unsigned char *buf, int size, unsigned long long pts, unsigned long long dts, unsigned long long duration);
	CMediaBuffer(const CMediaBuffer &other);
	~CMediaBuffer();

	unsigned char* GetData() const;
	size_t GetDataSize() const;

	unsigned long long GetPts() const;

	unsigned long long GetDts() const;

	unsigned long long GetDuration() const;

private:
	unsigned char *m_pData;
	size_t m_size;
	unsigned long long m_pts;
	unsigned long long m_dts;
	unsigned long long m_duration;
};

class CVideoBuffer : public CMediaBuffer
{
public:
	CVideoBuffer() : CMediaBuffer(), m_picType('I')
	{

	}

	CVideoBuffer(unsigned char *buf, int size, unsigned long long pts, 
		unsigned long long dts, unsigned long long duration, char type = 'I')
		: CMediaBuffer(buf, size, pts, dts, duration), m_picType(type)
	{

	}

	char m_picType;
};


class CAudioBuffer : public CMediaBuffer
{
public:
	CAudioBuffer() : CMediaBuffer(), m_type(OV_AUDIO)
	{

	}

	CAudioBuffer(unsigned char *buf, int size, unsigned long long pts, 
		unsigned long long dts, unsigned long long duration, EFrameType type = OV_AUDIO)
		: CMediaBuffer(buf, size, pts, dts, duration), m_type(type)
	{

	}

	EFrameType m_type;
};



#endif  //_MEDIA_BUFFER_H_