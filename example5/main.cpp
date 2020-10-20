#include <stdio.h>
#include <stdlib.h>

#include "../MediaCore/Media.h"
#include "../Common/Util.h"
#include "../Common/LogManager.h"
#include "../Common/tinyxml2.h"

#include <windows.h>

#include <signal.h>


#define LOG_FILTER	"Main"


void load_element_parame(CMediaElement *mEle, tinyxml2::XMLElement *paraEle, const std::string &portDir, int portIdx)
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
		if(portDir == "in")
		{
			element_set_inport_parame(mEle, portIdx, data);
			data = MetaData(META_KEY_MAX_BITRATE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_inport_parame(mEle, portIdx, data);
		}
		else if(portDir == "out")
		{
			element_set_outport_parame(mEle, portIdx, data);
			data = MetaData(META_KEY_MAX_BITRATE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_outport_parame(mEle, portIdx, data);
		}
		else
		{
			element_set_parame(mEle, data);
			data = MetaData(META_KEY_MAX_BITRATE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_parame(mEle, data);
		}
		return;
	}
	else if(paraTag == "QValue")
	{
		data = MetaData(META_KEY_MIN_QVALUE, paraEle->Attribute("min"), META_DATA_VAL_TYPE_INT);
		if(portDir == "in")
		{
			element_set_inport_parame(mEle, portIdx, data);
			data = MetaData(META_KEY_MAX_QVALUE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_inport_parame(mEle, portIdx, data);
		}
		else if(portDir == "out")
		{
			element_set_outport_parame(mEle, portIdx, data);
			data = MetaData(META_KEY_MAX_QVALUE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_outport_parame(mEle, portIdx, data);
		}
		else
		{
			element_set_parame(mEle, data);
			data = MetaData(META_KEY_MAX_QVALUE, paraEle->Attribute("max"), META_DATA_VAL_TYPE_INT);
			element_set_parame(mEle, data);
		}

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
	else if(paraTag == META_KEY_FILTE_DESC)
	{
		data = MetaData(paraTag, paraEle->Attribute("value"), META_DATA_VAL_TYPE_STRING);
		element_set_parame(mEle, data);
		return;
	}
	else
	{
		data = MetaData(paraTag, paraEle->Attribute("value"), META_DATA_VAL_TYPE_INT);
	}

	if(portDir == "in")
	{
		element_set_inport_parame(mEle, portIdx, data);
	}
	else if(portDir == "out")
	{
		element_set_outport_parame(mEle, portIdx, data);
	}
	else
	{
		element_set_parame(mEle, data);
	}
}

void load_element_port(CMediaElement *mEle, tinyxml2::XMLElement *paraEle)
{
	std::string portDir = paraEle->Attribute("dir");
	int portIdx = paraEle->IntAttribute("id");

	tinyxml2::XMLElement *subEle = paraEle->FirstChildElement();
	while(subEle != NULL)
	{
		load_element_parame(mEle, subEle, portDir, portIdx);
		subEle = subEle->NextSiblingElement();
	}
}

CMediaElement* load_element(tinyxml2::XMLNode *elenode)
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
				std::string tag = subEle->Name();
				if(tag == "Port")
				{
					load_element_port(mEle, subEle);
				}
				else
				{
					load_element_parame(mEle, subEle, "", -1);
				}
				subEle = subEle->NextSiblingElement();
			}
		}
	}

	return mEle;
}

CMediaElement* load_bin(tinyxml2::XMLNode *binnode)
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
				mEle = load_bin(node);
			}
			else
			{
				mEle = load_element(node);
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


CMediaElement* load_pipeline(const std::string &xmlPath)
{
	CMediaElement *pipeline = NULL;
	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(xmlPath.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement *root = doc.FirstChildElement("Pipeline");
		if(root)
		{
			pipeline = load_bin(root);
		}
		doc.DeleteChildren();
	}
	doc.Clear();

	return pipeline;
}

static bool g_exit = false;



static void sig_handle(int sig)
{
	if(sig == SIGINT)
		g_exit = true;
}

int main(int argc, char **argv)
{
	MetaData data;

	signal(SIGINT, sig_handle);

	media_core_init(0, 0, NULL);
	
	CMediaElement *pipeline = NULL;

	pipeline = load_pipeline("./pipeline.xml");

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_READY);

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_OPEN);

	while(!g_exit)
	{
		Sleep(1000);
	}

	LOG_INFO("Stop pipeline...");

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_STOP);
	LOG_INFO("Stop pipeline+++");
	element_destory(pipeline);

	media_core_uninit();
	return 0;
}