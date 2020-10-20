#ifndef _MEDIA_MESSAGE_H_
#define _MEDIA_MESSAGE_H_

#include "MediaDefs.h"

#include <vector>

class MediaMessage
{
public:
	MediaMessage() : m_type(MEDIA_MESSAGE_NONE), m_subType(MESSAGE_SUB_TYPE_NONE), m_src(NULL), m_dst(NULL)
	{

	}

	MediaMessage(MediaMessageType type, int subtype, void *src, void *dst)
		: m_type(type), m_subType(subtype), m_src(src), m_dst(dst)
	{

	}

	~MediaMessage() { }


	struct MSGContent
	{
		std::vector<MetaData> msgFileds;
	};


	MediaMessageType m_type;
	int m_subType;
	unsigned long long m_timestamp;
	void *m_src;
	void *m_dst;
	unsigned int m_seqnum;

	MSGContent m_content;
};


#endif  //_MEDIA_MESSAGE_H_