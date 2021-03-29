#include "MediaElement.h"

#include "LogManager.h"

#include <ostream>
#include <iomanip>
#include <sstream>

CElementDescription::CElementDescription() 
	: m_type(ELEMENT_TYPE_NONE), m_eleName(""), m_inPortCount(0), m_outPortCount(0) 
{

}

CElementDescription::CElementDescription(ElementType type, std::string name, unsigned inCount, unsigned outCount) 
	: m_type(type), m_eleName(name), m_inPortCount(inCount), m_outPortCount(outCount)
{

}

CElementDescription::~CElementDescription()
{
	m_exDatas.clear();
}

void CElementDescription::SetElementType(ElementType type) 
{
	m_type = type;
}

ElementType CElementDescription::GetType()
{
	return m_type;
}

void CElementDescription::SetElementName(const std::string &name) 
{ 
	m_eleName = name; 
}

const std::string& CElementDescription::GetName() 
{
	return m_eleName; 
}

void CElementDescription::SetElementPortCount(unsigned inCount, unsigned outCount)
{ 
	m_inPortCount = inCount; 
	m_outPortCount = outCount; 
}

void CElementDescription::GetElementPortCount(unsigned &inCount, unsigned &outCount) 
{
	inCount = m_inPortCount;
	outCount = m_outPortCount;
}

void CElementDescription::SetExternData(const MetaData &data) 
{
	m_exDatas[data.mKey] = data;
}

bool CElementDescription::ContainKey(const std::string &key) 
{
	return (m_exDatas.find(key) != m_exDatas.end());
}

int CElementDescription::GetMetaData(const std::string &key, MetaData &data)
{
	std::map<std::string, MetaData>::iterator ite = m_exDatas.find(key);
	if(ite != m_exDatas.end())
	{
		data = (*ite).second;
		return MEDIA_ERR_NONE;
	}

	return MEDIA_ERR_NOT_FOUND;
}

int CElementDescription::UpdateMetaData(const MetaData &data)
{
	std::map<std::string, MetaData>::iterator ite = m_exDatas.find(data.mKey);
	if(ite != m_exDatas.end())
	{
		(*ite).second = data;
		return MEDIA_ERR_NONE;
	}

	return MEDIA_ERR_NOT_FOUND;
}

std::map<std::string, MetaData>& CElementDescription::GetExternDatas()
{
	return m_exDatas;
}

void CElementDescription::Print()
{
	std::ostringstream oss;
	oss << std::endl;
	oss << m_eleName << ":" << std::endl;
	oss << "Type: " << m_type << std::endl;
	oss << "Port Count: " << "(" << m_inPortCount << "," << m_outPortCount << ")" << std::endl;
	std::map<std::string, MetaData>::const_iterator ite = m_exDatas.cbegin();
	for(; ite != m_exDatas.cend(); ite++)
	{
		MetaData data = (*ite).second;
		oss << data.mKey << ": " << data.mValue << std::endl;
	}

	std::string str = oss.str();
	LOG_DEBUG(str.c_str());
}

std::ostream& operator<< (std::ostream &os, const CElementDescription &desc)
{
	os << desc.m_eleName << ":" << std::endl;
	os << std::setw(4);
	os << "Type: " << desc.m_type << std::endl;
	os << "Port Count: " << "(" << desc.m_inPortCount << "," << desc.m_outPortCount << ")" << std::endl;
	std::map<std::string, MetaData>::const_iterator ite = desc.m_exDatas.cbegin();
	for(; ite != desc.m_exDatas.cend(); ite++)
	{
		MetaData data = (*ite).second;
		os << data.mKey << ": " << data.mValue << std::endl;
	}
	return os;
}





CMediaElement::CMediaElement() : m_bus(NULL), m_parentNode(NULL), m_errCode(0)
{
	for(int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_inPorts[i] = NULL;
		m_outPorts[i] = NULL;
	}

	m_curState = MEDIA_ELEMENT_STATE_NONE;
	m_pendingState = MEDIA_ELEMENT_STATE_NONE;

	m_prevElement = NULL;
	m_nextElement = NULL;
}

CMediaElement::~CMediaElement()
{
	for(int i = 0; i < MAX_SIDE_PORT; i++)
	{
		if(m_inPorts[i] != NULL)
		{
			delete m_inPorts[i];
			m_inPorts[i] = NULL;
		}

		if(m_outPorts[i] != NULL)
		{
			delete m_outPorts[i];
			m_outPorts[i] = NULL;
		}
	}
}

ElementType CMediaElement::GetType() 
{ 
	return m_desc.GetType();
}

const std::string& CMediaElement::GetName() 
{ 
	return m_desc.GetName(); 
}

const CElementDescription& CMediaElement::GetDescription() const
{
	return m_desc;
}

int CMediaElement::GetPrivateData(MetaData &data)
{
	return m_desc.GetMetaData(data.mKey, data);
}

void CMediaElement::SetPrivateData(const MetaData &data)
{
	m_desc.SetExternData(data);
}

int CMediaElement::UpdatePrivateData(const MetaData &data)
{
	return m_desc.UpdateMetaData(data);
}

void CMediaElement::SetState(MediaElementState state)
{
	m_stateLock.Lock();
	m_pendingState = state;
	m_stateLock.UnLock();
}

