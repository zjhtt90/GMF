#ifndef _FF_H264_ENCODER_H_
#define _FF_H264_ENCODER_H_

#include "../MediaCore/VideoCodec.h"

#include "../Common/SThread.h"
#include "../Common/SMutex.h"
#include "../Common/SEvent.h"
extern "C"
{
#include "libavcodec/avcodec.h"
};

#include <queue>

class CFFH264Encoder : public CVideoCodec, public CSThread
{
public:
	CFFH264Encoder();
	CFFH264Encoder(const std::string &name);
	virtual ~CFFH264Encoder();

	void SetConfig(struct BitrateControl bitrate, int qmin, int qmax, Preset preset, Profile profile, int interval);
	virtual int Open();
	virtual void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
	virtual int ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);
	void InitParam(AVCodecContext *ctx);
	virtual void Run();

	bool GenerateSdp();

	static char* ExtraDataToPsets(AVCodecContext *c);
	// 该函数来自ffmpeg库
	static const uint8_t *ff_avc_find_startcode_internal(const uint8_t *p, const uint8_t *end);
	// 该函数来自ffmpeg库
	static const uint8_t *ff_avc_find_startcode(const uint8_t *p, const uint8_t *end);
	// 该函数来自ffmpeg库
	static char *ff_data_to_hex(char *buff, const uint8_t *src, int s, int lowercase);

private:
	static short m_eleID;

	struct AVCodecContext *m_pCodecCtx;
	struct AVCodec *m_pCodec;

	struct BitrateControl m_bitrate; // 码率
	int m_qmin, m_qmax; // 最小最大量化值
	Preset m_preset; // 预设值
	int m_IFrameInterval; // I帧间隔
	Profile m_profile;

	std::string m_licPath; // H264/H265 licence文件路径


	CSMutex m_bufLock;
	std::queue<CMediaBuffer*> m_inBufs;
	CSEvent m_pauseEvent;

	std::string m_sdpSPS;

	std::string m_codecInfo;

	static const int MAX_PSET_SIZE = 1024;
	static const int MAX_EXTRADATA_SIZE = 1024;
	static const int NAL_SPS = 7;
	static const int NAL_PPS = 8;

	bool m_bForceKey;
};


class CFFH264EncoderFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFH264Encoder();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFH264Encoder(name);
	}
};


#endif  //_H264_ENCODER_H_