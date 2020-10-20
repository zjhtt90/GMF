#include "ConfigManager.h"

#include "../Common/LogManager.h"
#include "../Common/Util.h"

#include "tinyxml2.h"

#include "PipelineXMLHelper.h"

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define LOG_FILTER	"ConfigMamager"

static BaseSetting* LoadLogSetting(tinyxml2::XMLElement *logNode);
static BaseSetting* LoadSrcSetting(tinyxml2::XMLElement *node);
static BaseSetting* LoadFilterSetting(tinyxml2::XMLElement *node);
static BaseSetting* LoadSinkSetting(tinyxml2::XMLElement *node);



ConfigMamager ConfigMamager::m_instance;

ConfigMamager::ConfigMamager()
{
}


ConfigMamager::~ConfigMamager()
{
}

ConfigMamager& ConfigMamager::GetInstance()
{
	return m_instance;
}

int ConfigMamager::Load(const std::string &confFile, std::string &outPipeline)
{
	int res = 0;
	tinyxml2::XMLDocument doc;
	std::list<BaseSettingPtr> settingList;

	if((res = doc.LoadFile(confFile.c_str())) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement *root = doc.FirstChildElement("MediaGraph");
		if(root)
		{
			tinyxml2::XMLElement *ele = root->FirstChildElement("Log");
			std::string tagName;
			while(ele != NULL)
			{
				tagName = ele->Name();
				if(tagName == "Log")
				{
					settingList.push_back(BaseSettingPtr(LoadLogSetting(ele)));
				}
				else if(tagName == "MediaElementParam")
				{
					switch(ele->IntAttribute("type"))
					{
					case CONF_SETTING_SRC:
						settingList.push_back(BaseSettingPtr(LoadSrcSetting(ele)));
						break;
					case CONF_SETTING_FILTER:
						settingList.push_back(BaseSettingPtr(LoadFilterSetting(ele)));
						break;
					case CONF_SETTING_SINK:
						settingList.push_back(BaseSettingPtr(LoadSinkSetting(ele)));
					default:
						break;
					}
				}
				ele = ele->NextSiblingElement();
			}

			ele = root->FirstChildElement("PipelineSelector");
			if(ele != NULL)
			{
				ele = ele->FirstChildElement("model");
				const char *val = ele->Attribute("value");
				const char *idx = (ele->Attribute("index") != NULL ? ele->Attribute("index") : "0");
				if(val != NULL)
				{
					res = Load(settingList, val, idx, outPipeline);
				}
			}
		}
	}

	return res;
}

int ConfigMamager::Load(std::list<BaseSettingPtr> &settingList, const std::string &modelName, const std::string &strIndex, std::string &outPipeline)
{
	ResetSettingList();

	m_settingList = settingList;

	char pipelineModelFile[128] = {0};
	char tmpModelFile[128] = {0};
	snprintf(pipelineModelFile, 128, "%spipeline_%s.xml", DEFAULT_MODEL_DIR, modelName.c_str());
	snprintf(tmpModelFile, 128, "%spipeline_%s_%s.xml", MODEL_TMP_DIR, modelName.c_str(), strIndex.c_str());

#ifdef _WIN32
	if(_access(pipelineModelFile, 0) != -1)
#else
	if(access(pipelineModelFile, 0) == 0)
#endif
	{
#ifdef _WIN32
		_mkdir(MODEL_TMP_DIR);
#else
		mkdir(MODEL_TMP_DIR, 666);
#endif
		BuildTmpModelFile(pipelineModelFile, tmpModelFile);

		outPipeline = tmpModelFile;
		return 0;
	}

	return -1;
}

BaseSetting* ConfigMamager::GetLogSetting()
{
	BaseSetting *ptr = NULL;
	std::list<BaseSettingPtr>::iterator ite;
	for(ite = m_settingList.begin(); ite != m_settingList.end(); ite++)
	{
		if((*ite)->mType == CONF_SETTING_LOG)
		{
			ptr = (*ite).get();
		}
	}

	return ptr;
}

void ConfigMamager::BuildTmpModelFile(const char *srcFile, const char *dstFile)
{
	PipelineXMLHelper helper;
	if(helper.Open(srcFile) == 0)
	{
		helper.WriteSettingsToXml(&m_settingList);

		helper.SaveToFile(dstFile);

		helper.Close();
	}
}


static BaseSetting* LoadLogSetting(tinyxml2::XMLElement *logNode)
{
	LogConfSetting *confset = NULL;
	if(logNode != NULL)
	{
		confset = new LogConfSetting();

		tinyxml2::XMLElement *ele = NULL;
		ele = logNode->FirstChildElement();
		std::string tagName;
		while(ele != NULL)
		{
			tagName = ele->Name();
			std::string eleVal = ele->Attribute("value");
			if(tagName == "Consolelog")
			{
				if(eleVal == "1")
					confset->mOutType = CLogger::LOG_COUT;
			}
			else if(tagName == "Filelog")
			{
				if(eleVal == "1")
				{
					if(confset->mOutType == CLogger::LOG_COUT)
						confset->mOutType = CLogger::LOG_C_F;
					else
						confset->mOutType = CLogger::LOG_FILE;
				}
			}
			else if(tagName == "Color")
			{
				if(eleVal == "1")
					confset->mbShowColor = true;
			}
			else if(tagName == "LogLevel")
			{
				if(eleVal == "debug")
					confset->mOutLevel = CLogger::kDebug;
				else if(eleVal == "info")
					confset->mOutLevel = CLogger::kInfo;
				else if(eleVal == "warn")
					confset->mOutLevel = CLogger::kWarning;
				else if(eleVal == "error")
					confset->mOutLevel = CLogger::kError;
			}
			else if(tagName == "LogSavePath")
			{
				confset->mOutDir = eleVal;
			}

			ele = ele->NextSiblingElement();
		}
	}

	return confset;
}

