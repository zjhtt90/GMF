#include "FileSrc.h"
#include "../../Common/LogManager.h"
#include "../MediaBuffer.h"
#include "../../common/Util.h"
#include "../MediaMessage.h"

#include <assert.h>

#define LOG_FILTER	"FileSrc"

#define TRUNK_SIZE	1024

short CFileSrc::m_ID = 0;

CFileSrc::CFileSrc() : m_pFilePtr(NULL), m_lReadTrunkSize(TRUNK_SIZE)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_ID;
	std::string name = "FileSrc" + ss.str();	

	Init(name);
}

CFileSrc::CFileSrc(const std::string &name) : m_pFilePtr(NULL), m_lReadTrunkSize(TRUNK_SIZE)
{
	Init(name);
}

CFileSrc::~CFileSrc()
{
	m_ID--;
	assert(m_ID >= 0);
}

void CFileSrc::Init(const std::string &name)
{
	m_ID++;


	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);

}

void CFileSrc::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		m_desc.GetMetaData(META_KEY_MEDIA, data);
		m_outPorts[0]->SetProperty(data);
		if(data.mValue == "Video")
		{
			if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
			if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
			if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
		}
		else if(data.mValue == "Audio")
		{
			//TODO for audio param
		}

		//m_desc.Print();
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(Open() == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->StartTask();
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();

		Close();
	}
}

int CFileSrc::Open()
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
		m_pFilePtr = fopen(data.mValue.c_str(), "rb");
		if(!m_pFilePtr)
		{
			LOG_ERR("Open %s failed!", data.mValue.c_str());
			return MEDIA_ERR_NOT_FOUND;
		}
	}

	if(m_desc.GetMetaData(META_KEY_TRUNK_SIZE, data) == MEDIA_ERR_NONE)
	{
		m_lReadTrunkSize = CUtil::convert<unsigned long, std::string>(data.mValue);
	}

	return res;
}

int CFileSrc::Open(const std::string &uri)
{
	int res = MEDIA_ERR_NONE;
	MetaData data(META_KEY_FILE_PATH, uri, META_DATA_VAL_TYPE_STRING);
	if(m_desc.UpdateMetaData(data) == MEDIA_ERR_NOT_FOUND)
	{
		m_desc.SetExternData(data);
	}

	return this->Open();
}

void CFileSrc::Close()
{
	if(m_pFilePtr != NULL)
	{
		fclose(m_pFilePtr);
		m_pFilePtr = NULL;
	}
}

int CFileSrc::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int res = MEDIA_ERR_NONE;

	id = 0;
	if(m_pFilePtr != NULL)
	{
		if(feof(m_pFilePtr))
		{
			MediaMessage msg(MEDIA_MESSAGE_NOTIFY, MESSAGE_SUB_TYPE_END_OF_STREAM, this, GetPipeline());
			PostMessage(msg);

			return MEDIA_ERR_END_OF_STREAM;
		}

		unsigned char *buf = new unsigned char[m_lReadTrunkSize];
		size_t len = fread(buf, 1, m_lReadTrunkSize, m_pFilePtr);
		if(len > 0)
			*buffer = new CMediaBuffer(buf, len, 0, 0, 0);
		else
			res = MEDIA_ERR_READ_FAILED;

		delete buf;
	}

	return res;
}