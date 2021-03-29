#ifndef _MEDIA_DEFINES_H_
#define _MEDIA_DEFINES_H_

#include <string>
#include <vector>


enum ElementType {
	ELEMENT_TYPE_NONE = -1,
	ELEMENT_TYPE_SRC = 0,
	ELEMENT_TYPE_FILTER,
	ELEMENT_TYPE_SINK,
	ELEMENT_TYPE_PIPELINE
};


enum MediaErr {
	MEDIA_ERR_NONE = 0,
	MEDIA_ERR_OPEN,
	MEDIA_ERR_INVALIDE_PARAME,
	MEDIA_ERR_MEMALLOC,
	MEDIA_ERR_NOT_FOUND,
	MEDIA_ERR_PORT_NOT_COMPATIBLE,
	MEDIA_ERR_READ_FAILED,
	MEDIA_ERR_END_OF_STREAM,
	MEDIA_ERR_RTP_FAILED,
	MEDIA_ERR_RTP_TIMEOUT
};

enum MediaType {
    MEDIA_TYPE_NONE  = -1,
    MEDIA_TYPE_VIDEO = 0,
    MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_DATA,
	MEDIA_TYPE_SUBTITLE,
	MEDIA_TYPE_ATTACHMENT,
	MEDIA_TYPE_NB
};

inline std::string MediaTypeToStr(MediaType type)
{
	std::string str = "";
	switch(type)
	{
	case MEDIA_TYPE_VIDEO:
		str = "Video";
		break;
	case MEDIA_TYPE_AUDIO:
		str = "Audio";
		break;
	case MEDIA_TYPE_DATA:
		str = "Data";
		break;
	case MEDIA_TYPE_SUBTITLE:
		str = "Subtitle";
		break;
	default:
		str = "Unknow";
		break;
	}

	return str;
}

inline MediaType MediaStrToType(const std::string &media)
{
	MediaType type = MEDIA_TYPE_NONE;
	if(media == "Video")
	{
		type = MEDIA_TYPE_VIDEO;
	}
	else if(media == "Audio")
	{
		type = MEDIA_TYPE_AUDIO;
	}
	else if(media == "Data")
	{
		type = MEDIA_TYPE_DATA;
	}
	else if(media == "Subtitle")
	{
		type = MEDIA_TYPE_SUBTITLE;
	}

	return type;
}


typedef unsigned int TRACKID;

enum MediaElementState {
	MEDIA_ELEMENT_STATE_NONE = -1,
	MEDIA_ELEMENT_STATE_PREPARE = 0,
	MEDIA_ELEMENT_STATE_READY,
	MEDIA_ELEMENT_STATE_OPEN,
	MEDIA_ELEMENT_STATE_OPENING,
	MEDIA_ELEMENT_STATE_OPENED,
	MEDIA_ELEMENT_STATE_RUNNING,
	MEDIA_ELEMENT_STATE_PAUSE,
	MEDIA_ELEMENT_STATE_PAUSING,
	MEDIA_ELEMENT_STATE_PAUSED,
	MEDIA_ELEMENT_STATE_RESUME,
	MEDIA_ELEMENT_STATE_STOP,
	MEDIA_ELEMENT_STATE_STOPPED
};


#define META_KEY_INPORT_COUNT	"InPortCount"
#define META_KEY_OUTPORT_COUNT	"OutPortCount"
#define META_KEY_MEDIA			"Media"
#define META_KEY_FILE_PATH		"FilePath"
#define META_KEY_URI			"URI"
#define META_KEY_TRUNK_SIZE		"TrunkSize"
#define META_KEY_PORT_BASE		"PortBase"
#define META_KEY_SUBTITLE		"Subtitle"

#define META_KEY_CODEC_TYPE		"CodecType"
#define META_KEY_CODEC_ID		"CodecID"

//Audio
#define META_KEY_SAMPLE_RATE	"SampleRate"
#define META_KEY_CHANNEL_NUM	"AudioChannel"
#define META_KEY_SAMPLE_FORMAT	"SampleFormat"
#define META_KEY_FRAME_SAMPLE_COUNT	"FrameSampleCount"

//Video
#define META_KEY_VIDEO_WIDTH	"VideoWidth"
#define META_KEY_VIDEO_HEIGHT	"VideoHeight"
#define META_KEY_PIX_FORMAT		"ImgFormat"
#define META_KEY_VIDEO_FPS		"VideoFPS"
#define META_KEY_VIDEO_WINDOW	"VideoWindow"
#define META_KEY_VIEW_RECT		"ViewRect"
#define META_KEY_WINDOW_X_POS	"WindowXPos"
#define META_KEY_WINDOW_Y_POS	"WindowYPos"

//video overlay
#define META_KEY_VIDEO_X_POS	"VideoXPos"
#define META_KEY_VIDEO_Y_POS	"VideoYPos"

