#include "SpeakerCapture.h"

#include "../../Common/LogManager.h"
#include "../../Common/Util.h"

#include "../MediaBuffer.h"

#define LOG_FILTER	"SpeakerCapture"

CSpeakerCapture::CSpeakerCapture() : m_sampleRate(44100), m_channel(2), m_sampleFmt(AUDIO_SAMPLE_FMT_S16),
	m_pDevice(NULL), m_pAudioClient(NULL), m_pAudioCaptureClient(NULL)
{
	Init("SpeakerCapture");
}

CSpeakerCapture::CSpeakerCapture(const std::string &name) : m_sampleRate(44100), m_channel(2), m_sampleFmt(AUDIO_SAMPLE_FMT_S16),
	m_pDevice(NULL), m_pAudioClient(NULL), m_pAudioCaptureClient(NULL)
{
	Init(name);
}

CSpeakerCapture::~CSpeakerCapture()
{
}

void CSpeakerCapture::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	MetaData media(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_AUDIO), META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(media);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

IMMDevice* CSpeakerCapture::GetDefaultDevice(LPWSTR *device_id)
{
	IMMDevice* pDevice = NULL;
	IPropertyStore *pStore = NULL;
	IMMDeviceEnumerator *pMMDeviceEnumerator = NULL;

	HRESULT hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
		__uuidof(IMMDeviceEnumerator),
		(void**)&pMMDeviceEnumerator);
	if(FAILED(hr)) return NULL;

	hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	if(FAILED(hr)) return NULL;
	pDevice->OpenPropertyStore(STGM_READ, &pStore);
	if (pStore)
		pDevice->GetId(device_id);
	pMMDeviceEnumerator->Release();

	return pDevice;
}

static BOOL AdjustFormatTo16Bits(WAVEFORMATEX *pwfx)
{
	BOOL bRet = FALSE;

	if(pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
        pwfx->wBitsPerSample = 16;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

		bRet = TRUE;
	}
	else if(pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
        PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat))
		{
           pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		   //pwfx->nChannels = 1;
		   //pwfx->nSamplesPerSec = 8000;
           pEx->Samples.wValidBitsPerSample = 16;
           pwfx->wBitsPerSample = 16;
           pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
           pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;

		   bRet = TRUE;
        } 
	}

	return bRet;
}

void CSpeakerCapture::CheckParam()
{
	CoInitialize(NULL);

	HRESULT hr;
	LPWSTR devID = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	WAVEFORMATEX *pwfx;

	pDevice = GetDefaultDevice(&devID);
	if (pDevice == NULL)
		goto error;

	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr))
		goto error;

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = m_channel;
	format.nSamplesPerSec = m_sampleRate;
	format.wBitsPerSample = ((m_sampleFmt == AUDIO_SAMPLE_FMT_U8) ? 8 : 16);
	format.cbSize = 0;
	format.nBlockAlign = (format.wBitsPerSample / 8)*format.nChannels;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &format, &pwfx);
	if (hr == S_FALSE)
	{
		if (!AdjustFormatTo16Bits(pwfx))
		{
			goto error;
		}

		std::string strVal;
		m_sampleRate = pwfx->nSamplesPerSec;
		strVal = CUtil::convert<std::string, int>(m_sampleRate);
		m_outPorts[0]->UpdateProperty(MetaData(META_KEY_SAMPLE_RATE, strVal, META_DATA_VAL_TYPE_INT));

		m_channel = pwfx->nChannels;
		strVal = CUtil::convert<std::string, int>(m_channel);
		m_outPorts[0]->UpdateProperty(MetaData(META_KEY_CHANNEL_NUM, strVal, META_DATA_VAL_TYPE_INT));

		m_sampleFmt = ((pwfx->wBitsPerSample == 8) ? AUDIO_SAMPLE_FMT_U8 : AUDIO_SAMPLE_FMT_S16);
		strVal = CUtil::convert<std::string, int>(m_sampleFmt);
		m_outPorts[0]->UpdateProperty(MetaData(META_KEY_SAMPLE_FORMAT, strVal, META_DATA_VAL_TYPE_INT));
	}
	else if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT
		|| hr == E_POINTER || hr == AUDCLNT_E_DEVICE_INVALIDATED
		|| hr == AUDCLNT_E_SERVICE_NOT_RUNNING)
	{
		goto error;
	}

error:
	if (pwfx != NULL)
	{
		CoTaskMemFree(pwfx);
		pwfx = NULL;
	}
	if (pAudioClient != NULL)
	{
		pAudioClient->Release();
		pAudioClient = NULL;
	}

	if (pDevice)
	{
		pDevice->Release();
		pDevice = NULL;
	}
	CoUninitialize();
}

