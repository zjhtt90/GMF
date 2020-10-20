#ifndef _CONFIG_SETTING_H_
#define _CONFIG_SETTING_H_

#include "../Common/LogManager.h"
#include <memory>


#define DEFAULT_LOG_DIR	"./OutputLog"


enum ConfSettingType
{
	CONF_SETTING_NONE = -1,
	CONF_SETTING_LOG = 0, //日志输出配置
	CONF_SETTING_LOGIN, //系统登录配置
	CONF_SETTING_SRC, //source组件配置
	CONF_SETTING_FILTER, //filter组件配置
	CONF_SETTING_SINK //sink组件配置
};

struct BaseSetting
{
	BaseSetting(ConfSettingType type)
	{
		mType = type;
	}

	ConfSettingType mType;
};

typedef std::shared_ptr<BaseSetting> BaseSettingPtr;

struct LogConfSetting : public BaseSetting
{
	LogConfSetting() : BaseSetting(CONF_SETTING_LOG), 
		mOutType(-1), mbShowColor(false), mOutLevel(CLogger::kWarning), mOutDir(DEFAULT_LOG_DIR)
	{

	}

	int mOutType;
	bool mbShowColor;
	int mOutLevel;
	std::string mOutDir;
};


struct MediaSetting : public BaseSetting
{
	MediaSetting(ConfSettingType type) : BaseSetting(type)
	{

	}

	MediaSetting(ConfSettingType type, const std::string &owner) : BaseSetting(type), mOwnerEle(owner)
	{

	}

	std::string mOwnerEle;
};


struct SrcSetting : public MediaSetting
{
	SrcSetting() : MediaSetting(CONF_SETTING_SRC)
	{

	}

	SrcSetting(const std::string &owner) : MediaSetting(CONF_SETTING_SRC, owner)
	{

	}

	std::string mURI;

	int mTrunkSize;

#ifdef _WIN32
	HWND mWin;
#else
	void *mWin;
#endif
	int mFormat;

	int mWidth;
	int mHeight;
	int mFps;

	int mSampleRate;
	int mChannel;
};


struct FilterSetting : public MediaSetting
{
	FilterSetting() : MediaSetting(CONF_SETTING_FILTER)
	{

	}

	FilterSetting(const std::string &owner) : MediaSetting(CONF_SETTING_FILTER, owner)
	{

	}

	std::string mFilterDesc;

	int mFormat;

	int mWidth;
	int mHeight;
	int mFps;

	int mBitRate;
	int mMinBitRate;
	int mMaxBitRate;
	int mQMin;
	int mQMax;
	int mISpace;
	std::string mPreset;
	std::string mCtrl;
	int mQuality;
	std::string mProfile;


	int mSampleRate;
	int mChannel;
	int mSampleCount;
};




struct SinkSetting : public MediaSetting
{
	SinkSetting() : MediaSetting(CONF_SETTING_SINK)
	{

	}

	SinkSetting(const std::string &owner) : MediaSetting(CONF_SETTING_SINK, owner)
	{

	}

	int mInportCount;

	std::string mURI;

	int mFormat;

	int mWidth;
	int mHeight;

	int mSampleRate;
	int mChannel;
};


#endif  //_CONFIG_SETTING_H_