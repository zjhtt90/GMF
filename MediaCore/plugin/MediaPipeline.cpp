#include "MediaPipeline.h"

#include "../../Common/LogManager.h"
#include "../../common/Util.h"

#include <assert.h>

#define LOG_FILTER	"Pipeline"

short CMediaPipeline::m_pipeID = 0;

CMediaPipeline::CMediaPipeline()
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_pipeID;
	std::string name = "Pipeline" + ss.str();	

	Init(name);
}

CMediaPipeline::CMediaPipeline(const std::string &name)
{
	Init(name);
}

CMediaPipeline::~CMediaPipeline()
{
	/*
	RemoveMessageBus();

	if(m_defaultBus != NULL)
	{
		delete m_defaultBus;
		m_defaultBus = NULL;
	}
	*/
	m_pipeID--;
	assert(m_pipeID >= 0);
}

void CMediaPipeline::Init(const std::string &name)
{
	m_pipeID++;

	m_pWatcher = NULL;

	m_defaultBus = new CMediaBus();

	SetMessageBus(m_defaultBus);

	m_desc.SetElementName(name);
	m_desc.SetElementType(ELEMENT_TYPE_PIPELINE);

	MetaData data(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(data);
}

void CMediaPipeline::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		break;
	case MEDIA_MESSAGE_REPLY:
		break;
	case MEDIA_MESSAGE_NOTIFY:
		{
			if(m_pWatcher)
				m_pWatcher->MessageProc(msg.m_subType, msg.m_content.msgFileds);
		}
		break;
	default:
		break;
	}
}

void CMediaPipeline::SetMessageWatcher(MessageWatcher *watcher)
{
	m_pWatcher = watcher;
}