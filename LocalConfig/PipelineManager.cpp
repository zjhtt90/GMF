#include "PipelineManager.h"

#include "../Common/Util.h"
#include "../Common/LogManager.h"

#include "tinyxml2.h"

#define LOG_FILTER	"PipelineManager"


PipelineManager::PipelineManager()
{
}


PipelineManager::~PipelineManager()
{

}

void PipelineManager::Init(int logtype, int loglevel, const char *logfile)
{
	media_core_init(logtype, loglevel, logfile);
}

void PipelineManager::UnInit()
{
	media_core_uninit();
}

void PipelineManager::PreparePipeline(const std::string &name)
{
	if(name == "all")
	{
		for(std::map<std::string,CMediaElement*>::const_iterator ite = m_pipelines.cbegin();
			ite != m_pipelines.cend(); ite++)
		{
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_READY);
		}
	}
	else
	{
		std::map<std::string,CMediaElement*>::iterator ite = m_pipelines.find(name);
		if(ite != m_pipelines.end())
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_READY);
	}
}

void PipelineManager::RunPipeline(const std::string &name)
{
	if(name == "all")
	{
		for(std::map<std::string,CMediaElement*>::const_iterator ite = m_pipelines.cbegin();
			ite != m_pipelines.cend(); ite++)
		{
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_OPEN);
		}
	}
	else
	{
		std::map<std::string,CMediaElement*>::iterator ite = m_pipelines.find(name);
		if(ite != m_pipelines.end())
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_OPEN);
	}
}

void PipelineManager::StopPipeline(const std::string &name)
{
	if(name == "all")
	{
		for(std::map<std::string,CMediaElement*>::const_iterator ite = m_pipelines.cbegin();
			ite != m_pipelines.cend(); ite++)
		{
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_STOP);
		}
	}
	else
	{
		std::map<std::string,CMediaElement*>::iterator ite = m_pipelines.find(name);
		if(ite != m_pipelines.end())
			element_set_state(ite->second, MEDIA_ELEMENT_STATE_STOP);
	}
}


std::string PipelineManager::LoadPipeline(const std::string &xmlPath)
{
	CMediaElement *pipeline = NULL;
	std::string pipeName = "";
	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(xmlPath.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement *root = doc.FirstChildElement("Pipeline");
		if(root)
		{
			pipeline = LoadBin(root);

			pipeName = element_get_name(pipeline);
		}
		doc.DeleteChildren();
	}
	doc.Clear();

	if(pipeline != NULL)
	{
		m_pipelines[pipeName] = pipeline;
	}

	return pipeName;
}

CMediaElement* PipelineManager::LoadBin(tinyxml2::XMLNode *binnode)
{
	CMediaElement *bin = NULL;
	CMediaElement *lastele = NULL;
	tinyxml2::XMLElement *ele = NULL;

	if(binnode != NULL)
	{
		ele = binnode->ToElement();
		std::string tagName = ele->Name();
		std::string layout = ele->Attribute("layout");

		MetaData data(META_KEY_ELEMENT_LAYOUT, layout, META_DATA_VAL_TYPE_STRING);

		if(tagName == "Pipeline")
		{
			bin = element_create("Pipeline", ele->Attribute("name"));
			element_set_parame(bin, data);
		}
		else if(tagName == "Bin")
		{
			bin = element_create("Bin", ele->Attribute("name"));
			element_set_parame(bin, data);
		}
		else
		{
			return NULL;
		}

		tinyxml2::XMLNode *node = binnode->FirstChild();
		while(node != NULL)
		{
			ele = node->ToElement();
			CMediaElement *mEle = NULL;
			tagName = ele->Name();
			if(tagName == "Bin")
			{
				mEle = LoadBin(node);
			}
			else
			{
				mEle = LoadElement(node);
			}

			if(mEle != NULL)
			{
				bin_add_element(bin, mEle);
				if(lastele != NULL && layout == ELEMENT_LAYOUT_HORIZONTAL)
				{
					bin_connect_element(bin, lastele, mEle);
				}
				lastele = mEle;
			}

			node = node->NextSibling();
		}

		element_set_state(bin, MEDIA_ELEMENT_STATE_PREPARE);
	}

	return bin;
}

