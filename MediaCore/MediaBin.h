#ifndef _MEDIA_BIN_H_
#define _MEDIA_BIN_H_

#include "MediaElement.h"
#include <list>

class CMediaBin : public CMediaElement
{
public:
	CMediaBin();
	CMediaBin(const std::string &name);
	virtual ~CMediaBin();

	bool ContainElement(CMediaElement *elemnet);
	bool AddElment(CMediaElement *elemnet);
	bool RemoveElement(CMediaElement *elemnet);
	bool LinkElement(CMediaElement *up, CMediaElement *down);
	void UnLinkElement(CMediaElement *up, CMediaElement *down);

	virtual void SetState(MediaElementState state);

	virtual void RemoveMessageBus();

private:
	void Init(const std::string &name);
	void Prepare();
	void Ready();
	void SortElement();
	void Open();
	void Close();

protected:
	CMediaBus *m_defaultBus;
private:
	static short m_binID;
	unsigned short m_nChildCnt;
	std::list<CMediaElement*> m_childElements;
};



class CMediaBinFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CMediaBin();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CMediaBin(name);
	}
};


#endif  //_MEDIA_BIN_H_