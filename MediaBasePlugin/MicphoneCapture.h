#ifndef _MIC_CAPTURE_H_
#define _MIC_CAPTURE_H_

#include "../MediaCore/MediaBaseSrc.h"

#ifdef _WIN32
#include <dsound.h>
#pragma comment(lib, "../third_part/DXSDK9/lib/dsound.lib")
#pragma comment(lib, "../third_part/DXSDK9/lib/dxguid.lib")
#define NUM_REC_NOTIFICATIONS  16
#endif

#include <array>

#define MAX_BYTES_LENGTH	(1024 * 1024)

class CMicphoneCapture : public CMediaBaseSrc
{
public:
	CMicphoneCapture();
	CMicphoneCapture(const std::string &name);
	virtual ~CMicphoneCapture();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

private:
	void Init(const std::string &name);
#ifdef _WIN32
	static BOOL CALLBACK DSoundEnumCallback(GUID* pGUID, LPSTR strDesc, LPSTR strDrvName, VOID* pContext);
	HRESULT InitNotifications();
	HRESULT RecordCapturedData(BYTE **pData, int &lLength);
#endif

private:
	std::string m_devName;
	int m_sampleRate;
	int m_channel;
	int m_sampleFmt;
#ifdef _WIN32
	GUID* m_pCaptureGUID;
	LPDIRECTSOUNDCAPTURE m_pDSCapture;
	LPDIRECTSOUNDCAPTUREBUFFER m_pDSBCapture;
	LPDIRECTSOUNDNOTIFY        m_pDSNotify;
	DSBPOSITIONNOTIFY          m_aPosNotify[ NUM_REC_NOTIFICATIONS + 1 ];
	HANDLE                     m_hNotificationEvent;
	DWORD                      m_dwCaptureBufferSize;
	DWORD                      m_dwNextCaptureOffset;
	DWORD                      m_dwNotifySize;

	std::array<unsigned char, MAX_BYTES_LENGTH> m_bytesBuf;
	//unsigned char m_bytesBuf[MAX_BYTES_LENGTH];
#endif
};



class CMicphoneCaptureFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CMicphoneCapture();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CMicphoneCapture(name);
	}
};



#endif  //_MIC_CAPTURE_H_