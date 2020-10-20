#ifndef _FILE_SINK_H_
#define _FILE_SINK_H_

#include "../MediaBaseSink.h"

class CFileSink : public CMediaBaseSink
{
public:
	CFileSink();
	CFileSink(const std::string &name);
	~CFileSink();

	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
private:
	void Init(const std::string &name);

private:
	static short m_ID;
	FILE *m_pFilePtr;
};



class CFileSinkFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFileSink();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFileSink(name);
	}
};



#endif  //_FILE_SINK_H_