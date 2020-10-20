#include "MediaBin.h"
#include "../MirrorPort.h"

#include "../../Common/LogManager.h"
#include "../../common/Util.h"

#include <assert.h>

#define LOG_FILTER	"MediaBin"

short CMediaBin::m_binID = 0;
CMediaBin::CMediaBin() : m_defaultBus(NULL), m_nChildCnt(0)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_binID;
	std::string name = "MediaBin" + ss.str();	

	Init(name);
}

CMediaBin::CMediaBin(const std::string &name) : m_defaultBus(NULL), m_nChildCnt(0)
{
	Init(name);
}

CMediaBin::~CMediaBin()
{

	RemoveMessageBus();

	if(m_defaultBus != NULL)
	{
		delete m_defaultBus;
		m_defaultBus = NULL;
	}

	std::list<CMediaElement*>::const_iterator ite = m_childElements.cbegin();
	for(; ite != m_childElements.cend(); ite++)
	{
		delete *ite;
	}

	m_childElements.clear();


	m_binID--;
	assert(m_binID >= 0);
}

void CMediaBin::Init(const std::string &name)
{
	m_binID++;


	m_desc.SetElementName(name);

	MetaData data(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(data);
}

bool CMediaBin::ContainElement(CMediaElement *elemnet)
{
	std::list<CMediaElement*>::const_iterator ite = m_childElements.cbegin();
	for(; ite != m_childElements.cend(); ite++)
	{
		if(*ite == elemnet)
			return true;
	}

	return false;
}

bool CMediaBin::AddElment(CMediaElement *elemnet)
{
	if(ContainElement(elemnet))
		return false;
	
	elemnet->SetMessageBus(m_defaultBus);
	m_childElements.push_back(elemnet);

	elemnet->SetParentNode(this);

	m_nChildCnt++;

	return true;
}

bool CMediaBin::RemoveElement(CMediaElement *elemnet)
{
	if(!ContainElement(elemnet))
		return false;

	elemnet->RemoveMessageBus();
	m_childElements.remove(elemnet);
	m_nChildCnt--;

	return true;
}

bool CMediaBin::LinkElement(CMediaElement *up, CMediaElement *down)
{
	if(!ContainElement(up) || !ContainElement(down))
		return false;

	return up->LinkElement(down);
}

void CMediaBin::UnLinkElement(CMediaElement *up, CMediaElement *down)
{
	up->UnLinkElement();
}

void CMediaBin::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_PREPARE:
		this->Prepare();
		break;
	case MEDIA_ELEMENT_STATE_READY:
		this->Ready();
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		this->Open();
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		this->Close();
		break;
	default:
		break;
	}
}

