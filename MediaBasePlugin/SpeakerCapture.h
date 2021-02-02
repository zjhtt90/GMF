#ifndef _SPEAKER_CAPTURE_H_
#define _SPEAKER_CAPTURE_H_

#include "MediaBaseSrc.h"

#ifdef SYSTEM_WIN32
#include <mmdeviceapi.h>
#include <Audioclient.h>
#endif

class CSpeakerCapture : public CMediaBaseSrc
{
public:
	CSpeakerCapture();
	CSpeakerCapture(const std::string &name);
	virtual ~CSpeakerCapture();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);
	IMMDevice* GetDefaultDevice(LPWSTR *device_id);
	void CheckParam();

private:
	std::string m_devName;
	int m_sampleRate;
	int m_channel;
	int m_sampleFmt;
#ifdef SYSTEM_WIN32
	IMMDevice* m_pDevice;
	IAudioClient *m_pAudioClient;
	IAudioCaptureClient *m_pAudioCaptureClient;
#endif
};

class CSpeakerCaptureFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CSpeakerCapture();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CSpeakerCapture(name);
	}
};


#endif  //_SPEAKER_CAPTURE_H_