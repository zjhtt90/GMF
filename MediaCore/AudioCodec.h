#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#include "MediaBaseFilter.h"

class API_EXPORT CAudioCodec : public CMediaBaseFilter
{
public:
	CAudioCodec();
	virtual ~CAudioCodec();

	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void PortSettingChanged(TRACKID index, const std::string &key);

protected:
	int m_codecID;
	int m_sampleRate;
	int m_channel;
	int m_sampleFmt;
};


#endif  //_AUDIO_CODEC_H_