#include "VideoOverlayFilter.h"
#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"

#define LOG_FILTER	"VideoOverlayFilter"

CVideoOverlayFilter::CVideoOverlayFilter()
{
	Init("VideoOverlayFilter");
}

CVideoOverlayFilter::CVideoOverlayFilter(const std::string &name)
{
	Init(name);
}

CVideoOverlayFilter::~CVideoOverlayFilter()
{
}

void CVideoOverlayFilter::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_desc.SetElementPortCount(1, 1);
	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);
	m_outPorts[0]->SetProperty(data);
	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

void CVideoOverlayFilter::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	unsigned int inPortCnt = 1;
	unsigned int outPortCnt = 1;

	m_desc.GetElementPortCount(inPortCnt, outPortCnt);
	if(data.mKey == META_KEY_INPORT_COUNT)
	{
		inPortCnt = CUtil::convert<unsigned, std::string>(data.mValue);

		for(int i = 0; i < inPortCnt; i++)
		{
			m_inPorts[i] = new CMediaPort(this, PORT_DIR_IN, i);
		}
	}
	m_desc.SetElementPortCount(inPortCnt, outPortCnt);
}

void CVideoOverlayFilter::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_outFormat = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(InitFilterGraph() == 0)
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
	}
}

void CVideoOverlayFilter::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer == NULL)
	{
		return;
	}
	int ret = 0;
	AVFrame *srcFrame = av_frame_alloc();
	srcFrame->pkt_pts = buffer->GetPts();
	srcFrame->pkt_dts = buffer->GetDts();
	srcFrame->pkt_duration = buffer->GetDuration();
	av_image_fill_arrays(srcFrame->data, srcFrame->linesize, buffer->GetData(),
		(AVPixelFormat)(m_overInfos[id].mImgFormat), m_overInfos[id].mImgWidth, m_overInfos[id].mImgHeight,1);

	if ((ret = av_buffersrc_add_frame_flags(m_pBufferSrcCtx[id], srcFrame, AV_BUFFERSRC_FLAG_KEEP_REF)) < 0)
	{  
		LOG_ERR("Error while feeding the filtergraph(%d)", ret);    
	} 

	av_frame_unref(srcFrame);
}

int CVideoOverlayFilter::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	AVFrame *outFrame = av_frame_alloc();
	int ret = 0;
	ret = av_buffersink_get_frame(m_pBufferSinkCtx, outFrame);  
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)  
	{

	} 
	else if(ret >= 0)
	{
		int size = av_image_get_buffer_size((AVPixelFormat)outFrame->format, outFrame->width, outFrame->height,1);
		unsigned char *data = new unsigned char[size];
		av_image_copy_to_buffer(data, size, outFrame->data, outFrame->linesize, 
			(AVPixelFormat)outFrame->format,outFrame->width, outFrame->height,1);
		*buffer = new CMediaBuffer(data, size, outFrame->pkt_pts, outFrame->pkt_dts, outFrame->pkt_duration);
		delete data;
	}
	av_frame_unref(outFrame);

	return ret;
}

void CVideoOverlayFilter::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	if(m_inPorts[index]->GetProperty(key, data) == MEDIA_ERR_NONE)
	{
		if(key == META_KEY_VIDEO_X_POS)
		{
			m_overInfos[index].mStartX = CUtil::convert<int, std::string>(data.mValue);
		}
		else if(key == META_KEY_VIDEO_Y_POS)
		{
			m_overInfos[index].mStartY = CUtil::convert<int, std::string>(data.mValue);
		}
		else if(key == META_KEY_VIDEO_WIDTH)
		{
			m_overInfos[index].mImgWidth = CUtil::convert<int, std::string>(data.mValue);
			if(index == 0)
			{
				m_outPorts[0]->SetProperty(data);
			}
		}
		else if(key == META_KEY_VIDEO_HEIGHT)
		{
			m_overInfos[index].mImgHeight = CUtil::convert<int, std::string>(data.mValue);
			if(index == 0)
			{
				m_outPorts[0]->SetProperty(data);
			}
		}
		else if(key == META_KEY_PIX_FORMAT)
		{
			m_overInfos[index].mImgFormat = CUtil::convert<int, std::string>(data.mValue);
		}

	}
}