int CSpeakerCapture::Open()
{
	CoInitialize(NULL);

	HRESULT hr;
	WAVEFORMATEX* pwfx = NULL;

	LPWSTR devID = NULL;
	m_pDevice = GetDefaultDevice(&devID);
	if(m_pDevice == NULL) 
		goto error;

	hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
	if(FAILED(hr)) 
		goto error;
	/*
	hr = m_pAudioClient->GetMixFormat(&m_pwfx);
	if (FAILED(hr)) 
		goto error;
	*/

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = m_channel;
	format.nSamplesPerSec = m_sampleRate;
	format.wBitsPerSample = ((m_sampleFmt== AUDIO_SAMPLE_FMT_U8) ? 8 : 16);
	format.cbSize = 0;
	format.nBlockAlign = (format.wBitsPerSample/8)*format.nChannels;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &format, &pwfx);
	if(hr == S_FALSE)
	{
		if (pwfx)
		{
			m_sampleRate = pwfx->nSamplesPerSec;

			m_channel = pwfx->nChannels;

			m_sampleFmt = ((pwfx->wBitsPerSample == 8) ? AUDIO_SAMPLE_FMT_U8 : AUDIO_SAMPLE_FMT_S16);
		}
	}
	else if(hr == AUDCLNT_E_UNSUPPORTED_FORMAT 
		|| hr == E_POINTER || hr == AUDCLNT_E_DEVICE_INVALIDATED
		|| hr == AUDCLNT_E_SERVICE_NOT_RUNNING)
	{
		goto error;
	}

	if (pwfx)
	{
		if (!AdjustFormatTo16Bits(pwfx))
		{
			goto error;
		}
		memcpy(&format, pwfx, sizeof(WAVEFORMATEX));
	}
	hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, &format, 0);
	if(FAILED(hr)) 
		goto error;

	hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pAudioCaptureClient);
	if(FAILED(hr)) 
		goto error;

	hr = m_pAudioClient->Start();
	if(FAILED(hr)) 
		goto error;


	return MEDIA_ERR_NONE;

error:
	if(m_pAudioCaptureClient != NULL)
	{
		m_pAudioCaptureClient->Release();
		m_pAudioCaptureClient = NULL;
	}

	if(pwfx != NULL)
	{
		CoTaskMemFree(pwfx);
		pwfx = NULL;
	}
	if(m_pAudioClient != NULL)
	{
		m_pAudioClient->Release();
		m_pAudioClient = NULL;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
	CoUninitialize();
	return MEDIA_ERR_INVALIDE_PARAME;
}

void CSpeakerCapture::Close()
{
	if(m_pAudioCaptureClient != NULL)
	{
		m_pAudioCaptureClient->Release();
		m_pAudioCaptureClient = NULL;
	}

	if(m_pAudioClient != NULL)
	{

		m_pAudioClient->Stop();

		m_pAudioClient->Release();
		m_pAudioClient = NULL;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}
	CoUninitialize();
}

void CSpeakerCapture::SetState(MediaElementState state)
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

		if(m_desc.GetMetaData(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_sampleRate = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_channel = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->SetProperty(data);
			m_sampleFmt = CUtil::convert<int, std::string>(data.mValue);
		}

		CheckParam();

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
		m_pAudioClient->Stop();
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_RESUME)
	{
		m_pAudioClient->Start();
		m_outPorts[0]->StartTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();

		Close();
	}
}

int CSpeakerCapture::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	HRESULT hr;
	UINT32 nNextPacketSize(0);
	BYTE *pData = NULL;
	UINT32 nNumFramesToRead;
	DWORD dwFlags;
	//BYTE dummyData[1024*4] = { 0 };

	hr = m_pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
	if(FAILED(hr))
		return MEDIA_ERR_READ_FAILED;
	if (nNextPacketSize == 0) 
	{ // no audio data case, fill dummy data
		//long long ts = CUtil::GetTimeStamp();
		//*buffer = new CMediaBuffer(dummyData, sizeof(dummyData), ts, ts, 0);
	}
	else 
	{
		hr = m_pAudioCaptureClient->GetBuffer(
			&pData,
			&nNumFramesToRead,
			&dwFlags,
			NULL,
			NULL
			);
		if(FAILED(hr))
		{
			return MEDIA_ERR_READ_FAILED;
		}

		long long ts = CUtil::GetTimeStamp();
		*buffer = new CMediaBuffer(pData, nNumFramesToRead*4, ts, ts, 0);
	
		m_pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
	}

	return MEDIA_ERR_NONE;
}