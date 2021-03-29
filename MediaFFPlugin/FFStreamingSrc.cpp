#include "FFStreamingSrc.h"

#include "common.h"
#include "MediaBuffer.h"
#include "Util.h"
#include "LogManager.h"

extern "C"
{
#include "libavutil/avutil.h"
#include "libavutil/time.h"
};

#define LOG_FILTER	"FFStreamingSrc"

short CFFStreamingSrc::m_eleID = 0;

CFFStreamingSrc::CFFStreamingSrc() : m_fmtCtx(NULL)
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_eleID;
	std::string name = "FFStreamingSrc" + ss.str();
	Init(name);
}

CFFStreamingSrc::CFFStreamingSrc(const std::string &name) : m_fmtCtx(NULL)
{
	Init(name);
}

CFFStreamingSrc::~CFFStreamingSrc()
{

}

void CFFStreamingSrc::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);

	for (int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_oMediaInfo[i] = NULL;
	}

	m_bGetKeyFrame = false;
	m_curPlayTime = 0;
}

void CFFStreamingSrc::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if (data.mKey == META_KEY_OUTPORT_COUNT)
	{
		m_oPortCnt = CUtil::convert<int, std::string>(data.mValue);
	}
	for (int i = 0; i < m_oPortCnt; i++)
	{
		m_outPorts[i] = new CMediaPort(this, PORT_DIR_OUT, i);
	}

	m_desc.SetElementPortCount(0, m_oPortCnt);
}

