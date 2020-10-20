#ifndef _FF_AAC_ENCODER_H_
#define _FF_AAC_ENCODER_H_

#include "../MediaCore/AudioCodec.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};

class CFFAACEncoder : public CAudioCodec
{
public:
	CFFAACEncoder();
	CFFAACEncoder(const std::string &name);
	virtual ~CFFAACEncoder();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
	virtual int ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);
	std::string GetAudioSdp(int trackid);
	bool GenerateSdp();

private:
	static short m_encoderID;

	struct AVCodecContext *m_pCodecCtx;
	struct AVCodec *m_pCodec;
};


class CFFAACEncoderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFAACEncoder();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFAACEncoder(name);
	}
};


#endif  //_AAC_ENCODER_H_