#include "FFCameraCapture.h"

#include "common.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"


#define LOG_FILTER	"FFCameraCapture"

CFFCameraCapture::CFFCameraCapture() : m_fmtCtx(NULL), m_width(0), m_height(0), m_pixFmt(0), m_fps(30)
{
	Init("FFCameraCapture");
}

CFFCameraCapture::CFFCameraCapture(const std::string &name) : m_fmtCtx(NULL), m_width(0), m_height(0), m_pixFmt(0), m_fps(30)
{
	Init(name);
}

CFFCameraCapture::~CFFCameraCapture()
{
}

void CFFCameraCapture::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	MetaData media(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(media);
	m_outPorts[0]->SetProperty(media);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);

}

int CFFCameraCapture::Open()
{
	LOG_INFO("Start CameraCapture");
	int res = MEDIA_ERR_NONE;
	AVInputFormat *ifmt = av_find_input_format("dshow");
	if(ifmt == NULL)
	{
		LOG_ERR("Can not find show!");

		return MEDIA_ERR_NOT_FOUND;
	}

	m_fmtCtx = avformat_alloc_context();
	if(m_fmtCtx == NULL)
	{
		LOG_ERR("Alloc format context error");
		return MEDIA_ERR_MEMALLOC;
	}

	AVDictionary* options = NULL;

	char resolution[64];
	char framerate[64];
	char pixformat[8];
	snprintf(resolution , sizeof(resolution), "%dx%d", m_width, m_height);
	snprintf(framerate,sizeof(framerate),"%d", m_fps);
	snprintf(pixformat,sizeof(pixformat),"%d", m_pixFmt);
	av_dict_set(&options, "framerate", framerate, 0);
	av_dict_set(&options, "video_size", resolution, 0);
	av_dict_set(&options, "pixel_format", pixformat, 0);

	std::string devName = "video=" + m_devName;

	res = avformat_open_input(&m_fmtCtx, devName.c_str(), ifmt, &options);
	if(res < 0)
	{
		LOG_ERR("Open error, %d!", res);
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	//avformat_find_stream_info(m_fmtCtx, NULL);

	//int idx = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_VIDEO, 0, -1, NULL, 0);

	//AVCodecContext *codecCtx = NULL;
	//codecCtx = m_fmtCtx->streams[idx]->codec;

	return MEDIA_ERR_NONE;
}

void CFFCameraCapture::Close()
{
	LOG_INFO("Stop CameraCapture");
	if(m_fmtCtx != NULL)
	{
		avformat_close_input(&m_fmtCtx);

		avformat_free_context(m_fmtCtx);
		m_fmtCtx = NULL;
	}
}

void CFFCameraCapture::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;

		if(m_desc.GetMetaData(META_KEY_MEDIA, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_FILE_PATH, data) == MEDIA_ERR_NONE)
		{
			m_devName = data.mValue;
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_width = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_height = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_pixFmt = CUtil::convert<int, std::string>(data.mValue);
			m_pixFmt = FFPixFormat((VideoPixformat)m_pixFmt);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_FPS, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_fps = CUtil::convert<int, std::string>(data.mValue);
		}
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

int CFFCameraCapture::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int res = MEDIA_ERR_NONE;
	AVPacket pkt;

	if(m_fmtCtx == NULL)
	{
		return MEDIA_ERR_READ_FAILED;
	}

	av_init_packet(&pkt);

	res = av_read_frame(m_fmtCtx, &pkt);
	if(res < 0)
	{
		return MEDIA_ERR_READ_FAILED;
	}
	else
	{
		//long long ts = CUtil::GetTimeStamp();
		*buffer = new CMediaBuffer(pkt.data, pkt.size, pkt.pts, pkt.dts, pkt.duration);
	}
	av_packet_unref(&pkt);


	return MEDIA_ERR_NONE;
}