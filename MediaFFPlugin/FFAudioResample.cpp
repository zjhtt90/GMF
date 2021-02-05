#include "FFAudioResample.h"
#include "common.h"
#include "MediaBuffer.h"
#include "Util.h"
#include "LogManager.h"

#define LOG_FILTER	"FFAudioResample"


CFFAudioResample::CFFAudioResample()
	: m_pSwrCtx(NULL)
{
	Init("FFAudioResample");
}

CFFAudioResample::CFFAudioResample(const std::string &name)
	: m_pSwrCtx(NULL)
{
	Init(name);
}

CFFAudioResample::~CFFAudioResample()
{
}

void CFFAudioResample::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_desc.SetElementPortCount(1, 1);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);
}

int CFFAudioResample::Open()
{
	LOG_INFO("Start Audio Resample");
	m_pSwrCtx = swr_alloc_set_opts(m_pSwrCtx, av_get_default_channel_layout(m_dstChnum), (AVSampleFormat)m_dstFormat, m_dstSamplerate,
		av_get_default_channel_layout(m_srcChnum), (AVSampleFormat)m_srcFormat, m_srcSamplerate,
		0, 0);
	if (m_pSwrCtx == NULL)
	{
		LOG_ERR("Create convert faild!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}
	if (swr_init(m_pSwrCtx) < 0)
	{
		LOG_ERR("Init swr ctx error!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}
	return MEDIA_ERR_NONE;
}

void CFFAudioResample::Close()
{
	if (m_pSwrCtx)
	{
		swr_close(m_pSwrCtx);
		swr_free(&m_pSwrCtx);
	}
}

void CFFAudioResample::SetState(MediaElementState state)
{
	if (state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		if (m_desc.GetMetaData(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
		{
			m_dstSamplerate = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstSamplerate = m_srcSamplerate;
			m_inPorts[0]->GetProperty(META_KEY_SAMPLE_RATE, data);
			m_outPorts[0]->SetProperty(data);
		}

		if (m_desc.GetMetaData(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
		{
			m_dstChnum = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstChnum = m_srcChnum;
			m_inPorts[0]->GetProperty(META_KEY_CHANNEL_NUM, data);
			m_outPorts[0]->SetProperty(data);
		}

		if (m_desc.GetMetaData(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_dstFormat = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}
		else
		{
			m_dstFormat = m_srcFormat;
			m_inPorts[0]->GetProperty(META_KEY_SAMPLE_FORMAT, data);
			m_outPorts[0]->SetProperty(data);
		}
	}
	else if (state == MEDIA_ELEMENT_STATE_OPEN)
	{
		Open();
	}
	else if (state == MEDIA_ELEMENT_STATE_STOP)
	{
		Close();
	}
}

static void DumpData(const char* file, const unsigned char* pData, int size, int append)
{
	FILE* fp = NULL;
	if (append)
	{
		fp = fopen(file, "ab");
	}
	else
	{
		fp = fopen(file, "wb");
	}

	if (fp)
	{
		fwrite(pData, 1, size, fp);
		fclose(fp);
	}
}

void CFFAudioResample::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	//DumpData("audio_ori", buffer->GetData(), buffer->GetDataSize(), 1);

	int ret = 0;
	unsigned char* srcData[4] = { NULL };
	int srcline[4] = { 0 };
	av_samples_fill_arrays(srcData, srcline, buffer->GetData(), m_srcChnum, m_sampleCount, (AVSampleFormat)m_srcFormat, 1);

	unsigned char* dstData[4] = { NULL };
	int dstline[4] = { 0 };
	int out_samples = av_rescale_rnd(m_sampleCount, m_dstSamplerate, m_srcSamplerate, AV_ROUND_UP);
	//int out_samples = av_rescale_rnd(swr_get_delay(m_pSwrCtx, m_srcSamplerate) +
	//	m_sampleCount, m_dstSamplerate, m_srcSamplerate, AV_ROUND_UP);
	av_samples_alloc(dstData, dstline, m_dstChnum, out_samples, (AVSampleFormat)m_dstFormat, 1);
	ret = swr_convert(m_pSwrCtx, dstData, out_samples, (const uint8_t**)srcData, m_sampleCount);
	if (ret < 0)
	{
		LOG_ERR("Error while converting\n");

		av_free(dstData[0]);
		return;
	}

	//av_samples_get_buffer_size(dstline, m_dstChnum, ret, (AVSampleFormat)m_dstFormat, 1);

	//DumpData("audio_new", dstData[0], dstline[0], 1);

	CMediaBuffer *sBuf = new CMediaBuffer(dstData[0], dstline[0], buffer->GetPts(), buffer->GetDts(), buffer->GetDuration());
	av_free(dstData[0]);

	m_outPorts[0]->PushBufferToDownStream(sBuf);
	delete sBuf;
}

int CFFAudioResample::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

void CFFAudioResample::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[0]->GetProperty(key, data);

	if (key == META_KEY_SAMPLE_RATE)
	{
		m_srcSamplerate = CUtil::convert<int, std::string>(data.mValue);
	}
	else if (key == META_KEY_CHANNEL_NUM)
	{
		m_srcChnum = CUtil::convert<int, std::string>(data.mValue);
	}
	else if (key == META_KEY_SAMPLE_FORMAT)
	{
		m_srcFormat = CUtil::convert<int, std::string>(data.mValue);
	}
	else if (key == META_KEY_FRAME_SAMPLE_COUNT)
	{
		m_sampleCount = CUtil::convert<int, std::string>(data.mValue);
	}
	else
	{
		m_outPorts[0]->SetProperty(data);
	}
}