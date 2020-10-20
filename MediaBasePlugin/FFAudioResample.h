#ifndef _FF_AUDIO_RESAMPLE_H_
#define _FF_AUDIO_RESAMPLE_H_

#include "../MediaCore/MediaBaseFilter.h"
extern "C"
{
#include "libswresample/swresample.h"
};

class CFFAudioResample : public CMediaBaseFilter
{
public:
	CFFAudioResample();
	CFFAudioResample(const std::string &name);
	virtual ~CFFAudioResample();

	int Open();
	void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);
private:
	void Init(const std::string &name);

private:
	int m_srcSamplerate;
	int m_srcChnum;
	int m_srcFormat;
	int m_dstSamplerate;
	int m_dstChnum;
	int m_dstFormat;
	int m_sampleCount;

	SwrContext *m_pSwrCtx;
};

class CFFAudioResampleFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFAudioResample();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFAudioResample(name);
	}
};


#endif //_FF_AUDIO_RESAMPLE_H_