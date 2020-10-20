#ifndef _PIPELINE_XML_HELPER_H_
#define _PIPELINE_XML_HELPER_H_

#include <string>
#include <list>
#include "tinyxml2.h"
#include "ConfigSetting.h"

class PipelineXMLHelper
{
public:
	PipelineXMLHelper();
	~PipelineXMLHelper();


	int Open(const std::string &xmlfile);

	int WriteSettingsToXml(std::list<BaseSettingPtr> *confs);

	int ReadSettingsFromXml(std::list<BaseSettingPtr> *confs);

	int SaveToFile(const std::string &xmlfile);

	void Close();

private:
	BaseSettingPtr FindSettingByOwner(const std::string &owner);
	void WriteElementParam(tinyxml2::XMLElement *ele, BaseSettingPtr &setting);
	BaseSetting* ReadElementParam(tinyxml2::XMLElement *ele);

	void ReadBin(tinyxml2::XMLNode *binnode, bool bOnlyRead = true);
	void ReadElement(tinyxml2::XMLNode *elenode, bool bOnlyRead = true);

private:
	std::string m_xmlFile;

	tinyxml2::XMLDocument m_doc;
	tinyxml2::XMLElement *m_root;
	std::list<BaseSettingPtr> *m_pSettingList;
};


#endif  //_PIPELINE_XML_HELPER_H_