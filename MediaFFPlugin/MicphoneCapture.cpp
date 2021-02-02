#include "MicphoneCapture.h"

#include "../MediaCore/MediaBuffer.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"

#define LOG_FILTER	"MicphoneCapture"

#define MAX(a,b)		( (a) > (b) ? (a) : (b) )
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

CMicphoneCapture::CMicphoneCapture() : m_sampleRate(44100), m_channel(2)
	, m_pCaptureGUID(NULL),m_pDSCapture(NULL),m_pDSBCapture(NULL),m_pDSNotify(NULL),m_hNotificationEvent(NULL)
	, m_dwCaptureBufferSize(0),m_dwNextCaptureOffset(0),m_dwNotifySize(0)
{
	Init("MicphoneCapture");
}

CMicphoneCapture::CMicphoneCapture(const std::string &name) : m_sampleRate(44100), m_channel(2)
	, m_pCaptureGUID(NULL),m_pDSCapture(NULL),m_pDSBCapture(NULL),m_pDSNotify(NULL),m_hNotificationEvent(NULL)
	, m_dwCaptureBufferSize(0),m_dwNextCaptureOffset(0),m_dwNotifySize(0)
{
	Init(name);
}

CMicphoneCapture::~CMicphoneCapture()
{
}

void CMicphoneCapture::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	MetaData media(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_AUDIO), META_DATA_VAL_TYPE_STRING);
	m_desc.SetExternData(media);
	m_outPorts[0]->SetProperty(media);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);

	m_hNotificationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSoundEnumCallback, (VOID*)this);

}

BOOL CALLBACK CMicphoneCapture::DSoundEnumCallback(GUID* pGUID, LPSTR strDesc, LPSTR strDrvName, VOID* pContext)
{
	CMicphoneCapture *cap = reinterpret_cast<CMicphoneCapture*>(pContext);

	if(pGUID && cap->m_devName == std::string(strDesc))
	{
		cap->m_pCaptureGUID = pGUID;
	}

	return TRUE;
}

HRESULT CMicphoneCapture::InitNotifications()
{
	HRESULT hr; 

	if( NULL == m_pDSBCapture )
		return E_FAIL;

	// Create a notification event, for when the sound stops playing
	if(FAILED( hr = m_pDSBCapture->QueryInterface(IID_IDirectSoundNotify, (VOID**)&m_pDSNotify ) ) )
		return hr;

	// Setup the notification positions
	for( INT i = 0; i < NUM_REC_NOTIFICATIONS; i++ )
	{
		m_aPosNotify[i].dwOffset = (m_dwNotifySize * i) + m_dwNotifySize - 1;
		m_aPosNotify[i].hEventNotify = m_hNotificationEvent;             
	}

	// Tell DirectSound when to notify us. the notification will come in the from 
	// of signaled events that are handled in WinMain()
	if(FAILED( hr = m_pDSNotify->SetNotificationPositions( NUM_REC_NOTIFICATIONS, 
		m_aPosNotify ) ) )
		return hr;

	return S_OK;
}

int CMicphoneCapture::Open()
{
	LOG_INFO("Start MicphoneCapture");
	HRESULT hr;

	CoInitialize(NULL);

	if(hr = DirectSoundCaptureCreate(m_pCaptureGUID, &m_pDSCapture, NULL) < 0)
		goto err;

	WAVEFORMATEX wfx = {WAVE_FORMAT_PCM};
	wfx.nChannels = m_channel;
	wfx.nSamplesPerSec = m_sampleRate;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (wfx.wBitsPerSample/8) * m_channel;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * m_sampleRate;

	// Set the notification size
	m_dwNotifySize = MAX( 1024, wfx.nAvgBytesPerSec / 8 );
	m_dwNotifySize -= m_dwNotifySize % wfx.nBlockAlign;   

	// Set the buffer sizes 
	m_dwCaptureBufferSize = m_dwNotifySize * NUM_REC_NOTIFICATIONS;
	DSCBUFFERDESC dscbd = {0};
	dscbd.dwSize = sizeof(DSCBUFFERDESC);
	dscbd.dwFlags = 0;
	dscbd.dwBufferBytes = m_dwCaptureBufferSize;
	dscbd.dwReserved = 0;
	dscbd.lpwfxFormat = &wfx;
	dscbd.dwFXCount = 0;
	dscbd.lpDSCFXDesc = NULL;

	if(FAILED(m_pDSCapture->CreateCaptureBuffer(&dscbd, &m_pDSBCapture, NULL )))
		goto err;

	m_dwNextCaptureOffset = 0;

	if(FAILED(InitNotifications()))
		goto err;

	if( FAILED(m_pDSBCapture->Start( DSCBSTART_LOOPING )))
		goto err;

	return MEDIA_ERR_NONE;

err:
	SAFE_RELEASE(m_pDSNotify);
	SAFE_RELEASE(m_pDSBCapture);
	SAFE_RELEASE(m_pDSCapture);
	return MEDIA_ERR_INVALIDE_PARAME;
}

