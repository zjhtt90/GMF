#ifndef _MEDIA_ELEMENT_H_
#define _MEDIA_ELEMENT_H_

#include "MediaDefs.h"
#include "MediaPort.h"

#include "IElementFactory.h"

#include "MediaBus.h"

#include "../common/SMutex.h"

#include <map>
#include <vector>
#define MAX_SIDE_PORT	8

class API_EXPORT CElementDescription
{
	friend std::ostream& operator<< (std::ostream &os, const CElementDescription &desc);

public:
	CElementDescription();
	CElementDescription(ElementType type, std::string name, unsigned inCount, unsigned outCount);
	~CElementDescription();

	void SetElementType(ElementType type);
	ElementType GetType();
	void SetElementName(const std::string &name);
	const std::string& GetName();
	void SetElementPortCount(unsigned inCount, unsigned outCount);
	void GetElementPortCount(unsigned &inCount, unsigned &outCount);
	void SetExternData(const MetaData &data);
	bool ContainKey(const std::string &key);
	int GetMetaData(const std::string &key, MetaData &data);
	int UpdateMetaData(const MetaData &data);
	std::map<std::string, MetaData>& GetExternDatas();
	void Print();

private:
	ElementType m_type;
	std::string	m_eleName;
	unsigned m_inPortCount;
	unsigned m_outPortCount;
	std::map<std::string, MetaData> m_exDatas;
};




class API_EXPORT CMediaElement : public IBusMessageCallback
{
public:
	CMediaElement();
	virtual ~CMediaElement();
	ElementType GetType();
	const std::string& GetName();
	const CElementDescription& GetDescription() const;
	int GetPrivateData(MetaData &data);
	void SetPrivateData(const MetaData &data);
	int UpdatePrivateData(const MetaData &data);

	virtual void UseParame(const MetaData &data) {}

	virtual void SetState(MediaElementState state);

	MediaElementState GetCurrentState();

	void GetElementPortCount(unsigned &inCount, unsigned &outCount);

	CMediaPort* GetMediaPortWithIndex(PortDirection dir, TRACKID index);

	int GetMediaPortWithType(PortDirection dir, MediaType type, std::vector<CMediaPort*> &ports);

	bool LinkElement(CMediaElement *dst);
	void UnLinkElement();

	void SetMessageBus(CMediaBus *bus);
	virtual void RemoveMessageBus();
	CMediaBus* GetMessageBus();

	void SetParentNode(CMediaElement *parent);
	CMediaElement* GetParentNode();
	CMediaElement* GetPipeline();

	int PostMessage(const MediaMessage &msg);

	int SetPortExternData(PortDirection dir, TRACKID index, const MetaData &data);

	virtual void PortSettingChanged(TRACKID index, const std::string &key) 
	{

	}

	virtual void MessageHandle(void *sender, const MediaMessage &msg)
	{

	}

protected:
	CElementDescription m_desc;
	CMediaPort* m_inPorts[MAX_SIDE_PORT];
	CMediaPort* m_outPorts[MAX_SIDE_PORT];

	MediaElementState m_curState;
	MediaElementState m_pendingState;
	CSMutex m_stateLock;

	CMediaBus *m_bus;

	CMediaElement *m_parentNode;

public:
	CMediaElement *m_prevElement;
	CMediaElement *m_nextElement;
};


#endif  //_MEDIA_ELEMENT_H_