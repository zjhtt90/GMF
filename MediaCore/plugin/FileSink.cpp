#include "FileSink.h"
#include "../MediaBuffer.h"
#include "../../Common/Util.h"
#include "../../Common/LogManager.h"

#include <assert.h>
#include <sstream>

#define LOG_FILTER	"FileSink"

short CFileSink::m_ID = 0;

CFileSink::CFileSink() : m_pFilePtr(NULL)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_ID;
	std::string name = "FileSink" + ss.str();	

	Init(name);
}

CFileSink::CFileSink(const std::string &name) : m_pFilePtr(NULL)
{
	Init(name);
}

CFileSink::~CFileSink()
{
	m_ID--;
	assert(m_ID >= 0);
}

void CFileSink::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{
			MetaData data;
			if(m_desc.GetMetaData(META_KEY_MEDIA, data) == MEDIA_ERR_NONE)
			{
				m_inPorts[0]->SetProperty(data);
			}
		}
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		Open();
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		Close();
		break;
	default:
		break;
	}
}

int CFileSink::Open()
{
	int res = MEDIA_ERR_NONE;
	MetaData data;
	res = m_desc.GetMetaData(META_KEY_FILE_PATH, data);
	if(res != MEDIA_ERR_NONE)
	{
		LOG_ERR("Open error(%d)!", res);
		return res;
	}
	else
	{
		LOG_DEBUG("Will open %s.", data.mValue.c_str());
		m_pFilePtr = fopen(data.mValue.c_str(), "wb");
		if(!m_pFilePtr)
		{
			LOG_ERR("Open %s failed!", data.mValue.c_str());
			return MEDIA_ERR_NOT_FOUND;
		}
	}

	return res;
}

void CFileSink::Close()
{
	if(m_pFilePtr != NULL)
	{
		fclose(m_pFilePtr);
		m_pFilePtr = NULL;
	}
}

void CFileSink::Init(const std::string &name)
{
	m_ID++;

	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(1, 0);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
}

void CFileSink::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(m_pFilePtr != NULL)
	{
		fwrite(buffer->GetData(), 1, buffer->GetDataSize(), m_pFilePtr);
	}
}
