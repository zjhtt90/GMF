#ifndef _DSOUND_RENDER_H_
#define _DSOUND_RENDER_H_

#include "MediaBaseSink.h"
#include "SThread.h"
#include "SMutex.h"
#include <mmeapi.h>
#include <dsound.h>

#define MAX_AUDIO_BUF 4
#define MAX_AUDIO_BUF_SIZE	(1024*10)

class API_EXPORT CDSoundRender : public CMediaBaseSink, public CSThread
{
public:
	CDSoundRender();
	CDSoundRender(const std::string &name);
	virtual ~CDSoundRender();
	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual void PortSettingChanged(TRACKID index, const std::string &key);

private:
	void Init(const std::string &name);
	virtual void Run();

private:
	int m_samplerate;
	int m_channel;
	int m_sampleFmt;

	IDirectSound8 *m_pDS;
	IDirectSoundBuffer8 *m_pDSBuffer8;	//used to manage sound buffers.
	IDirectSoundBuffer *m_pDSBuffer;
	IDirectSoundNotify8 *m_pDSNotify;
	HANDLE m_event[MAX_AUDIO_BUF];
	HWND m_hwnd;

	unsigned char m_audioBuf[MAX_AUDIO_BUF_SIZE];
	int m_bufLen;
	CSMutex m_bufLock;

	bool m_bExit;
};


class CDSoundRenderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CDSoundRender();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CDSoundRender(name);
	}
};


#endif  //_DSOUND_RENDER_H_