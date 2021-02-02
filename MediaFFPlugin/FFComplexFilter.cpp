#include "FFComplexFilter.h"
#include "common.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"
#include <assert.h>

#define LOG_FILTER	"FFComplexFilter"

CFFComplexFilter::CFFComplexFilter() : m_iPortCnt(1), m_oPortCnt(1), m_pFilterGraph(NULL)
{
	Init("FFComplexFilter");
}

CFFComplexFilter::CFFComplexFilter(const std::string &name)
	 : m_iPortCnt(1), m_oPortCnt(1), m_pFilterGraph(NULL)
{
	Init(name);
}

CFFComplexFilter::~CFFComplexFilter()
{
}

void CFFComplexFilter::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	for(int i = 0; i < MAX_SIDE_PORT; i++)
	{
		m_iMediaInfo[i] = NULL;
		m_oMediaInfo[i] = NULL;
		m_pBufferSrcCtxs[i] = NULL;
		m_pBufferSinkCtxs[i] = NULL;
		m_srcFrame[i] = NULL;
		m_dstFrame[i] = NULL;
	}
}

void CFFComplexFilter::UseParame(const MetaData &data)
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

	if(data.mKey == META_KEY_OUTPORT_COUNT)
	{
		m_oPortCnt = CUtil::convert<int, std::string>(data.mValue);	
	}
	for(int i = 0; i < m_oPortCnt; i++)
	{
		m_outPorts[i] = new CMediaPort(this, PORT_DIR_OUT, i);
		m_outPorts[i]->SetActiveMode(true, STREAMING_PUSH);
	}

	m_desc.SetElementPortCount(m_iPortCnt, m_oPortCnt);
}


