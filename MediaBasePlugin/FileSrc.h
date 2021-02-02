#ifndef _FILE_SRC_H_
#define _FILE_SRC_H_

#include "MediaBaseSrc.h"


class CFileSrc : public CMediaBaseSrc
{
public:
	CFileSrc();
	CFileSrc(const std::string &name);
	virtual ~CFileSrc();
	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual int Open(const std::string &uri);
	virtual void Close();
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);

private:
	static short m_ID;
	FILE *m_pFilePtr;
	unsigned long m_lReadTrunkSize;
};


class CFileSrcFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFileSrc();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFileSrc(name);
	}
};


#endif  //_FILE_SRC_H_