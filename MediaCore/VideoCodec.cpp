#include "VideoCodec.h"
#include "Util.h"


CVideoCodec::CVideoCodec()
	: m_codecID(0), m_width(0), m_height(0), m_pixformat(0),
	m_srcFps(30), m_dstFps(30)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName("VideoCodec");

	m_desc.SetElementPortCount(1, 1);

	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_inPorts[0]->SetProperty(data);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);
	m_outPorts[0]->SetProperty(data);

}


CVideoCodec::~CVideoCodec()
{
}


void CVideoCodec::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		//m_outPorts[0]->UpdateProperty(MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING));
		m_outPorts[0]->SetProperty(MetaData(META_KEY_CODEC_ID, CUtil::convert<std::string, int>(m_codecID), META_DATA_VAL_TYPE_INT));

		if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
		{
			m_width = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
		{
			m_height = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_pixformat = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_FPS, data) == MEDIA_ERR_NONE)
		{
			m_dstFps = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}

	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(Open() == MEDIA_ERR_NONE)
		{
			//m_outPorts[0]->StartTask();
			m_curState = MEDIA_ELEMENT_STATE_RUNNING;
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		//m_outPorts[0]->PauseTask();
		m_curState = MEDIA_ELEMENT_STATE_PAUSED;
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		//m_outPorts[0]->StopTask();
		m_curState = MEDIA_ELEMENT_STATE_STOPPED;
		Close();
	}
}

int CVideoCodec::Open()
{
	return MEDIA_ERR_NONE;
}

void CVideoCodec::Close()
{

}

void CVideoCodec::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[0]->GetProperty(key, data);
	if(key == META_KEY_VIDEO_FPS)
	{
		m_srcFps = CUtil::convert<int, std::string>(data.mValue);
	}
	else
	{
		m_desc.SetExternData(data);
		if(m_outPorts[0])
			m_outPorts[0]->SetProperty(data);
	}
}

int CVideoCodec::ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf)
{
	return MEDIA_ERR_NONE;
}

void CVideoCodec::ExportCodecInfo(std::vector<MetaData>& infos)
{
	std::map<std::string, MetaData>& exDatas = m_desc.GetExternDatas();

	if(exDatas.empty())
	{
		return;
	}

	char codecid[16] = {0};
	snprintf(codecid, 16, "%d", m_codecID);
	infos.push_back(MetaData(META_KEY_CODEC_ID, codecid, META_DATA_VAL_TYPE_INT));

	std::map<std::string, MetaData>::iterator ite = exDatas.begin();
	for(; ite != exDatas.end(); ite++)
	{
		infos.push_back(ite->second);
	}

}