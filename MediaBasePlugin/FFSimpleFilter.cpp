#include "FFSimpleFilter.h"
#include "common.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"
#include <assert.h>

extern "C"
{
#include "libavcodec/avcodec.h"
};

#define LOG_FILTER	"FFSimpleFilter"

CFFSimpleFilter::CFFSimpleFilter() 
	: m_pFilterGraph(NULL), m_pBufferSrcCtx(NULL), m_pBufferSinkCtx(NULL), m_srcFrame(NULL), m_dstFrame(NULL)
{
	Init("FFSimpleFilter");
}

CFFSimpleFilter::CFFSimpleFilter(const std::string &name) 
	: m_pFilterGraph(NULL), m_pBufferSrcCtx(NULL), m_pBufferSinkCtx(NULL), m_srcFrame(NULL), m_dstFrame(NULL)
{
	Init(name);
}

CFFSimpleFilter::~CFFSimpleFilter()
{
}

void CFFSimpleFilter::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_desc.SetElementPortCount(1, 1);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

void CFFSimpleFilter::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		if(m_desc.GetMetaData(META_KEY_FILTE_DESC, data) == MEDIA_ERR_NONE)
		{
			m_filterDesc = data.mValue;
		}

		if(m_desc.GetMetaData(META_KEY_MEDIA, data) == MEDIA_ERR_NONE)
		{
			m_type = MediaStrToType(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
		{
			m_dstWidth = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstWidth = m_srcWidth;
			m_inPorts[0]->GetProperty(META_KEY_VIDEO_WIDTH, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
		{
			m_dstHeight = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstHeight = m_srcHeight;
			m_inPorts[0]->GetProperty(META_KEY_VIDEO_HEIGHT, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_dstPixfmt = CUtil::convert<int, std::string>(data.mValue);
			m_dstPixfmt = FFPixFormat((VideoPixformat)m_dstPixfmt);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstPixfmt = m_srcPixfmt;
			m_inPorts[0]->GetProperty(META_KEY_PIX_FORMAT, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_VIDEO_FPS, data) == MEDIA_ERR_NONE)
		{
			m_dstfps = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstfps = m_srcfps;
			m_inPorts[0]->GetProperty(META_KEY_VIDEO_FPS, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
		{
			m_dstSampleRate = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstSampleRate = m_srcSampleRate;
			m_inPorts[0]->GetProperty(META_KEY_SAMPLE_RATE, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_dstSamplefmt = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstSamplefmt = m_srcSamplefmt;
			m_inPorts[0]->GetProperty(META_KEY_SAMPLE_FORMAT, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
		{
			m_dstChannels = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstChannels = m_srcChannels;
			m_inPorts[0]->GetProperty(META_KEY_CHANNEL_NUM, data);
			m_outPorts[0]->SetProperty(data);
		}

		if(m_desc.GetMetaData(META_KEY_FRAME_SAMPLE_COUNT, data) == MEDIA_ERR_NONE)
		{
			m_samplecount = CUtil::convert<int, std::string>(data.mValue);
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(InitFilterGraph() == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->StartTask();
		}
		m_curState = MEDIA_ELEMENT_STATE_RUNNING;
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_outPorts[0]->PauseTask();
		m_curState = MEDIA_ELEMENT_STATE_PAUSED;
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();
		m_curState = MEDIA_ELEMENT_STATE_STOPPED;
		UnInitFilterGraph();
	}
}

void CFFSimpleFilter::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer == NULL)
		return;

	if(m_pBufferSrcCtx == NULL || m_pBufferSinkCtx == NULL ||
		m_srcFrame == NULL)
		return;

	if(m_type == MEDIA_TYPE_VIDEO)
	{
		av_image_fill_arrays(m_srcFrame->data, m_srcFrame->linesize, 
			(const uint8_t*)buffer->GetData(), (AVPixelFormat)m_srcPixfmt, m_srcWidth, m_srcHeight, 1);
		m_srcFrame->width = m_srcWidth;
		m_srcFrame->height = m_srcHeight;
		m_srcFrame->format = m_srcPixfmt;
		m_srcFrame->pts = buffer->GetPts();
	}
	else if(m_type == MEDIA_TYPE_AUDIO)
	{
		assert(buffer->GetDataSize() == m_samplecount * av_get_bytes_per_sample((AVSampleFormat)m_srcSamplefmt) * m_srcChannels);	
		m_srcFrame->pts = buffer->GetPts();
		m_srcFrame->nb_samples= m_samplecount;  
		m_srcFrame->format= (AVSampleFormat)m_srcSamplefmt;  
		m_srcFrame->channel_layout = av_get_default_channel_layout(m_srcChannels);
		m_srcFrame->channels = m_srcChannels;
		m_srcFrame->sample_rate = m_srcSampleRate;
		avcodec_fill_audio_frame(m_srcFrame, m_srcChannels, (AVSampleFormat)m_srcSamplefmt, (const uint8_t*)buffer->GetData(), buffer->GetDataSize(), 1);
	}

	if(av_buffersrc_add_frame_flags(m_pBufferSrcCtx, m_srcFrame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
	{
		LOG_ERR("Error while feeding the filtergraph");
	}

	av_frame_unref(m_srcFrame);
}

int CFFSimpleFilter::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int ret = 0;
	ret = av_buffersink_get_frame(m_pBufferSinkCtx, m_dstFrame);  
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)  
	{

	} 
	else if(ret >= 0)
	{
		if(m_type == MEDIA_TYPE_VIDEO)
		{
			int size = av_image_get_buffer_size((AVPixelFormat)m_dstFrame->format, m_dstFrame->width, m_dstFrame->height,1);
			unsigned char *data = new unsigned char[size];
			av_image_copy_to_buffer(data, size, m_dstFrame->data, m_dstFrame->linesize, 
				(AVPixelFormat)m_dstFrame->format,m_dstFrame->width, m_dstFrame->height,1);
			*buffer = new CMediaBuffer(data, size, m_dstFrame->pkt_pts, m_dstFrame->pkt_dts, m_dstFrame->pkt_duration);
			delete data;
		}
		else if(m_type == MEDIA_TYPE_AUDIO)
		{
			*buffer = new CMediaBuffer(m_dstFrame->data[0], m_dstFrame->linesize[0], m_dstFrame->pkt_pts, m_dstFrame->pkt_dts, m_dstFrame->pkt_duration);
		}
	}
	av_frame_unref(m_dstFrame);

	return ret;
}

void CFFSimpleFilter::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[0]->GetProperty(key, data);

	if(key == META_KEY_VIDEO_WIDTH)
	{
		m_srcWidth = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_VIDEO_HEIGHT)
	{
		m_srcHeight = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_PIX_FORMAT)
	{
		m_srcPixfmt = CUtil::convert<int, std::string>(data.mValue);
		m_srcPixfmt = FFPixFormat((VideoPixformat)m_srcPixfmt);
	}
	else if(key == META_KEY_VIDEO_FPS)
	{
		m_srcfps = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_SAMPLE_RATE)
	{
		m_srcSampleRate = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_SAMPLE_FORMAT)
	{
		m_srcSamplefmt = CUtil::convert<int, std::string>(data.mValue);
	}
	else if(key == META_KEY_CHANNEL_NUM)
	{
		m_srcChannels = CUtil::convert<int, std::string>(data.mValue);
	}
	else if (key == META_KEY_FRAME_SAMPLE_COUNT)
	{
		m_samplecount = CUtil::convert<int, std::string>(data.mValue);
	}
	else
	{
		if (key == META_KEY_MEDIA)
		{
			m_type = MediaStrToType(data.mValue);
		}
		m_outPorts[0]->SetProperty(data);
	}
}

int CFFSimpleFilter::InitFilterGraph()
{
	int res = MEDIA_ERR_NONE;
	const AVFilter *buffersrc = NULL;
	const AVFilter *buffersink = NULL;
	AVFilterInOut*  inputs = NULL;
	AVFilterInOut*  outputs = NULL;

	m_srcFrame = av_frame_alloc();
	m_dstFrame = av_frame_alloc();

	if(m_type == MEDIA_TYPE_VIDEO)
	{
		buffersrc  = avfilter_get_by_name("buffer");
	}
	else if(m_type == MEDIA_TYPE_AUDIO)
	{
		buffersrc  = avfilter_get_by_name("abuffer");
	}
	if(!buffersrc)
	{
		LOG_ERR("buffersrc not found");
		res = MEDIA_ERR_NOT_FOUND;
		goto label_error;
	}

	if(m_type == MEDIA_TYPE_VIDEO)
	{
		buffersink = avfilter_get_by_name("buffersink");
	}
	else if(m_type == MEDIA_TYPE_AUDIO)
	{
		buffersink = avfilter_get_by_name("abuffersink");
	}
	if(!buffersink)
	{
		LOG_ERR("buffersink not found");
		res = MEDIA_ERR_NOT_FOUND;
		goto label_error;
	}

	m_pFilterGraph = avfilter_graph_alloc();
	if(!m_pFilterGraph)
	{
		LOG_ERR("No enough memory to alloc avfiltergraph!");
		res = MEDIA_ERR_MEMALLOC;
		goto label_error;
	}

	char args[512] = {0};
	if(m_type == MEDIA_TYPE_VIDEO)
	{
		snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		m_srcWidth, m_srcHeight, m_srcPixfmt,1, m_srcfps,1, 1);
	}
	else if(m_type == MEDIA_TYPE_AUDIO)
	{
		snprintf(args, sizeof(args),
			"sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
			m_srcSampleRate,
			av_get_sample_fmt_name(AVSampleFormat(m_srcSamplefmt)),
			av_get_default_channel_layout(m_srcChannels));
		/*
		snprintf(args, sizeof(args),
			"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
			1, m_srcSampleRate, m_srcSampleRate,
			av_get_sample_fmt_name(AVSampleFormat(m_srcSamplefmt)), 
			av_get_default_channel_layout(m_srcChannels));*/
	}

	res = avfilter_graph_create_filter(&m_pBufferSrcCtx, buffersrc, "in", args, NULL, m_pFilterGraph);
	if (res < 0)
	{
		LOG_ERR("Create buffer source error(%d)", res);
		res = MEDIA_ERR_INVALIDE_PARAME;
		goto label_error;
	}

	/* buffer video sink: to terminate the filter chain. */
	res = avfilter_graph_create_filter(&m_pBufferSinkCtx, buffersink, "out", NULL, NULL, m_pFilterGraph);
	if (res < 0) 
	{
		LOG_ERR("Create buffer sink error(%d)", res);
		res = MEDIA_ERR_INVALIDE_PARAME;
		goto label_error;
	}

	inputs = avfilter_inout_alloc();
	if(!inputs)
	{
		LOG_ERR("No enough memory to alloc avfilter inout!");
		res = MEDIA_ERR_MEMALLOC;
		goto label_error;
	}
	outputs = avfilter_inout_alloc();
	if(!outputs)
	{
		LOG_ERR("No enough memory to alloc avfilter inout!");
		res = MEDIA_ERR_MEMALLOC;
		goto label_error;
	}

	outputs->name       = av_strdup("in");
	outputs->filter_ctx = m_pBufferSrcCtx;
	outputs->pad_idx    = 0;
	outputs->next       = NULL;

	inputs->name       = av_strdup("out");
	inputs->filter_ctx = m_pBufferSinkCtx;
	inputs->pad_idx    = 0;
	inputs->next       = NULL;

	if((avfilter_graph_parse_ptr(m_pFilterGraph ,m_filterDesc.c_str(), &inputs, &outputs, NULL)) < 0)
	{
		LOG_ERR("avfilter_graph_parse_ptr error");
		res = MEDIA_ERR_INVALIDE_PARAME;
		goto label_error;
	}

	if ((avfilter_graph_config(m_pFilterGraph ,NULL)) < 0)
	{
		LOG_ERR("avfilter_graph_config error");
		res = MEDIA_ERR_INVALIDE_PARAME;
		goto label_error;
	}

	res = MEDIA_ERR_NONE;

label_error:
	if(inputs != NULL)
	{
		avfilter_inout_free(&inputs);
	}
	if(outputs != NULL)
	{
		avfilter_inout_free(&outputs); 
	}

	return res;
}

void CFFSimpleFilter::UnInitFilterGraph()
{
	if(m_pFilterGraph != NULL)
	{
		avfilter_graph_free(&m_pFilterGraph);
		m_pFilterGraph = NULL;
		m_pBufferSrcCtx = NULL;
		m_pBufferSinkCtx = NULL;
	}

	if(m_srcFrame != NULL)
	{
		av_frame_free(&m_srcFrame);
		m_srcFrame = NULL;
	}

	if(m_dstFrame != NULL)
	{
		av_frame_free(&m_dstFrame);
		m_dstFrame = NULL;
	}
}