int CFFStreamingSrc::Open()
{
	LOG_INFO("Start CFFMediaSrc");
	int res = MEDIA_ERR_NONE;
	char errMsg[512] = { 0 };

	m_fmtCtx = avformat_alloc_context();
	if(m_fmtCtx == NULL)
	{
		m_errCode = MEDIA_ERR_MEMALLOC;
		m_errMsg = "Alloc format context error";
		LOG_ERR_S(m_errMsg);
		return m_errCode;
	}
	res = avformat_open_input(&m_fmtCtx, m_strURI.c_str(), NULL, 0);
	if(res < 0)
	{
		av_strerror(res, errMsg, sizeof(errMsg));
		LOG_ERR("Open error(%d): %s!", res, errMsg);
		m_errCode = MEDIA_ERR_OPEN;
		m_errMsg = errMsg;
		LOG_ERR(m_errMsg);
		return m_errCode;
	}

	if ((res = avformat_find_stream_info(m_fmtCtx, NULL)) < 0)
	{
		av_strerror(res, errMsg, sizeof(errMsg));
		LOG_ERR("Failed to retrieve input stream information,errcode(%d), %s", res, errMsg);
		m_errCode = MEDIA_ERR_NOT_FOUND;
		m_errMsg = errMsg;
		LOG_ERR(m_errMsg);
		return m_errCode;
	}

	av_dump_format(m_fmtCtx, 0, m_strURI.c_str(), 0);

	if (m_fmtCtx->nb_streams == 1 && m_fmtCtx->streams[0]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	{
		//audio only
		m_bGetKeyFrame = true;
	}

	for (int i = 0; i < m_fmtCtx->nb_streams; i++)
	{
		AVStream *in_stream = m_fmtCtx->streams[i];
		MediaMetaInfo* pMediaInfo = NULL;
		if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{		
			pMediaInfo = new VMetaInfo();
			((VMetaInfo*)pMediaInfo)->mCodecType = LocalCodecType(in_stream->codecpar->codec_id);
			((VMetaInfo*)pMediaInfo)->mWidth = in_stream->codecpar->width;
			((VMetaInfo*)pMediaInfo)->mHeight = in_stream->codecpar->height;
			((VMetaInfo*)pMediaInfo)->mPixfmt = (int)LocalPixFormat((AVPixelFormat)in_stream->codecpar->format);
			((VMetaInfo*)pMediaInfo)->mfps = av_q2d(in_stream->avg_frame_rate);
			((VMetaInfo*)pMediaInfo)->mStreamID = i;
			((VMetaInfo*)pMediaInfo)->mReverse = in_stream->start_time;
		}
		else if (in_stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			pMediaInfo = new AMetaInfo();
			((AMetaInfo*)pMediaInfo)->mCodecType = LocalCodecType(in_stream->codecpar->codec_id);
			((AMetaInfo*)pMediaInfo)->mSamplerate = in_stream->codecpar->sample_rate;
			((AMetaInfo*)pMediaInfo)->mChannel = in_stream->codecpar->channels;
			((AMetaInfo*)pMediaInfo)->mSamplefmt = in_stream->codecpar->format;
			((AMetaInfo*)pMediaInfo)->mSamplecnt = in_stream->codecpar->frame_size;
			((AMetaInfo*)pMediaInfo)->mStreamID = i;
			((AMetaInfo*)pMediaInfo)->mReverse = in_stream->start_time;
		}

		MetaData data;
		int j;
		for (j = 0; j < m_oPortCnt; j++)
		{
			m_outPorts[j]->GetProperty(META_KEY_MEDIA, data);
			if (MediaStrToType(data.mValue) == in_stream->codecpar->codec_type && m_oMediaInfo[j] == NULL)
				break;
		}
		m_oMediaInfo[j] = pMediaInfo;
	}


	return MEDIA_ERR_NONE;
}

void CFFStreamingSrc::Close()
{
	LOG_INFO("Stop CFFMediaSrc");
	for (int i = 0; i < MAX_SIDE_PORT; i++)
	{
		if (m_oMediaInfo[i] != NULL)
		{
			delete m_oMediaInfo[i];
			m_oMediaInfo[i] = NULL;
		}
	}

	if(m_fmtCtx != NULL)
	{
		avformat_close_input(&m_fmtCtx);

		avformat_free_context(m_fmtCtx);
		m_fmtCtx = NULL;
	}
}

void CFFStreamingSrc::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if(m_desc.GetMetaData(META_KEY_URI, data) == MEDIA_ERR_NONE)
		{
			m_strURI = data.mValue;
			if (Open() != MEDIA_ERR_NONE)
			{
				MediaMessage msg;
				msg.m_type = MEDIA_MESSAGE_NOTIFY;
				msg.m_src = this;
				msg.m_subType = MESSAGE_SUB_TYPE_ERROR;
				msg.m_content.msgFileds.push_back(MetaData("ErrCode",
							CUtil::convert<std::string, int>(m_errCode),
							META_DATA_VAL_TYPE_INT));
				msg.m_content.msgFileds.push_back(MetaData("ErrMsg", m_errMsg,
							META_DATA_VAL_TYPE_STRING));
				PostMessage(msg);
				return;
			}

			for (int i = 0; i < m_oPortCnt; i++)
			{	
				if (m_oMediaInfo[i] == NULL)
				{
					m_outPorts[i]->SetEnable(false);
					break;
				}
				data = MetaData(META_KEY_CODEC_ID, CUtil::convert<std::string, int>(m_oMediaInfo[i]->mCodecType), META_DATA_VAL_TYPE_INT);
				m_outPorts[i]->SetProperty(data);

				if (m_oMediaInfo[i]->mType == MEDIA_TYPE_VIDEO)
				{
					VMetaInfo* pMediaInfo = static_cast<VMetaInfo*>(m_oMediaInfo[i]);
					data = MetaData(META_KEY_VIDEO_WIDTH, CUtil::convert<std::string, int>(pMediaInfo->mWidth), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_VIDEO_HEIGHT, CUtil::convert<std::string, int>(pMediaInfo->mHeight), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_PIX_FORMAT, CUtil::convert<std::string, int>(pMediaInfo->mPixfmt), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_VIDEO_FPS, CUtil::convert<std::string, int>(pMediaInfo->mfps), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);
				}
				else if (m_oMediaInfo[i]->mType == MEDIA_TYPE_AUDIO)
				{
					AMetaInfo* pMediaInfo = static_cast<AMetaInfo*>(m_oMediaInfo[i]);
					data = MetaData(META_KEY_SAMPLE_RATE, CUtil::convert<std::string, int>(pMediaInfo->mSamplerate), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_CHANNEL_NUM, CUtil::convert<std::string, int>(pMediaInfo->mChannel), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_SAMPLE_FORMAT, CUtil::convert<std::string, int>(pMediaInfo->mSamplefmt), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);

					data = MetaData(META_KEY_FRAME_SAMPLE_COUNT, CUtil::convert<std::string, int>(pMediaInfo->mSamplecnt), META_DATA_VAL_TYPE_INT);
					m_outPorts[i]->SetProperty(data);
				}
			}
		}

		m_curState = MEDIA_ELEMENT_STATE_OPENED;
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN || state == MEDIA_ELEMENT_STATE_RESUME)
	{
		CSThread::Start();
		av_read_play(m_fmtCtx);
		m_curState = MEDIA_ELEMENT_STATE_RUNNING;
	}
	else if (state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		av_read_pause(m_fmtCtx);
		m_curState = MEDIA_ELEMENT_STATE_PAUSED;
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_curState = MEDIA_ELEMENT_STATE_STOPPED;
		CSThread::Kill();
		Close();		
	}
}