void CFFComplexFilter::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		if(m_desc.GetMetaData(META_KEY_FILTE_DESC, data) == MEDIA_ERR_NONE)
		{
			m_filterDesc = data.mValue;
		}

		for(int i = 0; i < m_oPortCnt; i++)
		{
			MediaType type = MEDIA_TYPE_VIDEO;
			if(m_outPorts[i]->GetProperty(META_KEY_MEDIA, data) == MEDIA_ERR_NONE)
			{
				type = MediaStrToType(data.mValue);
			}

			if(type == MEDIA_TYPE_VIDEO)
			{
				int width = 0;
				int height = 0;
				int fmt = 0;
				int fps = 0;
				if(m_outPorts[i]->GetProperty(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
				{
					width = CUtil::convert<int, std::string>(data.mValue);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
				{
					height = CUtil::convert<int, std::string>(data.mValue);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
				{
					fmt = CUtil::convert<int, std::string>(data.mValue);
					fmt = FFPixFormat((VideoPixformat)fmt);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_VIDEO_FPS, data) == MEDIA_ERR_NONE)
				{
					fps = CUtil::convert<int, std::string>(data.mValue);
				}

				m_oMediaInfo[i] = new VMetaInfo(width, height, fmt, fps);
			}
			else if(type == MEDIA_TYPE_AUDIO)
			{
				int samplerate = 0;
				int ch = 0;
				int samplefmt = 0;
				int samplecount = 0;

				if(m_outPorts[i]->GetProperty(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
				{
					samplerate = CUtil::convert<int, std::string>(data.mValue);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
				{
					samplefmt = CUtil::convert<int, std::string>(data.mValue);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
				{
					ch = CUtil::convert<int, std::string>(data.mValue);
				}
				if(m_outPorts[i]->GetProperty(META_KEY_FRAME_SAMPLE_COUNT, data) == MEDIA_ERR_NONE)
				{
					samplecount = CUtil::convert<int, std::string>(data.mValue);
				}

				m_oMediaInfo[i] = new AMetaInfo(samplerate, ch, samplefmt,samplecount);
			}
			
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(InitFilterGraph() == MEDIA_ERR_NONE)
		{
			for(int i = 0; i < m_oPortCnt; i++)
			{
				m_outPorts[i]->StartTask();
			}
		}
		m_curState = MEDIA_ELEMENT_STATE_RUNNING;
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		for(int i = 0; i < m_oPortCnt; i++)
		{
			m_outPorts[i]->PauseTask();
		}
		m_curState = MEDIA_ELEMENT_STATE_PAUSED;
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		for(int i = 0; i < m_oPortCnt; i++)
		{
			m_outPorts[i]->StopTask();
		}
		m_curState = MEDIA_ELEMENT_STATE_STOPPED;
		UnInitFilterGraph();
	}
}

void CFFComplexFilter::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer == NULL)
		return;

	if(m_pBufferSrcCtxs[id] == NULL ||
		m_srcFrame[id] == NULL)
		return;

	if(m_iMediaInfo[id]->mType == MEDIA_TYPE_VIDEO)
	{
		VMetaInfo *info = static_cast<VMetaInfo*>(m_iMediaInfo[id]);
		av_image_fill_arrays(m_srcFrame[id]->data, m_srcFrame[id]->linesize, 
			(const uint8_t*)buffer->GetData(), (AVPixelFormat)info->mPixfmt, info->mWidth, info->mHeight, 1);
		m_srcFrame[id]->width = info->mWidth;
		m_srcFrame[id]->height = info->mHeight;
		m_srcFrame[id]->format = info->mPixfmt;
		//m_srcFrame[id]->pts = buffer->GetPts();
	}
	else if(m_iMediaInfo[id]->mType == MEDIA_TYPE_AUDIO)
	{
		AMetaInfo *info = static_cast<AMetaInfo*>(m_iMediaInfo[id]);
		assert(buffer->GetDataSize() == 
			info->mSamplecnt * av_get_bytes_per_sample((AVSampleFormat)info->mSamplefmt) * info->mChannel);
		avcodec_fill_audio_frame(m_srcFrame[id], info->mChannel,
			(AVSampleFormat)info->mSamplefmt,(const uint8_t*)buffer->GetData(), buffer->GetDataSize(), 1);
		m_srcFrame[id]->pts = buffer->GetPts();
		m_srcFrame[id]->nb_samples= info->mSamplecnt;  
		m_srcFrame[id]->format= (AVSampleFormat)info->mSamplefmt;  
		m_srcFrame[id]->channel_layout = av_get_default_channel_layout(info->mChannel);
		m_srcFrame[id]->channels = info->mChannel;
		m_srcFrame[id]->sample_rate = info->mSamplerate;		
	}

	if(av_buffersrc_add_frame_flags(m_pBufferSrcCtxs[id], m_srcFrame[id], AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
	{
		LOG_ERR("Error while feeding the filtergraph");
	}

	av_frame_unref(m_srcFrame[id]);
}

int CFFComplexFilter::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int ret = 0;
	ret = av_buffersink_get_frame(m_pBufferSinkCtxs[id], m_dstFrame[id]);  
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)  
	{

	} 
	else if(ret >= 0)
	{
		if(m_oMediaInfo[id]->mType == MEDIA_TYPE_VIDEO)
		{
			int size = av_image_get_buffer_size((AVPixelFormat)m_dstFrame[id]->format,
				m_dstFrame[id]->width, m_dstFrame[id]->height,1);
			unsigned char *data = new unsigned char[size];
			av_image_copy_to_buffer(data, size, m_dstFrame[id]->data, m_dstFrame[id]->linesize, 
				(AVPixelFormat)m_dstFrame[id]->format,m_dstFrame[id]->width, m_dstFrame[id]->height,1);
			*buffer = new CMediaBuffer(data, size, m_dstFrame[id]->pts, m_dstFrame[id]->pkt_dts, m_dstFrame[id]->pkt_duration);
			delete data;

		}
		else if(m_oMediaInfo[id]->mType == MEDIA_TYPE_AUDIO)
		{
			*buffer = new CMediaBuffer(m_dstFrame[id]->data[0], m_dstFrame[id]->linesize[0], 
				m_dstFrame[id]->pts, m_dstFrame[id]->pkt_dts, m_dstFrame[id]->pkt_duration);
		}
	}
	av_frame_unref(m_dstFrame[id]);

	return ret;
}

void CFFComplexFilter::PortSettingChanged(TRACKID index, const std::string &key)
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
			((VMetaInfo*)m_iMediaInfo[index])->mPixfmt = CUtil::convert<int, std::string>(data.mValue);
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

int CFFComplexFilter::InitFilterGraph()
{
	int res = MEDIA_ERR_NONE;
	const AVFilter *buffersrc = NULL;
	const AVFilter *buffersink = NULL;
	AVFilterInOut*  inputs = NULL;
	AVFilterInOut*  outputs = NULL;
	int i;
	

	m_pFilterGraph = avfilter_graph_alloc();
	if(!m_pFilterGraph)
	{
		LOG_ERR("No enough memory to alloc avfiltergraph!");
		res = MEDIA_ERR_MEMALLOC;
		goto label_error;
	}

	char tag[32] = {0};
	for(i = 0; i < m_iPortCnt; i++)
	{
		char args[512] = {0};
		m_srcFrame[i] = av_frame_alloc();

		if(m_iMediaInfo[i]->mType == MEDIA_TYPE_VIDEO)
		{
			VMetaInfo *info = static_cast<VMetaInfo*>(m_iMediaInfo[i]);
			buffersrc  = avfilter_get_by_name("buffer");

			snprintf(args, sizeof(args),
				"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
				info->mWidth, info->mHeight, info->mPixfmt,1,info->mfps,1, 1);
		}
		else if(m_iMediaInfo[i]->mType == MEDIA_TYPE_AUDIO)
		{
			AMetaInfo *info = static_cast<AMetaInfo*>(m_iMediaInfo[i]);
			buffersrc  = avfilter_get_by_name("abuffer");

			snprintf(args, sizeof(args),
				"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%x",
				1, info->mSamplerate, info->mSamplerate,
				av_get_sample_fmt_name(AVSampleFormat(info->mSamplefmt)), 
				av_get_default_channel_layout(info->mChannel));
		}
		if(!buffersrc)
		{
			LOG_ERR("buffersrc not found");
			res = MEDIA_ERR_NOT_FOUND;
			goto label_error;
		}

		snprintf(tag, 32, "buffersrc%d", i);
		res = avfilter_graph_create_filter(&m_pBufferSrcCtxs[i], buffersrc, tag, args, NULL, m_pFilterGraph);
		if (res < 0)
		{
			LOG_ERR("Create buffer source error(%d)", res);
			res = MEDIA_ERR_INVALIDE_PARAME;
			goto label_error;
		}
	}

	for(i = 0; i < m_oPortCnt; i++)
	{
		m_dstFrame[i] = av_frame_alloc();

		if(m_oMediaInfo[i]->mType == MEDIA_TYPE_VIDEO)
		{
			buffersink = avfilter_get_by_name("buffersink");
		}
		else if(m_oMediaInfo[i]->mType == MEDIA_TYPE_AUDIO)
		{
			buffersink = avfilter_get_by_name("abuffersink");
		}

		if(!buffersink)
		{
			LOG_ERR("buffersink not found");
			res = MEDIA_ERR_NOT_FOUND;
			goto label_error;
		}

		snprintf(tag, 32, "buffersink%d", i);
		res = avfilter_graph_create_filter(&m_pBufferSinkCtxs[i], buffersink, tag, NULL, NULL, m_pFilterGraph);
		if (res < 0) 
		{
			LOG_ERR("Create buffer sink error(%d)", res);
			res = MEDIA_ERR_INVALIDE_PARAME;
			goto label_error;
		}
	}

	if ((res = avfilter_graph_parse2(m_pFilterGraph, m_filterDesc.c_str(), &inputs, &outputs)) < 0)
	{
		LOG_ERR("Parser Graph description error(%d)", res);
		res = MEDIA_ERR_INVALIDE_PARAME;
		goto label_error;
	}

	AVFilterInOut *input = inputs;
	for(i = 0; i < m_iPortCnt && input != NULL; i++)
	{
		avfilter_link(m_pBufferSrcCtxs[i], 0, input->filter_ctx, input->pad_idx);
		input = input->next;
	}

	AVFilterInOut *output = outputs;
	for(i = 0; i < m_oPortCnt && output != NULL; i++)
	{
		avfilter_link(output->filter_ctx, output->pad_idx, m_pBufferSinkCtxs[i], 0);
		output = output->next;
	}

	if ((res = avfilter_graph_config(m_pFilterGraph, NULL)) < 0)
	{
		LOG_ERR("avfilter_graph_config error(%d)", res);
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

void CFFComplexFilter::UnInitFilterGraph()
{
	if(m_pFilterGraph != NULL)
	{
		avfilter_graph_free(&m_pFilterGraph);
		m_pFilterGraph = NULL;
	}

	int i;

	for(i = 0; i < m_iPortCnt; i++)
	{
		m_pBufferSrcCtxs[i] = NULL;
		if(m_srcFrame[i] != NULL)
		{
			av_frame_free(&m_srcFrame[i]);
			m_srcFrame[i] = NULL;
		}

		if(m_iMediaInfo[i] != NULL)
		{
			delete m_iMediaInfo[i];
			m_iMediaInfo[i] = NULL;
		}
	}
	for(i = 0; i < m_oPortCnt; i++)
	{
		m_pBufferSinkCtxs[i] = NULL;
		if(m_dstFrame[i] != NULL)
		{
			av_frame_free(&m_dstFrame[i]);
			m_dstFrame[i] = NULL;
		}

		if(m_oMediaInfo[i] != NULL)
		{
			delete m_oMediaInfo[i];
			m_oMediaInfo[i] = NULL;
		}
	}
}