//for codec
/*码率类型*/
#define META_KEY_BITRATE_TYPE		"CodecControl"
/*当前码率*/
#define META_KEY_CURRENT_BITRATE	"CurBitrate"
/*最小码率*/
#define META_KEY_MIN_BITRATE		"MinBitrate"
/*最大码率*/
#define META_KEY_MAX_BITRATE		"MaxBitrate"
/*编码质量*/
#define META_KEY_CODEC_QUALITY		"CodecQuality"
/*编码预设*/
#define META_KEY_CODEC_PRESET		"CodecPreset"
/*编码配置级别*/
#define META_KEY_CODEC_PROFILE		"CodecProfile"
/*编码最大量化值*/
#define META_KEY_MAX_QVALUE			"MaxQ"
/*编码最小量化值*/
#define META_KEY_MIN_QVALUE			"MinQ"
/*I帧间隔*/
#define META_KEY_I_FRAME_INTERVAL	"IFrameInterval"

/*心跳间隔*/
#define META_KEY_HEART_INTERVAL		"HeartBeatInterval"
/*心跳超时*/
#define META_KEY_HEART_TIMEOUT		"HeartBeatTimeout"


#define META_KEY_CODEC_EXTRA_DATA	"CodecExtraData"


#define META_KEY_ELEMENT_LAYOUT	"ElementLayout"

#define ELEMENT_LAYOUT_HORIZONTAL	"Horizon"
#define ELEMENT_LAYOUT_VERTICAL		"Vertical"



/*数据过滤描述*/
#define META_KEY_FILTE_DESC			"FilterDescription"


enum META_DATA_VAL_TYPE {
	META_DATA_VAL_TYPE_UNKNOW = -1,
	META_DATA_VAL_TYPE_STRING = 0,
	META_DATA_VAL_TYPE_INT,
	META_DATA_VAL_TYPE_PTR,
	META_DATA_VAL_TYPE_BOOL,
	META_DATA_VAL_TYPE_FLOAT
};

struct MetaData
{
	std::string mKey;
	std::string	mValue;
	META_DATA_VAL_TYPE mValType;

	MetaData() { }
	MetaData(const std::string &key, const std::string &val, META_DATA_VAL_TYPE valType) :
		mKey(key), mValue(val), mValType(valType)
	{
	}

	bool operator==(const MetaData &other)
	{
		return (mKey == other.mKey && mValType == other.mValType && mValue == other.mValue);
	}
};


enum MediaCodecType
{
	MediaCodec_NONE = 0,
	MediaCodec_MPEG1VIDEO,
	MediaCodec_MPEG2VIDEO,
	MediaCodec_H261,
	MediaCodec_H263,
	MediaCodec_MJPEG,
	MediaCodec_MPEG4,
	MediaCodec_H264,
	MediaCodec_H265,
	MediaCodec_PCM,
	MediaCodec_MULAW,
	MediaCodec_ALAW,
	MediaCodec_ADPCM,
	MediaCodec_AMR_NB,
	MediaCodec_AMR_WB,
	MediaCodec_MP3,
	MediaCodec_AAC,
	MediaCodec_AC3,
	MediaCodec_WMAV1,
	MediaCodec_WMAV2,
	MediaCodec_SPEEX,
	MediaCodec_G723,
	MediaCodec_G729,
	MediaCodec_ILBC,
	MediaCodec_OPUS
};



enum Preset
{
	Fast,
	Medium,
	UltraFast,
	PresetMax
};

enum Profile
{
	BaselineProfile,
	MainProfile,
	HighProfile,
	ProfileMax
};

struct BitrateControl
{
	// 码率类型
	enum BitrateType
	{
		CBR, // 恒定码率
		VBR, // 可变码率
		CQP, // 恒定质量
		BitrateMax
	};

	enum BitrateType type; // 码率控制类型
	int cur_bitrate; // 当前码率(KBit)
	int min_bitrate; // 最小码率(KBit)
	int max_bitrate; // 最大码率(KBit)
	unsigned int quality; // 编码质量

	BitrateControl() : type(CBR), cur_bitrate(0), min_bitrate(0), max_bitrate(0), quality(0)
	{}

	BitrateControl(unsigned int bitrate) :
		type(CBR), cur_bitrate(bitrate), min_bitrate(0), max_bitrate(0), quality(0)
	{}

	BitrateControl(unsigned int bitrate, unsigned int minbitrate, unsigned int maxbitrate) :
		type(VBR), cur_bitrate(bitrate), min_bitrate(minbitrate), max_bitrate(maxbitrate), quality(0)
	{}

	BitrateControl(unsigned int q, int) :
		type(CQP), cur_bitrate(0), min_bitrate(0), max_bitrate(0), quality(q)
	{}

	BitrateControl(const BitrateControl &bc) : type(bc.type), cur_bitrate(bc.cur_bitrate),
		min_bitrate(bc.min_bitrate), max_bitrate(bc.max_bitrate), quality(bc.quality)
	{}
};


typedef struct __VideoRect {
	int x;
	int y;
	int width;
	int height;
}VideoRect;



enum EFrameType
{
	OV_VIDEO_I = 0x00000000,
	OV_AUDIO   = 0x00000001,
	OV_HEADE   = 0x00000080,
	OV_VIDEO_P = 0x00010000,
	OV_VIDEO_B = 0x00020000,
	OV_AAC	   = 0x00120000,
	OV_H265	   = 0x00130000,
};

