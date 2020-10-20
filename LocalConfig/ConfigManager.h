#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include "ConfigSetting.h"

#include <list>

#define DEFAULT_CONFIG_DIR		"config/"
#define	DEFAULT_CONFIG_FILE		DEFAULT_CONFIG_DIR"MediaGraph.xml"
#define DEFAULT_MODEL_DIR		DEFAULT_CONFIG_DIR"share/"
#define MODEL_TMP_DIR			DEFAULT_CONFIG_DIR"tmp/"

class API_EXPORT ConfigMamager
{
public:
	static ConfigMamager& GetInstance();
	~ConfigMamager();

	int Load(const std::string &confFile, std::string &outPipeline);
	int Load(std::list<BaseSettingPtr> &settingList, const std::string &modelName, const std::string &strIndex, std::string &outPipeline);
	BaseSetting* GetLogSetting();
private:
	ConfigMamager();

	void BuildTmpModelFile(const char *srcFile, const char *dstFile);

	void ResetSettingList();

private:
	static ConfigMamager m_instance;
	std::list<BaseSettingPtr> m_settingList;
};


#endif  //_CONFIG_MANAGER_H_