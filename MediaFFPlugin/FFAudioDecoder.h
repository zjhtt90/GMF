#ifndef _FF_AUDIO_DECODER_H_
#define _FF_AUDIO_DECODER_H_

#include "AudioCodec.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

#include <queue>

class FFAudioDecoder : public CAudioCodec
{
public:
	FFAudioDecoder();
	FFAudioDecoder(const std::string &name);
	virtual ~FFAudioDecoder();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

private:
	void Init(const std::string &name);

private:
	static short m_eleID;

	struct AVCodecContext *m_pCodecCtx;
	struct AVCodec *m_pCodec;
	struct AVFrame *m_dstFrameBuf;

	bool m_bGetInfo;
};


class CFFAudioDecoderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new FFAudioDecoder();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new FFAudioDecoder(name);
	}
};


#endif  //_FF_VIDEO_DECODER_H_