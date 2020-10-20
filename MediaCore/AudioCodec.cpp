#include "AudioCodec.h"
#include "../Common/Util.h"

CAudioCodec::CAudioCodec() : m_codecID(0), m_sampleRate(44100), m_channel(2), m_sampleFmt(AUDIO_SAMPLE_FMT_S16)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName("AudioCodec");

	m_desc.SetElementPortCount(1, 1);

	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_AUDIO), META_DATA_VAL_TYPE_STRING);
	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_inPorts[0]->SetProperty(data);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);
	m_outPorts[0]->SetProperty(data);
}


CAudioCodec::~CAudioCodec()
{
}

void CAudioCodec::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		//m_outPorts[0]->UpdateProperty(MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_AUDIO), META_DATA_VAL_TYPE_STRING));
		m_outPorts[0]->SetProperty(MetaData(META_KEY_CODEC_ID, CUtil::convert<std::string, int>(m_codecID), META_DATA_VAL_TYPE_INT));

		if(m_desc.GetMetaData(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
		{
			m_sampleRate = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
		{
			m_channel = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_sampleFmt = CUtil::convert<int, std::string>(data.mValue);
		}

	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(Open() == MEDIA_ERR_NONE)
		{
			m_curState = MEDIA_ELEMENT_STATE_RUNNING;
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_curState = MEDIA_ELEMENT_STATE_PAUSED;
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_curState = MEDIA_ELEMENT_STATE_STOPPED;
		Close();
	}
}

int CAudioCodec::Open()
{
	return MEDIA_ERR_NONE;
}

void CAudioCodec::Close()
{

}

void CAudioCodec::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);

	m_desc.SetExternData(data);
	if(m_outPorts[index])
		m_outPorts[index]->SetProperty(data);
}
