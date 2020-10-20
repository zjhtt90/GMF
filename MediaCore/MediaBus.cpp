#include "MediaBus.h"

#include <assert.h>

CMediaBus::CMediaBus() : m_refCount(0)
{
	Ref();
}


CMediaBus::~CMediaBus()
{
	UnRef();

	assert(m_refCount == 0);
}

int CMediaBus::PushMessage(const MediaMessage &msg)
{
	if(msg.m_type == MEDIA_MESSAGE_NONE)
		return MEDIA_ERR_INVALIDE_PARAME;

	m_msgBus[msg.m_type].PushMessage(msg);

	return MEDIA_ERR_NONE;
}

void CMediaBus::AttachElement(CMediaElement *ele)
{
	for(int i = MEDIA_MESSAGE_QUERY; i <= MEDIA_MESSAGE_NOTIFY; i++)
	{
		m_msgBus[i].AttachElement(ele);
	}
}

void CMediaBus::DetachElement(CMediaElement *ele)
{
	for(int i = MEDIA_MESSAGE_QUERY; i <= MEDIA_MESSAGE_NOTIFY; i++)
	{
		m_msgBus[i].DetachElement(ele);
	}
}

void CMediaBus::Ref()
{
	m_refCount++;
}

void CMediaBus::UnRef()
{
	m_refCount--;
}