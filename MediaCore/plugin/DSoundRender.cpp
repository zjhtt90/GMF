#include "DSoundRender.h"
#include "LogManager.h"
#include "../MediaBuffer.h"
#include "Util.h"

#define LOG_FILTER	"DSoundRender"

#define BUFFERNOTIFYSIZE 1024

CDSoundRender::CDSoundRender()
{
	Init("DSoundRender");
}

CDSoundRender::CDSoundRender(const std::string &name)
{
	Init(name);
}


CDSoundRender::~CDSoundRender()
{

}

void CDSoundRender::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SINK);
	m_desc.SetElementName(name);
	
	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_AUDIO), META_DATA_VAL_TYPE_STRING);
	this->SetPrivateData(data);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_inPorts[0]->SetProperty(data);

	m_desc.SetElementPortCount(1, 0);
}

void CDSoundRender::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	switch(state)
	{
	case MEDIA_ELEMENT_STATE_READY:
		{
			//m_desc.Print();
		}
		break;
	case MEDIA_ELEMENT_STATE_OPEN:
		Open();
		break;
	case MEDIA_ELEMENT_STATE_STOP:
		Close();
		break;
	default:
		break;
	}
}

int CDSoundRender::Open()
{
	//Init DirectSound
	if (FAILED(DirectSoundCreate8(NULL, &m_pDS, NULL)))
		return -1;
	if (FAILED(m_pDS->SetCooperativeLevel(m_hwnd, DSSCL_NORMAL)))
		return -1;

	DSBUFFERDESC dsbd;
	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes = MAX_AUDIO_BUF*BUFFERNOTIFYSIZE;
	//WAVE Header
	dsbd.lpwfxFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	dsbd.lpwfxFormat->wFormatTag = WAVE_FORMAT_PCM;
	/* format type */
	(dsbd.lpwfxFormat)->nChannels = m_channel;
	/* number of channels (i.e. mono, stereo...) */
	(dsbd.lpwfxFormat)->nSamplesPerSec = m_samplerate;
	/* sample rate */
	(dsbd.lpwfxFormat)->nAvgBytesPerSec = m_samplerate*GetBytesPerAudioSample((AudioSampleFormat)m_sampleFmt)*m_channel;
	/* for buffer estimation */
	(dsbd.lpwfxFormat)->nBlockAlign = GetBytesPerAudioSample((AudioSampleFormat)m_sampleFmt)*m_channel;
	/* block size of data */
	(dsbd.lpwfxFormat)->wBitsPerSample = GetBytesPerAudioSample((AudioSampleFormat)m_sampleFmt) * 8;
	/* number of bits per sample of mono data */
	(dsbd.lpwfxFormat)->cbSize = 0;

	//Creates a sound buffer object to manage audio samples. 
	HRESULT hr1;
	if (FAILED(m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL)))
	{
		return -1;
	}

	if (FAILED(m_pDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&m_pDSBuffer8)))
	{
		return -1;
	}
	//Get IDirectSoundNotify8
	if (FAILED(m_pDSBuffer8->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify)))
	{
		return -1;
	}

	DSBPOSITIONNOTIFY m_pDSPosNotify[MAX_AUDIO_BUF];

	for (int i = 0; i < MAX_AUDIO_BUF; i++)
	{
		m_pDSPosNotify[i].dwOffset = i*BUFFERNOTIFYSIZE;
		m_event[i] = ::CreateEvent(NULL, false, false, NULL);
		m_pDSPosNotify[i].hEventNotify = m_event[i];
	}

	m_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF, m_pDSPosNotify);

	//Start Playing
	m_pDSBuffer8->SetCurrentPosition(0);
	m_pDSBuffer8->Play(0, 0, DSBPLAY_LOOPING);

	m_bExit = false;
	CSThread::Start();

	return MEDIA_ERR_NONE;
}

void CDSoundRender::Close()
{
	m_bExit = true;
	Kill();

	if (m_pDSBuffer8)
	{
		m_pDSBuffer8->Stop();
		m_pDSBuffer8->Release();
		m_pDSBuffer8 = NULL;
	}

	for (int i = 0; i < MAX_AUDIO_BUF; i++)
	{
		if (m_event[i])
			::CloseHandle(m_event[i]);
		m_event[i] = NULL;
	}

	if (m_pDSNotify)
	{
		m_pDSNotify->Release();
		m_pDSNotify = NULL;
	}

	if (m_pDSBuffer)
	{
		m_pDSBuffer->Stop();
		m_pDSBuffer->Release();
		m_pDSBuffer = NULL;
	}

	if (m_pDS)
	{
		m_pDS->Release();
		m_pDS = NULL;
	}
}

void CDSoundRender::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	m_bufLock.Lock();
	if (m_bufLen + buffer->GetDataSize() <= MAX_AUDIO_BUF_SIZE)
	{
		memcpy(m_audioBuf + m_bufLen, buffer->GetData(), buffer->GetDataSize());
		m_bufLen += buffer->GetDataSize();
	}
	m_bufLock.UnLock();
}

void CDSoundRender::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	if(index == 0)
	{
		m_inPorts[0]->GetProperty(META_KEY_SAMPLE_RATE, data);
		m_desc.UpdateMetaData(data);
		m_samplerate = CUtil::convert<int, std::string>(data.mValue);


		m_inPorts[0]->GetProperty(META_KEY_CHANNEL_NUM, data);
		m_desc.UpdateMetaData(data);
		m_channel = CUtil::convert<int, std::string>(data.mValue);
		

		m_inPorts[0]->GetProperty(META_KEY_SAMPLE_FORMAT, data);
		m_desc.UpdateMetaData(data);
		m_sampleFmt = CUtil::convert<int, std::string>(data.mValue);
	}
}

void CDSoundRender::Run()
{
	DWORD res = WAIT_OBJECT_0;
	LPVOID buf = NULL;
	DWORD  buf_len = 0;
	DWORD offset = BUFFERNOTIFYSIZE;

	while (!m_bExit)
	{
		res = WaitForMultipleObjects(MAX_AUDIO_BUF, m_event, FALSE, INFINITE);

		if ((res >= WAIT_OBJECT_0) && (res <= WAIT_OBJECT_0 + 3))
		{
			m_pDSBuffer8->Lock(offset, BUFFERNOTIFYSIZE, &buf, &buf_len, NULL, NULL, 0);

			m_bufLock.Lock();
			if (m_bufLen >= buf_len)
			{
				memcpy(buf, m_audioBuf, buf_len);
				m_bufLen -= buf_len;
				memmove(m_audioBuf, m_audioBuf + buf_len, m_bufLen);
			}
			m_bufLock.UnLock();

			m_pDSBuffer8->Unlock(buf, buf_len, NULL, 0);
			offset += buf_len;
			offset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);
			//printf("this is %7d of buffer\n", offset);
		}
		else
		{
			CSThread::Sleep(100);
		}
	}
}