#include "stdafx.h"
#include "MediaPlayer.h"

#include "Util.h"
#include "Media.h"
#include "LogManager.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define LOG_FILTER	"MediaPlayer"

namespace GMF {

MediaPlayer::MediaPlayer()
	: m_pPipeline(NULL), m_curPlayTime(0), m_hWnd(NULL)
{
	CreatePipeline();
}


MediaPlayer::~MediaPlayer()
{
	DestroyPipeline();
}

void MediaPlayer::InitCore()
{
	media_core_init(0, 0, NULL);
}

void MediaPlayer::UninitCore()
{
	media_core_uninit();
}


int MediaPlayer::Open(const std::string& strURL, void* hWnd)
{
	if (m_pPipeline == NULL)
	{
		LOG_ERR("no pipeline!");
		return -1;
	}

	if (strURL.empty() || hWnd == NULL)
	{
		return -1;
	}

	element_set_parame(m_pSrc, MetaData(META_KEY_URI, strURL, META_DATA_VAL_TYPE_STRING));
	m_hWnd = hWnd;
	//RECT rect = { 0 };
	//::GetClientRect((HWND)m_hWnd, &rect);
	//element_set_parame(m_pScaler, MetaData(META_KEY_VIDEO_WIDTH, CUtil::convert<std::string, int>(rect.right - rect.left), META_DATA_VAL_TYPE_INT));
	//element_set_parame(m_pScaler, MetaData(META_KEY_VIDEO_HEIGHT, CUtil::convert<std::string, int>(rect.bottom - rect.top), META_DATA_VAL_TYPE_INT));

	element_set_parame(m_pVideoRender, MetaData(META_KEY_VIDEO_WINDOW, CUtil::convert<std::string, long>((long)hWnd), META_DATA_VAL_TYPE_PTR));
	
	element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_READY);

	return 0;
}

/*
开始播放
seektime	开始播放的时间位置(单位：秒)，默认从文件开始
rate		播放速率，默认原始倍速
*/
int MediaPlayer::Play(unsigned long seektime, float rate)
{
	element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_OPEN);

	return 0;
}

/*
跳转到指定的时间
*/
int MediaPlayer::Seek(unsigned long seektime)
{
	return 0;
}

/*
倍速播放（2倍速、4倍速、8倍速、16倍速）
*/
int MediaPlayer::Rate(float rate)
{
	return 0;
}

/*
暂停播放
*/
int MediaPlayer::Pause()
{
	element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_PAUSE);
	return 0;
}

/*
恢复播放
*/
int MediaPlayer::Resume()
{
	element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_RESUME);
	return 0;
}

/*
停止播放
*/
int MediaPlayer::Stop()
{
	element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_STOP);
	return 0;
}

/*
关闭媒体源
*/
int MediaPlayer::Close()
{
	return 0;
}


int MediaPlayer::CreatePipeline()
{
	if (m_pPipeline != NULL)
	{
		LOG_WARN("pipeline(%s) has exist!", element_get_name(m_pPipeline).c_str());
		return 0;
	}

	m_pPipeline = element_create("Pipeline", "rtsppipeline");
	if (m_pPipeline == NULL)
	{
		LOG_ERR("can`t create pipeline!");
		return -1;
	}
	LOG_DEBUG("create pipeline(%p), name:%s", m_pPipeline, element_get_name(m_pPipeline).c_str());
	element_get_parame(m_pPipeline, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING));
	{
		m_pSrc = element_create("FFStreamingSrc", "");
		if (m_pSrc == NULL)
		{
			LOG_ERR("can`t create StreamingSrc!");
			return -1;
		}
		element_use_parame(m_pSrc, MetaData(META_KEY_OUTPORT_COUNT, "2", META_DATA_VAL_TYPE_INT));
		element_set_outport_parame(m_pSrc, 0, MetaData(META_KEY_MEDIA, "Video", META_DATA_VAL_TYPE_STRING));
		element_set_outport_parame(m_pSrc, 1, MetaData(META_KEY_MEDIA, "Audio", META_DATA_VAL_TYPE_STRING));

		pipeline_add_element(m_pPipeline, m_pSrc);
	}

	{
		CMediaElement* pDecBin = element_create("Bin", "decoderbin");
		if (pDecBin == NULL)
		{
			LOG_ERR("can`t create decoder bin!");
			return -1;
		}
		element_get_parame(pDecBin, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_VERTICAL, META_DATA_VAL_TYPE_STRING));
		pipeline_add_element(m_pPipeline, pDecBin);

		{
			CMediaElement* pVDecBin = element_create("Bin", "vdecoderbin");
			if (pVDecBin == NULL)
			{
				LOG_ERR("can`t create video decoder bin!");
				return -1;
			}
			element_get_parame(pVDecBin, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING));
			bin_add_element(pDecBin, pVDecBin);

			CMediaElement* pVDec = element_create("FFVideoDecoder", "");
			if (pVDec == NULL)
			{
				LOG_ERR("can`t create video decoder!");
				return -1;
			}
			bin_add_element(pVDecBin, pVDec);
			/*
			m_pScaler = element_create("FFVideoScaler", "");
			if (m_pScaler == NULL)
			{
				LOG_ERR("can`t create video scaler!");
				return -1;
			}
			bin_add_element(pVDecBin, m_pScaler);
			bin_connect_element(pVDecBin, pVDec, m_pScaler);
			*/
			m_pVideoRender = element_create("D3DVideoRender", "");
			if (m_pVideoRender == NULL)
			{
				LOG_ERR("can`t create video render!");
				return -1;
			}
			bin_add_element(pVDecBin, m_pVideoRender);
			bin_connect_element(pVDecBin, pVDec, m_pVideoRender);

			element_set_state(pVDecBin, MEDIA_ELEMENT_STATE_PREPARE);
		}

		{
			CMediaElement* pADecBin = element_create("Bin", "adecoderbin");
			if (pADecBin == NULL)
			{
				LOG_ERR("can`t create audio decoder bin!");
				return -1;
			}
			element_get_parame(pADecBin, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING));
			bin_add_element(pDecBin, pADecBin);

			CMediaElement* pADec = element_create("FFAudioDecoder", "");
			if (pADec == NULL)
			{
				LOG_ERR("can`t create audio decoder!");
				return -1;
			}
			bin_add_element(pADecBin, pADec);

			CMediaElement* pARender = element_create("DSoundRender", "");
			if (pARender == NULL)
			{
				LOG_ERR("can`t create audio render!");
				return -1;
			}
			bin_add_element(pADecBin, pARender);
			bin_connect_element(pADecBin, pADec, pARender);

			element_set_state(pADecBin, MEDIA_ELEMENT_STATE_PREPARE);
		}
		element_set_state(pDecBin, MEDIA_ELEMENT_STATE_PREPARE);
		pipeline_connect_element(m_pPipeline, m_pSrc, pDecBin);
	}

	return 0;
}

int MediaPlayer::DestroyPipeline()
{
	if (m_pPipeline)
	{
		if(element_get_state(m_pPipeline) > MEDIA_ELEMENT_STATE_READY)
			element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_STOP);
		LOG_INFO("Stop pipeline+++");
		element_set_state(m_pPipeline, MEDIA_ELEMENT_STATE_NONE);
		delete m_pPipeline;
		m_pPipeline = NULL;
	}

	return 0;
}

}