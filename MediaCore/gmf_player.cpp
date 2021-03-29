#include "gmf_player.h"
#include "Util.h"
#include "Media.h"
#include "LogManager.h"

void gmf_init(int logtype, int loglevel, const char *logpath)
{
	media_core_init(logtype, loglevel, logpath);
}

void gmf_uninit()
{
	media_core_uninit();
}

void gmf_set_log_func(log_func func)
{
	CLogManager::setLogCallback((log_output_cb)func);
}


struct _gmf_player
{
	CMediaElement* pPipeline;
	CMediaElement* pSrc;
	CMediaElement* pVideoRender;
	void* hWnd;
	char url[256];
};

static int create_pipeline(gmf_player_t* hPlayer, const char* name);
static int destroy_pipeline(gmf_player_t* hPlayer);

gmf_player_t* gmf_player_create(const char* name, const char* pipelineFile)
{
	gmf_player_t* player = NULL;

	player = (gmf_player_t*)malloc(sizeof(gmf_player_t));
	if (player)
	{
		memset(player, 0, sizeof(gmf_player_t));
		if (create_pipeline(player, name) != 0)
		{
			LOG_ERR("create pipeline error!");
			destroy_pipeline(player);
			free(player);
			player = NULL;
		}
	}

	return player;
}

void gmf_player_destory(gmf_player_t* hPlayer)
{
	if (hPlayer)
	{
		destroy_pipeline(hPlayer);
		free(hPlayer);
		hPlayer = NULL;
	}
}

int gmf_player_open(gmf_player_t* hPlayer, const char* url, void* hWnd)
{
	if (hPlayer->pPipeline == NULL)
	{
		LOG_ERR("no pipeline!");
		return -1;
	}

	if (url == NULL || hWnd == NULL)
	{
		return -1;
	}

	strcpy(hPlayer->url, url);
	hPlayer->hWnd = hWnd;
	element_set_parame(hPlayer->pSrc, MetaData(META_KEY_URI, url, META_DATA_VAL_TYPE_STRING));

	element_set_parame(hPlayer->pVideoRender, MetaData(META_KEY_VIDEO_WINDOW, CUtil::convert<std::string, long>((long)hWnd), META_DATA_VAL_TYPE_PTR));

	element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_READY);

	return 0;
}

int gmf_player_close(gmf_player_t* hPlayer)
{
	element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_STOP);

	return 0;
}

int gmf_player_play(gmf_player_t* hPlayer)
{
	element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_OPEN);

	return 0;
}

int gmf_player_pause(gmf_player_t* hPlayer)
{
	element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_PAUSE);
	return 0;
}

int gmf_player_resume(gmf_player_t* hPlayer)
{
	element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_RESUME);
	return 0;
}

int gmf_player_seek(gmf_player_t* hPlayer, unsigned long pos)
{
	return 0;
}

int gmf_player_rate(gmf_player_t* hPlayer, float rate)
{
	return 0;
}


static int create_pipeline(gmf_player_t* hPlayer, const char* name)
{
	if (hPlayer->pPipeline != NULL)
	{
		LOG_WARN("pipeline(%s) has exist!", element_get_name(hPlayer->pPipeline).c_str());
		return 0;
	}

	if (name != NULL)
	{
		hPlayer->pPipeline = element_create("Pipeline", name);
	}
	else
	{
		hPlayer->pPipeline = element_create("Pipeline", "");
	}
	if (hPlayer->pPipeline == NULL)
	{
		LOG_ERR("can`t create pipeline!");
		return -1;
	}
	LOG_DEBUG("create pipeline(%p), name:%s", hPlayer->pPipeline, element_get_name(hPlayer->pPipeline).c_str());
	element_get_parame(hPlayer->pPipeline, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_HORIZONTAL, META_DATA_VAL_TYPE_STRING));
	{
		hPlayer->pSrc = element_create("FFStreamingSrc", "");
		if (hPlayer->pSrc == NULL)
		{
			LOG_ERR("can`t create StreamingSrc!");
			return -1;
		}
		element_use_parame(hPlayer->pSrc, MetaData(META_KEY_OUTPORT_COUNT, "2", META_DATA_VAL_TYPE_INT));
		element_set_outport_parame(hPlayer->pSrc, 0, MetaData(META_KEY_MEDIA, "Video", META_DATA_VAL_TYPE_STRING));
		element_set_outport_parame(hPlayer->pSrc, 1, MetaData(META_KEY_MEDIA, "Audio", META_DATA_VAL_TYPE_STRING));

		pipeline_add_element(hPlayer->pPipeline, hPlayer->pSrc);
	}

	{
		CMediaElement* pDecBin = element_create("Bin", "decoderbin");
		if (pDecBin == NULL)
		{
			LOG_ERR("can`t create decoder bin!");
			return -1;
		}
		element_get_parame(pDecBin, MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_VERTICAL, META_DATA_VAL_TYPE_STRING));
		pipeline_add_element(hPlayer->pPipeline, pDecBin);

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

			hPlayer->pVideoRender = element_create("D3DVideoRender", "");
			if (hPlayer->pVideoRender == NULL)
			{
				LOG_ERR("can`t create video render!");
				return -1;
			}
			bin_add_element(pVDecBin, hPlayer->pVideoRender);
			bin_connect_element(pVDecBin, pVDec, hPlayer->pVideoRender);

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
		pipeline_connect_element(hPlayer->pPipeline, hPlayer->pSrc, pDecBin);
	}

	return 0;
}

static int destroy_pipeline(gmf_player_t* hPlayer)
{
	if (hPlayer->pPipeline)
	{
		if (element_get_state(hPlayer->pPipeline) > MEDIA_ELEMENT_STATE_READY)
			element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_STOP);
		LOG_INFO("Stop pipeline+++");
		element_set_state(hPlayer->pPipeline, MEDIA_ELEMENT_STATE_NONE);
		delete hPlayer->pPipeline;
		hPlayer->pPipeline = NULL;
	}

	return 0;
}