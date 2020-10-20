#ifndef _MEDIA_BASE_SRC_H_
#define _MEDIA_BASE_SRC_H_

#include "MediaElement.h"

#include <string>

class CMediaBuffer;

class API_EXPORT CMediaBaseSrc : public CMediaElement
{
public:
	CMediaBaseSrc();
	virtual ~CMediaBaseSrc();

	virtual int Open();
	virtual int Open(const std::string &uri);
	virtual void Close();
	virtual unsigned long GetSize();
	virtual bool IsSeekable();
	virtual int Seek(unsigned long milisecond);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);


private:
	static short m_srcID;
};

#endif  //_MEDIA_BASE_SRC_H_

