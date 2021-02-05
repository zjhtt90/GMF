#include "FFMediaStorage.h"
#include "common.h"
#include "MediaBuffer.h"
#include "Util.h"
#include "LogManager.h"
#include <assert.h>

#define LOG_FILTER	"FFMediaStorage"

CFFMediaStorage::CFFMediaStorage() : m_iPortCnt(0), m_pOFmtCtx(NULL)
{
	Init("FFMediaStorage");
}

CFFMediaStorage::CFFMediaStorage(const std::string &name) : m_iPortCnt(0), m_pOFmtCtx(NULL)
{
	Init(name);
}

CFFMediaStorage::~CFFMediaStorage()
{

}

void CFFMediaStorage::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);

	for(int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_iMediaInfo[i] = NULL;
	}


}

void CFFMediaStorage::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if(data.mKey == META_KEY_INPORT_COUNT)
	{
		m_iPortCnt = CUtil::convert<int, std::string>(data.mValue);		
	}
	for(int i = 0; i < m_iPortCnt; i++)
	{
		m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
	}


	m_desc.SetElementPortCount(m_iPortCnt, 0);
}

void CFFMediaStorage::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{

		}
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		break;
	default:
		break;
	}
}

void CFFMediaStorage::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{

}

void CFFMediaStorage::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);

	if(key == META_KEY_MEDIA)
	{
		MediaType type = MediaStrToType(data.mValue);
		if(m_iMediaInfo[index] == NULL)
		{
			if(type == MEDIA_TYPE_VIDEO)
			{
				m_iMediaInfo[index] = new VMetaInfo();
			}
			else if(type == MEDIA_TYPE_AUDIO)
			{
				m_iMediaInfo[index] = new AMetaInfo();
			}
		}
	}
	else if(key == META_KEY_VIDEO_WIDTH)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mWidth = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_VIDEO_HEIGHT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mHeight = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_PIX_FORMAT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mPixfmt = FFPixFormat((VideoPixformat)CUtil::convert<int, std::string>(data.mValue));
		}
	}
	else if(key == META_KEY_VIDEO_FPS)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((VMetaInfo*)m_iMediaInfo[index])->mfps = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_SAMPLE_RATE)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplerate = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_SAMPLE_FORMAT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplefmt = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_CHANNEL_NUM)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mChannel = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(key == META_KEY_FRAME_SAMPLE_COUNT)
	{
		if(m_iMediaInfo[index] != NULL)
		{
			((AMetaInfo*)m_iMediaInfo[index])->mSamplecnt = CUtil::convert<int, std::string>(data.mValue);
		}
	}
}

void CFFMediaStorage::MessageHandle(void *sender, const MediaMessage &msg)
{
	switch(msg.m_type)
	{
	case MEDIA_MESSAGE_QUERY:
		break;
	case MEDIA_MESSAGE_REPLY:
		{
			if(msg.m_subType == MESSAGE_SUB_TYPE_CODEC_INFO)
			{

			}
		}
		break;
	case MEDIA_MESSAGE_NOTIFY:
		break;
	default:
		break;
	}
}


void CFFMediaStorage::AddStream(TRACKID index)
{
	MediaMessage msg(MEDIA_MESSAGE_QUERY, MESSAGE_SUB_TYPE_CODEC_INFO, this, NULL);
	msg.m_seqnum = index;

	m_inPorts[index]->GetPropertyDatas(msg.m_content.msgFileds);

	switch(m_iMediaInfo[index]->mType)
	{
	case MEDIA_TYPE_VIDEO:
		break;
	case MEDIA_TYPE_AUDIO:
		break;
	default:
		break;
	}
	
	if(m_bus != NULL)
	{
		m_bus->PushMessage(msg);
	}


}

int CFFMediaStorage::Open()
{
	int res = MEDIA_ERR_NONE;
	MetaData data;
	res = m_desc.GetMetaData(META_KEY_FILE_PATH, data);
	if (res != MEDIA_ERR_NONE)
	{
		LOG_ERR("Open error(%d)!", res);
		return res;
	}
	else
	{
		char errMsg[512] = {0};
		res = avformat_alloc_output_context2(&m_pOFmtCtx, NULL, NULL, data.mValue.c_str());
		if (res < 0)
		{
			av_strerror(res, errMsg, 512);
			LOG_ERR("Alloc output ctx error(%d), %s", res, errMsg);
			return res;
		}
		m_pOFmtCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
	}

	return res;
}

void CFFMediaStorage::Close()
{
	if (m_pOFmtCtx)
	{
		if (!(m_pOFmtCtx->flags & AVFMT_NOFILE))
		{
			avio_closep(&m_pOFmtCtx->pb);
		}
		avformat_free_context(m_pOFmtCtx);
		m_pOFmtCtx = NULL;
	}
	
}