struct MediaMetaInfo
{
	int mStreamID;
	MediaType mType;
	MediaCodecType mCodecType;
	int mReverse;
	MediaMetaInfo(MediaType type) : mType(type), mCodecType(MediaCodec_NONE), mReverse(-1)
	{

	}
};

struct VMetaInfo : public MediaMetaInfo
{
	int mWidth;
	int mHeight;
	int mPixfmt;
	int mfps;
	VMetaInfo() : MediaMetaInfo(MEDIA_TYPE_VIDEO), mWidth(0),mHeight(0),mPixfmt(0),mfps(0)
	{

	}
	VMetaInfo(int w,int h,int fmt,int fps) 
		: MediaMetaInfo(MEDIA_TYPE_VIDEO), mWidth(w),mHeight(h),mPixfmt(fmt),mfps(fps)
	{

	}
};

struct AMetaInfo : public MediaMetaInfo
{
	int mSamplerate;
	int mChannel;
	int mSamplefmt;
	int mSamplecnt;
	AMetaInfo() : MediaMetaInfo(MEDIA_TYPE_AUDIO),mSamplerate(0),mChannel(0),mSamplefmt(0),mSamplecnt(0)
	{
	}
	AMetaInfo(int samplerate,int ch,int samplefmt,int samplecnt) 
		: MediaMetaInfo(MEDIA_TYPE_AUDIO),mSamplerate(samplerate),mChannel(ch),mSamplefmt(samplefmt),mSamplecnt(samplecnt)
	{
	}
};



typedef enum
{
	MEDIA_MESSAGE_NONE = -1,
	MEDIA_MESSAGE_QUERY,
	MEDIA_MESSAGE_REPLY,
	MEDIA_MESSAGE_NOTIFY
} MediaMessageType;

enum MessageSubType
{
	MESSAGE_SUB_TYPE_NONE = -1,
	//警告消息
	MESSAGE_SUB_TYPE_WARN,
	//错误消息
	MESSAGE_SUB_TYPE_ERROR,
	//状态变化消息
	MESSAGE_SUB_TYPE_CHANGE,
	//源数据流停止
	MESSAGE_SUB_TYPE_END_OF_STREAM,
	//组件所需SDP消息
	MESSAGE_SUB_TYPE_SDP,
	//组件所需媒体编码信息
	MESSAGE_SUB_TYPE_CODEC_INFO,
	//强制编码器产生关键帧
	MESSAGE_SUB_TYPE_FORCE_KEY
};

struct MessageWatcher
{
	virtual void MessageProc(int msgType, const std::vector<MetaData> &msgContent) = 0;
};


#define ALIGN(x, a) (((x)+((a)-1))&~((a)-1)) //a字节对齐
#define ALIGN_BY	4


enum VideoPixformat
{
	VIDEO_PIX_FMT_NONE = -1,
	VIDEO_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
	VIDEO_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
	VIDEO_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range
	VIDEO_PIX_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
	VIDEO_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped
	VIDEO_PIX_FMT_GRAY8, ///<        Y        ,  8bpp
	VIDEO_PIX_FMT_RGB555,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb)
	VIDEO_PIX_FMT_BGR555,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb)
	VIDEO_PIX_FMT_RGB565,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb)
	VIDEO_PIX_FMT_BGR565,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb)
	VIDEO_PIX_FMT_RGB24,  ///< packed RGB 8:8:8, 24bpp, RGBRGB...
	VIDEO_PIX_FMT_BGR24,  ///< packed BGR 8:8:8, 24bpp, BGRBGR...
	VIDEO_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
	VIDEO_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
	VIDEO_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
	VIDEO_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
};


enum AudioSampleFormat {
	AUDIO_SAMPLE_FMT_NONE = -1,
	AUDIO_SAMPLE_FMT_U8,          ///< unsigned 8 bits
	AUDIO_SAMPLE_FMT_S16,         ///< signed 16 bits
	AUDIO_SAMPLE_FMT_S32,         ///< signed 32 bits
	AUDIO_SAMPLE_FMT_FLT,         ///< float
	AUDIO_SAMPLE_FMT_DBL,         ///< double

	AUDIO_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
	AUDIO_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
	AUDIO_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
	AUDIO_SAMPLE_FMT_FLTP,        ///< float, planar
	AUDIO_SAMPLE_FMT_DBLP,        ///< double, planar
	AUDIO_SAMPLE_FMT_S64,         ///< signed 64 bits
	AUDIO_SAMPLE_FMT_S64P,        ///< signed 64 bits, planar

	AUDIO_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

inline int GetBytesPerAudioSample(AudioSampleFormat fmt)
{
	int bytesCount = 0;
	switch (fmt)
	{
	case AUDIO_SAMPLE_FMT_U8:
		bytesCount = 1;
		break;
	case AUDIO_SAMPLE_FMT_S16:
		bytesCount = 2;
		break;
	case AUDIO_SAMPLE_FMT_S32:
		bytesCount = 4;
		break;
	default:
		break;
	}

	return bytesCount;
}


#endif  //_MEDIA_DEFINES_H_