static BaseSetting* LoadSrcSetting(tinyxml2::XMLElement *node)
{
	SrcSetting *setting = NULL;
	std::string owner = node->Attribute("owner");
	if(node != NULL)
	{
		setting = new SrcSetting(owner);
		tinyxml2::XMLElement *ele = NULL;
		ele = node->FirstChildElement();
		std::string tagName;
		while(ele != NULL)
		{
			tagName = ele->Name();
			if(tagName == "FilePath")
			{
				setting->mURI = ele->Attribute("value");
			}
			else if(tagName == "TrunkSize")
			{
				setting->mTrunkSize = ele->IntAttribute("value");
			}
			else if(tagName == "VideoWidth")
			{
				setting->mWidth = ele->IntAttribute("value");
			}
			else if(tagName == "VideoHeight")
			{
				setting->mHeight = ele->IntAttribute("value");
			}
			else if(tagName == "ImgFormat" || tagName == "SampleFormat")
			{
				setting->mFormat = ele->IntAttribute("value");
			}
			else if(tagName == "VideoFPS")
			{
				setting->mFps = ele->IntAttribute("value");
			}
			else if(tagName == "SampleRate")
			{
				setting->mSampleRate = ele->IntAttribute("value");
			}
			else if(tagName == "AudioChannel")
			{
				setting->mChannel = ele->IntAttribute("value");
			}
			

			ele = ele->NextSiblingElement();
		}
	}

	return setting;
}

static BaseSetting* LoadFilterSetting(tinyxml2::XMLElement *node)
{
	FilterSetting *setting = NULL;
	std::string owner = node->Attribute("owner");
	if(node != NULL)
	{
		setting = new FilterSetting(owner);
		tinyxml2::XMLElement *ele = NULL;
		ele = node->FirstChildElement();
		std::string tagName;
		while(ele != NULL)
		{
			tagName = ele->Name();
			
			if(tagName == "FilterDescription")
			{
				setting->mFilterDesc = ele->Attribute("value");
			}
			else if(tagName == "VideoWidth")
			{
				setting->mWidth = ele->IntAttribute("value");
			}
			else if(tagName == "VideoHeight")
			{
				setting->mHeight = ele->IntAttribute("value");
			}
			else if(tagName == "ImgFormat" || tagName == "SampleFormat")
			{
				setting->mFormat = ele->IntAttribute("value");
			}
			else if(tagName == "VideoFPS")
			{
				setting->mFps = ele->IntAttribute("value");
			}
			else if(tagName == "SampleRate")
			{
				setting->mSampleRate = ele->IntAttribute("value");
			}
			else if(tagName == "AudioChannel")
			{
				setting->mChannel = ele->IntAttribute("value");
			}
			else if(tagName == "FrameSampleCount")
			{
				setting->mSampleCount = ele->IntAttribute("value");
			}
			else if(tagName == "Bitrate")
			{
				setting->mBitRate = ele->IntAttribute("value");
			}
			else if(tagName == "BitrateRange")
			{
				setting->mMinBitRate = ele->IntAttribute("min");
				setting->mMaxBitRate = ele->IntAttribute("max");
			}
			else if(tagName == "QValue")
			{
				setting->mQMin = ele->IntAttribute("min");
				setting->mQMax = ele->IntAttribute("max");
			}
			else if(tagName == "IFrameInterval")
			{
				setting->mISpace = ele->IntAttribute("value");
			}
			else if(tagName == "CodecPreset")
			{
				setting->mPreset = ele->Attribute("value");
			}
			else if(tagName == "CodecControl")
			{
				setting->mCtrl = ele->Attribute("value");
			}
			else if(tagName == "CodecQuality")
			{
				setting->mQuality = ele->IntAttribute("value");
			}
			else if(tagName == "CodecProfile")
			{
				setting->mProfile = ele->Attribute("value");
			}

			ele = ele->NextSiblingElement();
		}
	}

	return setting;
}

static BaseSetting* LoadSinkSetting(tinyxml2::XMLElement *node)
{
	SinkSetting *setting = NULL;
	std::string owner = node->Attribute("owner");
	if(node != NULL)
	{
		setting = new SinkSetting(owner);
		tinyxml2::XMLElement *ele = NULL;
		ele = node->FirstChildElement();
		std::string tagName;
		while(ele != NULL)
		{
			tagName = ele->Name();

			if(tagName == "InPortCount")
			{
				setting->mInportCount = ele->IntAttribute("value");
			}
			else if(tagName == "FilePath")
			{
				setting->mURI = ele->Attribute("value");
			}
			else if(tagName == "VideoWidth")
			{
				setting->mWidth = ele->IntAttribute("value");
			}
			else if(tagName == "VideoHeight")
			{
				setting->mHeight = ele->IntAttribute("value");
			}
			else if(tagName == "ImgFormat" || tagName == "SampleFormat")
			{
				setting->mFormat = ele->IntAttribute("value");
			}
			else if(tagName == "SampleRate")
			{
				setting->mSampleRate = ele->IntAttribute("value");
			}
			else if(tagName == "AudioChannel")
			{
				setting->mChannel = ele->IntAttribute("value");
			}

			ele = ele->NextSiblingElement();
		}
	}

	return setting;
}


void ConfigMamager::ResetSettingList()
{
	m_settingList.clear();
}