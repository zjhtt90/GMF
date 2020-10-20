#ifndef _FF_VIDEO_DECODER_H_
#define _FF_VIDEO_DECODER_H_

#include "VideoCodec.h"

#include "SThread.h"
#include "SMutex.h"
#include "SEvent.h"
extern "C"
{
#include "libavcodec/avcodec.h"
};

#include <queue>

class FFVideoDecoder : public CVideoCodec, public CSThread
{
public:
	FFVideoDecoder();
	FFVideoDecoder(const std::string &name);
	virtual ~FFVideoDecoder();

	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
	virtual int ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf);

private:
	void Init(const std::string &name);
	virtual void Run();

private:
	static short m_eleID;

	struct AVCodecContext *m_pCodecCtx;
	struct AVCodec *m_pCodec;
	struct AVFrame *m_dstFrameBuf;

	CSMutex m_bufLock;
	std::queue<CMediaBuffer*> m_inBufs;
	CSEvent m_pauseEvent;

	bool m_bGetInfo;
};


class CFFVideoDecoderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new FFVideoDecoder();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new FFVideoDecoder(name);
	}
};


#endif  //_FF_VIDEO_DECODER_H_