MediaElementState CMediaElement::GetCurrentState()
{
	MediaElementState s = MEDIA_ELEMENT_STATE_NONE;
	m_stateLock.Lock();
	s = m_curState;
	m_stateLock.UnLock();

	return s;
}

void CMediaElement::GetElementPortCount(unsigned &inCount, unsigned &outCount) 
{
	m_desc.GetElementPortCount(inCount, outCount);
}

CMediaPort* CMediaElement::GetMediaPortWithIndex(PortDirection dir, TRACKID index)
{
	if(dir == PORT_DIR_IN)
	{
		return m_inPorts[index];
	}
	else if(dir == PORT_DIR_OUT)
	{
		return m_outPorts[index];
	}

	return NULL;
}

int CMediaElement::SetPortExternData(PortDirection dir, TRACKID index, const MetaData &data)
{
	CMediaPort *port = GetMediaPortWithIndex(dir, index);
	if(port == NULL)
	{
		LOG_ERR("Invalite port(%d) index(%d)", dir, index);
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	port->SetProperty(data);
}

int CMediaElement::GetMediaPortWithType(PortDirection dir, MediaType type, std::vector<CMediaPort*> &ports)
{
	unsigned int tCnt = 0;
	CMediaPort **tPorts = NULL;
	unsigned int InCnt = 0;
	unsigned int OutCnt = 0;

	this->GetElementPortCount(InCnt, OutCnt);

	if(dir == PORT_DIR_IN)
	{
		tCnt = InCnt;
		tPorts = m_inPorts;
	}
	else if(dir == PORT_DIR_OUT)
	{
		tCnt = OutCnt;
		tPorts = m_outPorts;
	}


	{
		std::string media = "";
		switch(type)
		{
		case MEDIA_TYPE_AUDIO:
			media = "Audio";
			break;
		case MEDIA_TYPE_VIDEO:
			media = "Video";
			break;
		case MEDIA_TYPE_SUBTITLE:
			media = "Subtitle";
			break;
		case MEDIA_TYPE_DATA:
			media = "Data";
			break;
		default:
			media = "Unknow";
			break;
		}

		for(int i = 0; i < tCnt; i++)
		{
			MetaData data;
			if(tPorts[i]->GetProperty("Media", data) == MEDIA_ERR_NONE)
			{
				if(data.mValue == media)
				{
					ports.push_back(tPorts[i]);
				}
			}
			else if(!tPorts[i]->IsLinked())
			{
				ports.push_back(tPorts[i]);
			}
		}
	}

	if(ports.empty())
		return MEDIA_ERR_NOT_FOUND;

	return MEDIA_ERR_NONE;
}

bool CMediaElement::LinkElement(CMediaElement *dst)
{
	int type;
	for(type =MEDIA_TYPE_VIDEO; type <= MEDIA_TYPE_SUBTITLE; type++)
	{
		std::vector<CMediaPort*> upVidPorts;
		if(this->GetMediaPortWithType(PORT_DIR_OUT, (MediaType)type, upVidPorts) == MEDIA_ERR_NONE)
		{
			std::vector<CMediaPort*> downVidPorts;
			if(dst->GetMediaPortWithType(PORT_DIR_IN, (MediaType)type, downVidPorts) == MEDIA_ERR_NONE)
			{
				int linkCnt = (upVidPorts.size() < downVidPorts.size()) ? upVidPorts.size() : downVidPorts.size(); 
				for(int i = 0; i < linkCnt; i++)
				{
					if(!upVidPorts[i]->IsLinked() && !downVidPorts[i]->IsLinked())
						upVidPorts[i]->LinkPeerPort(downVidPorts[i]);
				}
			}
		}
	}

	this->m_nextElement = dst;
	dst->m_prevElement = this;

	return true;
}

void CMediaElement::UnLinkElement()
{
	unsigned int inCnt, outCnt = 0;
	this->GetElementPortCount(inCnt, outCnt);

	for(int i = 0; i < outCnt; i++)
	{
		CMediaPort *port = this->GetMediaPortWithIndex(PORT_DIR_OUT, i);
		port->UnLinkPeerPort();
	}

	this->m_nextElement->m_prevElement = NULL;
	this->m_nextElement = NULL;
}

void CMediaElement::SetMessageBus(CMediaBus *bus)
{
	if(m_bus != NULL && m_bus == bus)
		return;

	m_bus = bus;
	if(bus != NULL)
	{
		bus->Ref();
		bus->AttachElement(this);
	}
}

void CMediaElement::RemoveMessageBus()
{
	if(m_bus != NULL)
	{
		m_bus->DetachElement(this);
		m_bus->UnRef();
		m_bus = NULL;
	}
}

CMediaBus* CMediaElement::GetMessageBus()
{
	return m_bus;
}

int CMediaElement::PostMessage(const MediaMessage &msg)
{
	if(m_bus == NULL)
	{
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	m_bus->PushMessage(msg);

	return MEDIA_ERR_NONE;
}

void CMediaElement::SetParentNode(CMediaElement *parent)
{
	m_parentNode = parent;
}

CMediaElement* CMediaElement::GetParentNode()
{
	return m_parentNode;
}

CMediaElement* CMediaElement::GetPipeline()
{
	CMediaElement *ele = this;
	CMediaElement *last = NULL;

	while(ele != NULL)
	{
		last = ele;
		ele = ele->GetParentNode();
	}

	if(last != NULL && last->GetType() == ELEMENT_TYPE_PIPELINE)
	{
		return last;
	}

	return NULL;
}