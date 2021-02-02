#ifndef _RTP_SRC_H_
#define _RTP_SRC_H_

#include "../MediaCore/MediaBaseSrc.h"

#include "jrtplib3/rtpsession.h"

class CRTPSrc : public CMediaBaseSrc
{
public:
	CRTPSrc();
	CRTPSrc(const std::string &name);
	virtual ~CRTPSrc();

	virtual int Open();
	virtual void Close();

	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);
	int Open(int portbase);

private:
	static short m_ID;
	jrtplib::RTPSession m_rtpSess;
};


class CRTPSrcFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CRTPSrc();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CRTPSrc(name);
	}
};


#endif  //_RTP_SRC_H_