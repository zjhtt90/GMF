#ifndef _PIPELINE_MANAGER_H_
#define _PIPELINE_MANAGER_H_

#include "../MediaCore/Media.h"
#include "tinyxml2.h"
#include <map>


class API_EXPORT PipelineManager
{
public:
	PipelineManager();
	~PipelineManager();

	static void Init(int logtype = 0, int loglevel = 0, const char *logfile = NULL);
	static void UnInit();

	std::string LoadPipeline(const std::string &xmlPath);

	void PreparePipeline(const std::string &name = "all");
	void RunPipeline(const std::string &name = "all");
	void StopPipeline(const std::string &name = "all");

private:
	CMediaElement* LoadBin(tinyxml2::XMLNode *binnode);
	CMediaElement* LoadElement(tinyxml2::XMLNode *elenode);
	void LoadElementParame(CMediaElement *mEle, tinyxml2::XMLElement *paraEle);

private:
	std::map<std::string, CMediaElement*> m_pipelines;
};


#endif  //_PIPELINE_MANAGER_H_