#ifndef _MEDIA_BUS_H_
#define _MEDIA_BUS_H_

#include "MessageBus.h"

#define  MESSAGE_TYPE_COUNT	(MEDIA_MESSAGE_NOTIFY+1)

class API_EXPORT CMediaBus
{
public:
	CMediaBus();
	~CMediaBus();

	int PushMessage(const MediaMessage &msg);
	void AttachElement(CMediaElement *ele);
	void DetachElement(CMediaElement *ele);

	void Ref();
	void UnRef();

private:
	CMessageBus m_msgBus[MESSAGE_TYPE_COUNT];

	unsigned int m_refCount;
};


#endif  //_MEDIA_BUS_H_