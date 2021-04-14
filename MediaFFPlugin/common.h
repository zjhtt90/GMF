#ifndef _COMMON_H_
#define _COMMON_H_

#include "MediaDefs.h"

extern "C"
{
#include "libavutil/error.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/avcodec.h"
};

inline AVCodecID FFCodecType(MediaCodecType type)
{
	AVCodecID codecID = AV_CODEC_ID_NONE;
	switch (type)
	{
	case MediaCodec_MPEG1VIDEO:
		codecID = AV_CODEC_ID_MPEG1VIDEO;
		break;
	case MediaCodec_MPEG2VIDEO:
		codecID = AV_CODEC_ID_MPEG2VIDEO;
		break;
	case MediaCodec_H261:
		codecID = AV_CODEC_ID_H261;
		break;
	case MediaCodec_H263:
		codecID = AV_CODEC_ID_H263;
		break;
	case MediaCodec_MJPEG:
		codecID = AV_CODEC_ID_MJPEG;
		break;
	case MediaCodec_MPEG4:
		codecID = AV_CODEC_ID_MPEG4;
		break;
	case MediaCodec_H264:
		codecID = AV_CODEC_ID_H264;
		break;
	case MediaCodec_H265:
		codecID = AV_CODEC_ID_H265;
		break;
	case MediaCodec_PCM:
		codecID = AV_CODEC_ID_PCM_S16LE;
		break;
	case MediaCodec_MULAW:
		codecID = AV_CODEC_ID_PCM_MULAW;
		break;
	case MediaCodec_ALAW:
		codecID = AV_CODEC_ID_PCM_ALAW;
		break;
	case MediaCodec_ADPCM:
		codecID = AV_CODEC_ID_ADPCM_G722;
		break;
	case MediaCodec_AMR_NB:
		codecID = AV_CODEC_ID_AMR_NB;
		break;
	case MediaCodec_AMR_WB:
		codecID = AV_CODEC_ID_AMR_WB;
		break;
	case MediaCodec_MP3:
		codecID = AV_CODEC_ID_MP3;
		break;
	case MediaCodec_AAC:
		codecID = AV_CODEC_ID_AAC;
		break;
	case MediaCodec_AC3:
		codecID = AV_CODEC_ID_AC3;
		break;
	case MediaCodec_WMAV1:
		codecID = AV_CODEC_ID_WMAV1;
		break;
	case MediaCodec_WMAV2:
		codecID = AV_CODEC_ID_WMAV2;
		break;
	case MediaCodec_SPEEX:
		codecID = AV_CODEC_ID_SPEEX;
		break;
	case MediaCodec_G723:
		codecID = AV_CODEC_ID_G723_1;
		break;
	case MediaCodec_G729:
		codecID = AV_CODEC_ID_G729;
		break;
	case MediaCodec_ILBC:
		codecID = AV_CODEC_ID_ILBC;
		break;
	case MediaCodec_OPUS:
		codecID = AV_CODEC_ID_OPUS;
		break;
	default:
		break;
	}

	return codecID;
}

inline MediaCodecType LocalCodecType(AVCodecID type)
{
	MediaCodecType codecType = MediaCodec_NONE;
	switch (type)
	{
	case AV_CODEC_ID_MPEG1VIDEO:
		codecType = MediaCodec_MPEG1VIDEO;
		break;
	case AV_CODEC_ID_MPEG2VIDEO:
		codecType = MediaCodec_MPEG2VIDEO;
		break;
	case AV_CODEC_ID_H261:
		codecType = MediaCodec_H261;
		break;
	case AV_CODEC_ID_H263:
		codecType = MediaCodec_H263;
		break;
	case AV_CODEC_ID_MJPEG:
		codecType = MediaCodec_MJPEG;
		break;
	case AV_CODEC_ID_MPEG4:
		codecType = MediaCodec_MPEG4;
		break;
	case AV_CODEC_ID_H264:
		codecType = MediaCodec_H264;
		break;
	case AV_CODEC_ID_H265:
		codecType = MediaCodec_H265;
		break;
	case AV_CODEC_ID_PCM_S16LE:
		codecType = MediaCodec_PCM;
		break;
	case AV_CODEC_ID_PCM_MULAW:
		codecType = MediaCodec_MULAW;
		break;
	case AV_CODEC_ID_PCM_ALAW:
		codecType = MediaCodec_ALAW;
		break;
	case AV_CODEC_ID_ADPCM_G722:
		codecType = MediaCodec_ADPCM;
		break;
	case AV_CODEC_ID_AMR_NB:
		codecType = MediaCodec_AMR_NB;
		break;
	case AV_CODEC_ID_AMR_WB:
		codecType = MediaCodec_AMR_WB;
		break;
	case AV_CODEC_ID_MP3:
		codecType = MediaCodec_MP3;
		break;
	case AV_CODEC_ID_AAC:
		codecType = MediaCodec_AAC;
		break;
	case AV_CODEC_ID_AC3:
		codecType = MediaCodec_AC3;
		break;
	case AV_CODEC_ID_WMAV1:
		codecType = MediaCodec_WMAV1;
		break;
	case AV_CODEC_ID_WMAV2:
		codecType = MediaCodec_WMAV2;
		break;
	case AV_CODEC_ID_SPEEX:
		codecType = MediaCodec_SPEEX;
		break;
	case AV_CODEC_ID_G723_1:
		codecType = MediaCodec_G723;
		break;
	case AV_CODEC_ID_G729:
		codecType = MediaCodec_G729;
		break;
	case AV_CODEC_ID_ILBC:
		codecType = MediaCodec_ILBC;
		break;
	case AV_CODEC_ID_OPUS:
		codecType = MediaCodec_OPUS;
		break;
	default:
		break;
	}

	return codecType;
}