void CMicphoneCapture::Close()
{
	LOG_INFO("Stop MicphoneCapture");
	m_pDSBCapture->Stop();

	SAFE_RELEASE( m_pDSNotify );
	SAFE_RELEASE( m_pDSBCapture );
	SAFE_RELEASE( m_pDSCapture ); 

	CloseHandle(m_hNotificationEvent);

	CoUninitialize();
}

void CMicphoneCapture::SetState(MediaElementState state)
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
			//m_sampleFmt = CUtil::convert<int, std::string>(data.mValue);
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
		m_pDSBCapture->Stop();
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_RESUME)
	{
		m_pDSBCapture->Start( DSCBSTART_LOOPING );
		m_outPorts[0]->StartTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();

		Close();
	}
}

HRESULT CMicphoneCapture::RecordCapturedData(BYTE **pData, int &lLength) 
{
	HRESULT hr;
	VOID*   pbCaptureData    = NULL;
	DWORD   dwCaptureLength;
	VOID*   pbCaptureData2   = NULL;
	DWORD   dwCaptureLength2;
	UINT    dwDataWrote;
	DWORD   dwReadPos;
	DWORD   dwCapturePos;
	LONG lLockSize;

	if( NULL == m_pDSBCapture )
		return S_FALSE;

	if( FAILED( hr = m_pDSBCapture->GetCurrentPosition( &dwCapturePos, &dwReadPos ) ) )
		return hr;

	lLockSize = dwReadPos - m_dwNextCaptureOffset;
	if( lLockSize < 0 )
		lLockSize += m_dwCaptureBufferSize;

	// Block align lock size so that we are always write on a boundary
	lLockSize -= (lLockSize % m_dwNotifySize);

	if( lLockSize == 0 )
		return S_FALSE;

	// Lock the capture buffer down
	if( FAILED( hr = m_pDSBCapture->Lock( m_dwNextCaptureOffset, lLockSize, 
		&pbCaptureData, &dwCaptureLength, 
		&pbCaptureData2, &dwCaptureLength2, 0L ) ) )
		return hr;

	lLength = 0;
	lLength += dwCaptureLength;

	// Move the capture offset along
	m_dwNextCaptureOffset += dwCaptureLength; 
	m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

	if( pbCaptureData2 != NULL )
	{
		lLength += dwCaptureLength2;

		// Move the capture offset along
		m_dwNextCaptureOffset += dwCaptureLength2; 
		m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
	}

	*pData = new BYTE[lLength];
	memcpy(*pData, pbCaptureData, dwCaptureLength);
	if(pbCaptureData2 != NULL)
	{
		memcpy(*pData+dwCaptureLength, pbCaptureData2, dwCaptureLength2);
	}

	// Unlock the capture buffer
	m_pDSBCapture->Unlock( pbCaptureData,  dwCaptureLength, 
		pbCaptureData2, dwCaptureLength2 );


	return S_OK;
}

int CMicphoneCapture::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int res = MEDIA_ERR_NONE;

	BYTE *data = NULL;
	int len;
	RecordCapturedData(&data, len);

	if(len > 0 && data != NULL)
	{
		long long ts = CUtil::GetTimeStamp();
		*buffer = new CMediaBuffer(data, len, ts, ts, 0);
		delete data;

		return MEDIA_ERR_NONE;
	}

	return MEDIA_ERR_READ_FAILED;
}