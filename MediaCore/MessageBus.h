#ifndef _MESSAGE_BUS_H_
#define _MESSAGE_BUS_H_

#include <deque>
#include <list>

#include "../Common/SThread.h"
#include "../Common/SMutex.h"
#include "../Common/SEvent.h"
#include "MediaMessage.h"

class CMediaElement;

class IBusMessageCallback
{
public:
	virtual void MessageHandle(void *sender, const MediaMessage &msg) = 0;
};


class CMessageBus : public CSThread
{
public:
	CMessageBus();
	~CMessageBus();

	void PushMessage(const MediaMessage &msg);
	MediaMessage PopMessage();
	void AttachElement(CMediaElement *ele);
	void DetachElement(CMediaElement *ele);


private:
	virtual void Run();

	void CleanUp();

private:
	std::deque<MediaMessage> m_msgQueue;
	CSMutex m_queueLock;

	CSEvent m_event;

	std::list<CMediaElement*> m_elements;

	bool m_bExit;
};


#endif  //_MESSAGE_BUS_H_