int CVideoOverlayFilter::InitFilterGraph()
{
	int ret = 0;
	std::string filterDesc = "[in0]";

	m_pFilterGraph = avfilter_graph_alloc(); 
	if(m_pFilterGraph == NULL)
	{
		LOG_ERR_S("Create Filter Graph error");
		return MEDIA_ERR_MEMALLOC;
	}

	unsigned int inPortCnt = 1;
	unsigned int outPortCnt = 1;
	char args[512] = {0};
	char buffername[16] = {0};

	m_desc.GetElementPortCount(inPortCnt, outPortCnt);
	for(int i = 0; i < inPortCnt; i++)
	{
		AVFilter *bufferSrc = avfilter_get_by_name("buffer");	
		snprintf(buffername, sizeof(buffername), "bufferIn%d", i);
		snprintf(args, sizeof(args),  
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",  
			m_overInfos[i].mImgWidth, m_overInfos[i].mImgHeight, m_overInfos[i].mImgFormat,  
			1, 30,  
			1, 0);

		/* buffer video sink: to terminate the filter chain. */  
		ret = avfilter_graph_create_filter(&m_pBufferSrcCtx[i], bufferSrc, buffername,
			args, NULL, m_pFilterGraph);
		if (ret < 0) 
		{  
			LOG_ERR("Create buffer sink error(%d)", ret);  
			goto end;
		}  
	}


	AVFilter *bufferSink = avfilter_get_by_name("buffersink");
	/* buffer video sink: to terminate the filter chain. */  
	ret = avfilter_graph_create_filter(&m_pBufferSinkCtx, bufferSink, "bufferOut",  
		NULL, NULL, m_pFilterGraph);            
	if (ret < 0) 
	{  
		LOG_ERR("Create buffer sink error(%d)", ret);  
		goto end;
	}  

	enum AVPixelFormat pix_fmts[] = { (AVPixelFormat)m_outFormat, AV_PIX_FMT_NONE };
	/* Set a binary option to an integer list. */  
	ret = av_opt_set_int_list(m_pBufferSinkCtx, "pix_fmts", pix_fmts,  
		AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);     
	if (ret < 0)
	{  
		LOG_ERR("Cannot set output pixel format,%d", ret);  
		goto end;
	}

	for(int i = 1; i < inPortCnt; i++)
	{
		char subDesc[32] = {0};
		snprintf(subDesc, sizeof(subDesc), "[in%d]overlay=%d:%d", i,m_overInfos[i].mStartX,m_overInfos[i].mStartY);
		filterDesc += subDesc;
		if(i == inPortCnt - 1)
		{
			filterDesc += "[out]";
		}
		else
		{
			char ddesc[16] = {0};
			snprintf(ddesc, sizeof(ddesc), "[tmpOut%d];[tmpOut%d]", i-1, i-1);
			filterDesc += ddesc;
		}
	}

	AVFilterInOut *inputs = NULL;
	AVFilterInOut *outputs = NULL;
	ret = avfilter_graph_parse2(m_pFilterGraph, filterDesc.c_str(), &inputs, &outputs);
	if(ret < 0)
	{
		LOG_ERR("Parser Filter description error(%d)", ret);
		goto end;
	}

	AVFilterInOut *curInout = inputs;
	int idx = 0;
	do 
	{
		ret = avfilter_link(m_pBufferSrcCtx[idx++], 0, curInout->filter_ctx, curInout->pad_idx);
		if(ret != 0)
		{
			LOG_ERR("Link bufferSrc error(%d)", ret);
			goto end;
		}
		curInout = curInout->next;
	} 
	while(curInout != NULL);

	ret = avfilter_link(outputs->filter_ctx, 0, m_pBufferSinkCtx, 0);
	if(ret != 0)
	{
		LOG_ERR("Link bufferSink error(%d)", ret);
		goto end;
	}

	/* Check validity and configure all the links and formats in the graph */  
	if ((ret = avfilter_graph_config(m_pFilterGraph, NULL)) < 0)     
		goto end;

end:
	if(inputs != NULL)
	{
		avfilter_inout_free(&inputs);
	}
	if(outputs != NULL)
	{
		avfilter_inout_free(&outputs); 
	}

	if(m_pFilterGraph != NULL)
	{
		avfilter_graph_free(&m_pFilterGraph);
	}
	return ret;
}

void CVideoOverlayFilter::UnInitFilterGraph()
{
	if(m_pFilterGraph != NULL)
	{
		avfilter_graph_free(&m_pFilterGraph);
		m_pFilterGraph = NULL;
	}
}