inline AVPixelFormat FFPixFormat(VideoPixformat fmt)
{
	AVPixelFormat vFmt = AV_PIX_FMT_NONE;
	switch (fmt)
	{
	case VIDEO_PIX_FMT_YUV420P:
		vFmt = AV_PIX_FMT_YUV420P;
		break;
	case VIDEO_PIX_FMT_YUYV422:
		vFmt = AV_PIX_FMT_YUYV422;
		break;
	case VIDEO_PIX_FMT_YUVJ420P:
		vFmt = AV_PIX_FMT_YUVJ420P;
		break;
	case VIDEO_PIX_FMT_NV12:
		vFmt = AV_PIX_FMT_NV12;
		break;
	case VIDEO_PIX_FMT_NV21:
		vFmt = AV_PIX_FMT_NV21;
		break;
	case VIDEO_PIX_FMT_GRAY8:
		vFmt = AV_PIX_FMT_GRAY8;
		break;
	case VIDEO_PIX_FMT_RGB555:
		vFmt = AV_PIX_FMT_RGB555;
		break;
	case VIDEO_PIX_FMT_BGR555:
		vFmt = AV_PIX_FMT_BGR555;
		break;
	case VIDEO_PIX_FMT_RGB565:
		vFmt = AV_PIX_FMT_RGB565;
		break;
	case VIDEO_PIX_FMT_BGR565:
		vFmt = AV_PIX_FMT_BGR565;
		break;
	case VIDEO_PIX_FMT_RGB24:
		vFmt = AV_PIX_FMT_RGB24;
		break;
	case VIDEO_PIX_FMT_BGR24:
		vFmt = AV_PIX_FMT_BGR24;
		break;
	case VIDEO_PIX_FMT_ARGB:
		vFmt = AV_PIX_FMT_ARGB;
		break;
	case VIDEO_PIX_FMT_RGBA:
		vFmt = AV_PIX_FMT_RGBA;
		break;
	case VIDEO_PIX_FMT_ABGR:
		vFmt = AV_PIX_FMT_ABGR;
		break;
	case VIDEO_PIX_FMT_BGRA:
		vFmt = AV_PIX_FMT_BGRA;
		break;
	default:
		break;
	}

	return vFmt;
}

inline VideoPixformat LocalPixFormat(AVPixelFormat fmt)
{
	VideoPixformat vFmt = VIDEO_PIX_FMT_NONE;
	switch (fmt)
	{
	case AV_PIX_FMT_YUV420P:
		vFmt = VIDEO_PIX_FMT_YUV420P;
		break;
	case AV_PIX_FMT_YUYV422:
		vFmt = VIDEO_PIX_FMT_YUYV422;
		break;
	case AV_PIX_FMT_YUVJ420P:
		vFmt = VIDEO_PIX_FMT_YUVJ420P;
		break;
	case AV_PIX_FMT_NV12:
		vFmt = VIDEO_PIX_FMT_NV12;
		break;
	case AV_PIX_FMT_NV21:
		vFmt = VIDEO_PIX_FMT_NV21;
		break;
	case AV_PIX_FMT_GRAY8:
		vFmt = VIDEO_PIX_FMT_GRAY8;
		break;
	case AV_PIX_FMT_RGB565:
		vFmt = VIDEO_PIX_FMT_RGB565;
		break;
	case AV_PIX_FMT_BGR565:
		vFmt = VIDEO_PIX_FMT_BGR565;
		break;
	case AV_PIX_FMT_RGB24:
		vFmt = VIDEO_PIX_FMT_RGB24;
		break;
	case AV_PIX_FMT_BGR24:
		vFmt = VIDEO_PIX_FMT_BGR24;
		break;
	case AV_PIX_FMT_ARGB:
		vFmt = VIDEO_PIX_FMT_ARGB;
		break;
	case AV_PIX_FMT_RGBA:
		vFmt = VIDEO_PIX_FMT_RGBA;
		break;
	case AV_PIX_FMT_ABGR:
		vFmt = VIDEO_PIX_FMT_ABGR;
		break;
	case AV_PIX_FMT_BGRA:
		vFmt = VIDEO_PIX_FMT_BGRA;
		break;
	default:
		break;
	}

	return vFmt;
}


#endif //_COMMON_H_