void CMediaBin::Prepare()
{
	if(this->GetType() == ELEMENT_TYPE_PIPELINE)
	{
		return;
	}

	MetaData data;
	m_desc.GetMetaData(META_KEY_ELEMENT_LAYOUT, data);
	if(data.mValue == ELEMENT_LAYOUT_VERTICAL)
	{
		CMediaElement *first = m_childElements.front();
		ElementType type = first->GetType();

		m_desc.SetElementType(type);

		unsigned int tinCnt = 0, toutCnt = 0;
		for(std::list<CMediaElement*>::iterator ite = m_childElements.begin();
			ite != m_childElements.end(); ite++)
		{
			unsigned int inCnt = 0, outCnt = 0;
			CMediaElement *ele = *ite;
			unsigned int i = 0;
			ele->GetElementPortCount(inCnt, outCnt);
			for(i = 0; i < inCnt; i++)
			{
				m_inPorts[tinCnt] = new CMirrorPort(ele->GetMediaPortWithIndex(PORT_DIR_IN, i), this, PORT_DIR_IN, tinCnt);
				tinCnt++;
			}

			for(i = 0; i < outCnt; i++)
			{
				m_outPorts[toutCnt] = new CMirrorPort(ele->GetMediaPortWithIndex(PORT_DIR_OUT, i), this, PORT_DIR_OUT, toutCnt);
				toutCnt++;
			}

			//ele->SetState(MEDIA_ELEMENT_STATE_PREPARE);
		}

		m_desc.SetElementPortCount(tinCnt, toutCnt);

	}
	else
	{
		SortElement();
		CMediaElement *first = m_childElements.front();
		CMediaElement *last = m_childElements.back();
		if(first->GetType() == ELEMENT_TYPE_SRC 
			&& last->GetType() == ELEMENT_TYPE_SINK)
		{
			m_desc.SetElementType(ELEMENT_TYPE_PIPELINE);
		}
		else if(first->GetType() == ELEMENT_TYPE_SRC 
			&& last->GetType() != ELEMENT_TYPE_SINK)
		{
			m_desc.SetElementType(ELEMENT_TYPE_SRC);
		}
		else if(first->GetType() != ELEMENT_TYPE_SRC
			&& last->GetType() == ELEMENT_TYPE_SINK)
		{
			m_desc.SetElementType(ELEMENT_TYPE_SINK);
		}
		else if(first->GetType() != ELEMENT_TYPE_SRC
			&& last->GetType() != ELEMENT_TYPE_SINK)
		{
			m_desc.SetElementType(ELEMENT_TYPE_FILTER);
		}

		unsigned int inCnt, outCnt = 0;
		unsigned int tinCnt, toutCnt = 0;
		unsigned int i = 0;
		first->GetElementPortCount(inCnt, outCnt);
		for(i = 0; i < inCnt; i++)
		{
			m_inPorts[i] = new CMirrorPort(first->GetMediaPortWithIndex(PORT_DIR_IN, i), this, PORT_DIR_IN, i);
		}
		tinCnt = inCnt;

		last->GetElementPortCount(inCnt, outCnt);
		for(i = 0; i < outCnt; i++)
		{
			m_outPorts[i] = new CMirrorPort(last->GetMediaPortWithIndex(PORT_DIR_OUT, i), this, PORT_DIR_OUT, i);
		}

		toutCnt = outCnt;

		m_desc.SetElementPortCount(tinCnt, toutCnt);

		
		for(std::list<CMediaElement*>::iterator ite = m_childElements.begin(); 
			ite != m_childElements.end(); ite++)
		{
			(*ite)->SetState(MEDIA_ELEMENT_STATE_PREPARE);
		}
	}
}

void CMediaBin::Ready()
{
	CMediaBus *bus = GetMessageBus();

	for(std::list<CMediaElement*>::iterator ite = m_childElements.begin(); 
		ite != m_childElements.end(); ite++)
	{
		if(bus != m_defaultBus)
		{
			(*ite)->SetMessageBus(bus);
		}

		(*ite)->SetState(MEDIA_ELEMENT_STATE_READY);
	}

}


void CMediaBin::SortElement()
{
	CMediaElement *first = m_childElements.front();
	while (first->m_prevElement != NULL)
	{
		first = first->m_prevElement;
	}

	m_childElements.clear();

	while(first != NULL)
	{
		m_childElements.push_back(first);
		first = first->m_nextElement;
	}
}


void CMediaBin::Open()
{
	LOG_INFO_S("Start "+m_desc.GetName());
	MetaData data;
	m_desc.GetMetaData(META_KEY_ELEMENT_LAYOUT, data);
	if(data.mValue == ELEMENT_LAYOUT_VERTICAL)
	{
		for(std::list<CMediaElement*>::iterator ite = m_childElements.begin();
			ite != m_childElements.end(); ite++)
		{
			CMediaElement *ele = *ite;
			ele->SetState(MEDIA_ELEMENT_STATE_OPEN);
		}
	}
	else
	{
		CMediaElement *ele = m_childElements.back();
		while(ele != NULL)
		{
			ele->SetState(MEDIA_ELEMENT_STATE_OPEN);
			ele = ele->m_prevElement;
		}
	}
}

void CMediaBin::Close()
{
	LOG_INFO_S("Stop "+m_desc.GetName());
	MetaData data;
	m_desc.GetMetaData(META_KEY_ELEMENT_LAYOUT, data);
	if(data.mValue == ELEMENT_LAYOUT_VERTICAL)
	{
		for(std::list<CMediaElement*>::iterator ite = m_childElements.begin();
			ite != m_childElements.end(); ite++)
		{
			CMediaElement *ele = *ite;
			ele->SetState(MEDIA_ELEMENT_STATE_STOP);
		}
	}
	else
	{
		CMediaElement *ele = m_childElements.back();
		while(ele != NULL)
		{
			ele->SetState(MEDIA_ELEMENT_STATE_STOP);
			ele = ele->m_prevElement;
		}
	}
}

void CMediaBin::RemoveMessageBus()
{
	for(std::list<CMediaElement*>::iterator ite = m_childElements.begin();
		ite != m_childElements.end(); ite++)
	{
		CMediaElement *ele = *ite;
		ele->RemoveMessageBus();
	}

	m_bus->UnRef();
}