int CFFStreamingSrc::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{

	return MEDIA_ERR_NONE;
}

void CFFStreamingSrc::Run()
{
	int ret = 0;
	AVPacket pkt;
	int64_t start_time = av_gettime();
	while (1)
	{
		//LOG_INFO("current state %d", m_curState);
		if (m_curState == MEDIA_ELEMENT_STATE_RUNNING)
		{
			//Get an AVPacket
			ret = av_read_frame(m_fmtCtx, &pkt);
			if (ret < 0)
			{
				//LOG_ERR("Read frame error(%d)", ret);
				if (ret == AVERROR_EOF)
				{
					LOG_WARN("Read end of file");
					m_curState = MEDIA_ELEMENT_STATE_PAUSED;
					MediaMessage msg;
					msg.m_type = MEDIA_MESSAGE_NOTIFY;
					msg.m_src = this;
					msg.m_subType = MESSAGE_SUB_TYPE_END_OF_STREAM;
					PostMessage(msg);
				}
				continue;
			}

			AVStream *in_stream = m_fmtCtx->streams[pkt.stream_index];

			AVRational time_base = in_stream->time_base;
			AVRational time_base_q = { 1, AV_TIME_BASE };

			MediaMetaInfo* pMediaInfo = NULL;
			int portIdx = GetMediaInfoByStreamID(pkt.stream_index, &pMediaInfo);
			if (pMediaInfo == NULL || portIdx < 0)
			{
				LOG_ERR("can`t find media info for packet stream(%d)!", pkt.stream_index);
				MediaMessage msg;
				msg.m_type = MEDIA_MESSAGE_NOTIFY;
				msg.m_src = this;
				msg.m_subType = MESSAGE_SUB_TYPE_ERROR;
				msg.m_content.msgFileds.push_back(MetaData("ErrCode",
					CUtil::convert<std::string, int>(MEDIA_ERR_INVALIDE_PARAME),
					META_DATA_VAL_TYPE_INT));
				msg.m_content.msgFileds.push_back(MetaData("ErrMsg", "can`t find media info for packet stream!",
					META_DATA_VAL_TYPE_STRING));
				PostMessage(msg);

				m_curState = MEDIA_ELEMENT_STATE_PAUSED;
				continue;
			}
			if (pMediaInfo->mType == MEDIA_TYPE_VIDEO)	//video
			{
				if (pkt.flags & AV_PKT_FLAG_KEY)
				{
					LOG_DEBUG("Key frame!");
					m_bGetKeyFrame = true;
				}

				if (!m_bGetKeyFrame) //从I帧开始发送数据
				{
					continue;
				}

			}
			else if (pMediaInfo->mType == MEDIA_TYPE_AUDIO)	//audio
			{
				if (!m_bGetKeyFrame)
				{
					continue;
				}
			}


			int playTime = av_rescale_q(pkt.dts - in_stream->start_time, time_base, time_base_q) / AV_TIME_BASE;
			if (m_curPlayTime != playTime)
			{
				m_curPlayTime = playTime;
				LOG_DEBUG("Current Play time: %d s", m_curPlayTime);
			}

			unsigned long long ts = av_rescale_q(pkt.dts, time_base, time_base_q);
			CMediaBuffer *outBuf = new CMediaBuffer(pkt.data, pkt.size, ts, ts, 0);
			if (outBuf != NULL)
			{
				m_outPorts[portIdx]->PushBufferToDownStream(outBuf);
				delete outBuf;
			}

			av_free_packet(&pkt);

			CSThread::Sleep(1);

		}
		else if (m_curState == MEDIA_ELEMENT_STATE_PAUSED)
		{
		
		}
		else if (m_curState == MEDIA_ELEMENT_STATE_STOPPED)
		{
			break;
		}
	}
}

int CFFStreamingSrc::GetMediaInfoByStreamID(int id, MediaMetaInfo** pMediaInfo)
{
	for (int i = 0; i < m_oPortCnt; i++)
	{
		if (m_oMediaInfo[i]->mStreamID == id)
		{
			*pMediaInfo = m_oMediaInfo[i];
			return i;
		}
	}

	return -1;
}