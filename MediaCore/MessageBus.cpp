#include "MessageBus.h"
#include "MediaElement.h"

CMessageBus::CMessageBus() : m_bExit(false)
{
	CSThread::Start();
}


CMessageBus::~CMessageBus()
{
	CleanUp();
}


void CMessageBus::PushMessage(const MediaMessage &msg)
{
	m_queueLock.Lock();
	m_msgQueue.push_back(msg);
	m_queueLock.UnLock();

	if(m_msgQueue.size() == 1)
	{
		m_event.post();
	}
}

MediaMessage CMessageBus::PopMessage()
{
	MediaMessage msg;
	m_queueLock.Lock();
	if(!m_msgQueue.empty())
	{
		msg = m_msgQueue.front();
		m_msgQueue.pop_front();
	}
	m_queueLock.UnLock();

	return msg;
}

void CMessageBus::AttachElement(CMediaElement *ele)
{
	m_elements.push_back(ele);
}

void CMessageBus::DetachElement(CMediaElement *ele)
{
	m_elements.remove(ele);
}


void CMessageBus::Run()
{
	while(!m_bExit)
	{
		MediaMessage msg = PopMessage();

		if(msg.m_type != MEDIA_MESSAGE_NONE)
		{
			if(msg.m_dst != NULL)
			{
				CMediaElement *ele = reinterpret_cast<CMediaElement*>(msg.m_dst);
				ele->MessageHandle(msg.m_src, msg);
			}
			else
			{
				for(std::list<CMediaElement*>::iterator ite = m_elements.begin(); ite != m_elements.end(); ite++)
				{
					CMediaElement *ele = *ite;
					if(ele != msg.m_src)
					{
						ele->MessageHandle(msg.m_src, msg);
					}
				}
			}
			CSThread::Sleep(1);
		}
		else
		{
			m_event.wait();
		}
	}
}

void CMessageBus::CleanUp()
{
	m_bExit = true;

	m_event.post();

	CSThread::Kill();

	m_msgQueue.clear();

	if(!m_elements.empty())
		m_elements.erase(m_elements.cbegin(), m_elements.cend());
}