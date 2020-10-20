#include "LiveRTSPSink.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/LogManager.h"
#include "../Common/Util.h"

#include "MediaServer.h"


extern "C"
{
#include "libavutil/common.h"
};


#define LOG_FILTER	"LiveRTSPSink"

CLiveRTSPSink::CLiveRTSPSink()
{
	Init("LiveRTSPSink");
}

CLiveRTSPSink::CLiveRTSPSink(const std::string &name)
{
	Init(name);
}

CLiveRTSPSink::~CLiveRTSPSink()
{

}

void CLiveRTSPSink::Init(const std::string &name)
{
	m_sdpMediaCnt = 0;
	m_bGetKeyFrame = false;

	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);

	for (int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_pSrc[i] = NULL;
	}

}

void CLiveRTSPSink::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if(data.mKey == META_KEY_INPORT_COUNT)
	{
		int inPortCnt = CUtil::convert<int, std::string>(data.mValue);

		m_channelCount = inPortCnt;

		for(int i = 0; i < inPortCnt; i++)
		{
			m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
		}

		m_desc.SetElementPortCount(inPortCnt, 0);
	}
}

void CLiveRTSPSink::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{
			
			LOG_DEBUG("-----");
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

int CLiveRTSPSink::Open()
{
	LOG_INFO("Start RTSPSink");
	media_server_init(554, NULL, NULL);

	LiveStreamIO* pVSrc = NULL;
	LiveStreamIO* pASrc = NULL;

	for (int i = 0; i < m_channelCount; i++)
	{
		MediaType type = m_inPorts[i]->GetMediaType();

		if (type == MEDIA_TYPE_VIDEO)
		{
			pVSrc = m_pSrc[i];
			m_pSrc[i]->SetCallback(LiveStreamVSrcCallbackFunc, this);
		}
		else
		{
			pASrc = m_pSrc[i];
			m_pSrc[i]->SetCallback(LiveStreamASrcCallbackFunc, this);
		}
	}

	media_server_add_live("0", pVSrc, pASrc);

	m_curState = MEDIA_ELEMENT_STATE_RUNNING;
	CSThread::Start();

	return MEDIA_ERR_NONE;
}

void CLiveRTSPSink::Close()
{
	LOG_INFO("Stop RTSPSink");

	CSThread::Kill();

	media_server_uninit();

	m_curState = MEDIA_ELEMENT_STATE_STOPPED;
}

void CLiveRTSPSink::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	/*
	if (!m_bGetKeyFrame)
	{
		if (((CVideoBuffer*)buffer)->m_picType == 'I')
		{
			m_bGetKeyFrame = true;
		}
		else
			return;
	}
	*/
	m_pSrc[id]->Write(buffer->GetData(), buffer->GetDataSize());	
}

void CLiveRTSPSink::Run()
{
	while (1)
	{
		//LOG_INFO("current state %d", m_curState);
		if (m_curState == MEDIA_ELEMENT_STATE_RUNNING)
		{
			media_server_run();
		}
		else if (m_curState == MEDIA_ELEMENT_STATE_PAUSED)
		{
			CSThread::Sleep(100);
		}
		else if (m_curState == MEDIA_ELEMENT_STATE_STOPPED)
		{
			break;
		}
	}
}

void CLiveRTSPSink::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);
	if (key == META_KEY_MEDIA)
	{		
		if (MediaStrToType(data.mValue) == MEDIA_TYPE_AUDIO)
		{
			if (m_pSrc[index] == NULL)
			{
				m_pSrc[index] = new LiveStreamIO(10 * 1024);
			}

			m_pSrc[index]->SetExtParam(LiveStreamIO::PARAM_KEY_FORMAT, std::string("pcm"));
		}
		else if(MediaStrToType(data.mValue) == MEDIA_TYPE_VIDEO)
		{
			if (m_pSrc[index] == NULL)
			{
				m_pSrc[index] = new LiveStreamIO(256 * 1024);
			}
		}
	}
	else if (key == META_KEY_SAMPLE_RATE)
	{
		m_pSrc[index]->SetExtParam(LiveStreamIO::PARAM_KEY_AUDIO_SAMPLERATE, CUtil::convert<int, std::string>(data.mValue));
	}
	else if (key == META_KEY_CHANNEL_NUM)
	{
		m_pSrc[index]->SetExtParam(LiveStreamIO::PARAM_KEY_AUDIO_CHANNEL, CUtil::convert<int, std::string>(data.mValue));
	}
	else if (key == META_KEY_SAMPLE_FORMAT)
	{
		int samplefmt = CUtil::convert<int, std::string>(data.mValue);
		int bits = 16;
		switch (samplefmt)
		{
		case AUDIO_SAMPLE_FMT_U8:
			bits = 8;
			break;
		case AUDIO_SAMPLE_FMT_S16:
			bits = 16;
			break;
		case AUDIO_SAMPLE_FMT_S32:
			bits = 32;
			break;
		default:
			break;
		}
		m_pSrc[index]->SetExtParam(LiveStreamIO::PARAM_KEY_AUDIO_BITPERSAMPLE, bits);
	}
}

void CLiveRTSPSink::LiveStreamVSrcCallbackFunc(int state, void* pObj)
{
	CLiveRTSPSink* pThis = reinterpret_cast<CLiveRTSPSink*>(pObj);
	if (state)
	{
		LOG_INFO("video src open!");
		MediaMessage msgReq;
		msgReq.m_type = MEDIA_MESSAGE_QUERY;
		msgReq.m_subType = MESSAGE_SUB_TYPE_FORCE_KEY;
		msgReq.m_src = pThis;
		pThis->m_bus->PushMessage(msgReq);
	}
}

void CLiveRTSPSink::LiveStreamASrcCallbackFunc(int state, void* pObj)
{

}

void CLiveRTSPSink::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		break;
	case MEDIA_MESSAGE_REPLY:
		
		break;
	case MEDIA_MESSAGE_NOTIFY:
		break;
	default:
		break;
	}
}