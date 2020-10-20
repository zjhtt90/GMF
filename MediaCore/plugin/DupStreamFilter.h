#ifndef _DUP_STREAM_H_
#define _DUP_STREAM_H_

#include "../MediaBaseFilter.h"

class CDupStreamFilter : public CMediaBaseFilter
{
public:
	CDupStreamFilter();
	CDupStreamFilter(const std::string &name);
	~CDupStreamFilter();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);
private:
	void Init(const std::string &name);

};




class CDupStreamFilterFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CDupStreamFilter();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CDupStreamFilter(name);
	}
};


#endif  //_DUP_STREAM_H_