CMediaElement* PipelineManager::LoadElement(tinyxml2::XMLNode *elenode)
{
	CMediaElement *mEle = NULL;

	tinyxml2::XMLElement *ele = elenode->ToElement();
	if(ele != NULL)
	{
		mEle = element_create(ele->Attribute("Factory"), ele->Attribute("name"));

		if(mEle != NULL)
		{
			tinyxml2::XMLElement *subEle = ele->FirstChildElement();
			while(subEle != NULL)
			{
				LoadElementParame(mEle, subEle);
				subEle = subEle->NextSiblingElement();
			}
		}
	}

	return mEle;
}

void PipelineManager::LoadElementParame(CMediaElement *mEle, tinyxml2::XMLElement *paraEle)
{
	if(mEle == NULL || paraEle == NULL)
	{
		return;
	}

	std::string paraTag = paraEle->Name();
	std::string val = "";
	MetaData data;

	if(paraTag == META_KEY_MEDIA)
	{
		data = MetaData(META_KEY_MEDIA, paraEle->Attribute("value"), META_DATA_VAL_TYPE_STRING);
	}
	else if(paraTag == META_KEY_TRUNK_SIZE)
	{
		data = MetaData(META_KEY_TRUNK_SIZE, paraEle->Attribute("value"), META_DATA_VAL_TYPE_INT);
	}
	else if(paraTag == META_KEY_BITRATE_TYPE)
	{
		val = paraEle->Attribute("value");
		std::string metaVal = "";
		if(val == "cbr")
		{
			metaVal = CUtil::convert<std::string, int>(BitrateControl::CBR);
		}
		else if(val == "vbr")
		{
			metaVal = CUtil::convert<std::string, int>(BitrateControl::VBR);
		}
		else if(val == "cqp")
		{
			metaVal = CUtil::convert<std::string, int>(BitrateControl::CQP);
		}
		data = MetaData(META_KEY_BITRATE_TYPE, metaVal, META_DATA_VAL_TYPE_INT);
	}
	else if(paraTag == "Bitrate")
	{
		data = MetaData(META_KEY_CURRENT_BITRATE, paraEle->Attribute("value"), META_DATA_VAL_TYPE_INT);
	}
	else if(paraTag == "BitrateRange")
	{
		data = MetaData(META_KEY_MIN_BITRATE, paraEle->Attribute("min"), META_DATA_VAL_TYPE_INT);
		element_set_parame(mEle, data);
		data = MetaData(META_KEY_MAX_BITRATE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
		element_set_parame(mEle, data);

		return;
	}
	else if(paraTag == "QValue")
	{
		data = MetaData(META_KEY_MIN_QVALUE, paraEle->Attribute("min"), META_DATA_VAL_TYPE_INT);
		element_set_parame(mEle, data);
		data = MetaData(META_KEY_MAX_QVALUE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
		element_set_parame(mEle, data);

		return;
	}
	else if(paraTag == META_KEY_CODEC_PRESET)
	{
		val = paraEle->Attribute("value");
		std::string metaVal = "";
		if(val == "fast")
		{
			metaVal = CUtil::convert<std::string, int>(Fast);
		}
		else if(val == "medium")
		{
			metaVal = CUtil::convert<std::string, int>(Medium);
		}
		else if(val == "ultrafast")
		{
			metaVal = CUtil::convert<std::string, int>(UltraFast);
		}
		data = MetaData(META_KEY_CODEC_PRESET, metaVal, META_DATA_VAL_TYPE_INT);
	}
	else if(paraTag == META_KEY_CODEC_PROFILE)
	{
		val = paraEle->Attribute("value");
		std::string metaVal = "";
		if(val == "baseline")
		{
			metaVal = CUtil::convert<std::string, int>(BaselineProfile);
		}
		else if(val == "main")
		{
			metaVal = CUtil::convert<std::string, int>(MainProfile);
		}
		else if(val == "high")
		{
			metaVal = CUtil::convert<std::string, int>(HighProfile);
		}
		data = MetaData(META_KEY_CODEC_PROFILE, metaVal, META_DATA_VAL_TYPE_INT);
	}
	else if(paraTag == META_KEY_INPORT_COUNT || paraTag == META_KEY_OUTPORT_COUNT)
	{
		data = MetaData(paraTag, paraEle->Attribute("value"), META_DATA_VAL_TYPE_INT);
		element_use_parame(mEle, data);
		return;
	}
	else
	{
		data = MetaData(paraTag, paraEle->Attribute("value"), META_DATA_VAL_TYPE_INT);
	}

	element_set_parame(mEle, data);
}
