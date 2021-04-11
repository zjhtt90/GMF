#include "Media.h"
#include "MediaElement.h"
#include "PluginManager.h"
#include "LogManager.h"

#include "MediaBin.h"
#include "MediaPipeline.h"

#include <iostream>


void media_core_init(int logtype, int loglevel, const char *logpath)
{
	CLogManager::setLoggerInfo((CLogger::LoggerType)logtype, (CLogger::Level)loglevel);
	if(logpath != NULL)
		CLogManager::setLogPath(logpath);
	CLogManager::startup();

	CPluginManager::GetInstance().LoadPlugins();
}

void media_core_uninit()
{
	CPluginManager::GetInstance().CleanUpPlugins();

	CLogManager::cleanup();
}

CMediaElement* element_create(const std::string &factoryName, const std::string &name)
{
	CMediaElement *element = NULL;
	IElementFactory *factory = CPluginManager::GetInstance().GetElementFactory(factoryName);
	if(factory != NULL)
	{
		if(name.empty())
			element = factory->CreateElement();
		else
			element = factory->CreateElement(name);
		if(element != NULL)
		{
			//std::cout << element << std::endl;
		}
	}

	return element;
}

std::string element_get_name(CMediaElement* element)
{
	if(element != NULL)
	{
		return element->GetName();
	}

	return "";
}

void element_destory(CMediaElement* element)
{
	if(element != NULL)
	{
		delete element;
	}
}

bool element_connect(CMediaElement *up, unsigned int outPortIdx, CMediaElement *down, unsigned int inPortIdx)
{
	CMediaPort *outPort = up->GetMediaPortWithIndex(PORT_DIR_OUT, outPortIdx);
	CMediaPort *inPort = down->GetMediaPortWithIndex(PORT_DIR_IN, inPortIdx);

	if(outPort == NULL || inPort == NULL)
	{
		return false;
	}

	if(outPort->LinkPeerPort(inPort) != MEDIA_ERR_NONE)
	{
		return false;
	}

	return true;
}

void element_disconnect(CMediaElement *up, unsigned int outPortIdx)
{
	CMediaPort *outPort = up->GetMediaPortWithIndex(PORT_DIR_OUT, outPortIdx);

	if(outPort == NULL)
	{
		return;
	}

	if(outPort->IsLinked())
		outPort->UnLinkPeerPort();
}

int element_get_parame(CMediaElement *element, MetaData &param)
{
	return element->GetPrivateData(param);
}

void element_set_parame(CMediaElement *element, const MetaData &param)
{
	element->SetPrivateData(param);
}

void element_use_parame(CMediaElement *element, const MetaData &param)
{
	element->UseParame(param);
}

void element_set_inport_parame(CMediaElement *element, unsigned int inPortIdx, const MetaData &param)
{
	element->SetPortExternData(PORT_DIR_IN, inPortIdx, param);
}

void element_set_outport_parame(CMediaElement *element, unsigned int outPortIdx, const MetaData &param)
{
	element->SetPortExternData(PORT_DIR_OUT, outPortIdx, param);
}

void element_set_state(CMediaElement *element, MediaElementState state)
{
	element->SetState(state);
}

MediaElementState element_get_state(CMediaElement *element)
{
	return element->GetCurrentState();
}


void bin_add_element(CMediaElement *bin,  CMediaElement *element)
{
	static_cast<CMediaBin*>(bin)->AddElment(element);
}

void bin_remove_element(CMediaElement *bin, CMediaElement *element)
{
	static_cast<CMediaBin*>(bin)->RemoveElement(element);
}

bool bin_connect_element(CMediaElement *bin, CMediaElement *up, CMediaElement *down)
{
	return static_cast<CMediaBin*>(bin)->LinkElement(up, down);
}

void bin_disconnect_element(CMediaElement *bin, CMediaElement *up, CMediaElement *down)
{
	static_cast<CMediaBin*>(bin)->UnLinkElement(up, down);
}


void pipeline_set_watcher(CMediaElement *pipeline,  MessageWatcher *watcher)
{
	static_cast<CMediaPipeline*>(pipeline)->SetMessageWatcher(watcher);
}

void pipeline_add_element(CMediaElement *pipeline,  CMediaElement *element)
{
	static_cast<CMediaPipeline*>(pipeline)->AddElment(element);
}

void pipeline_remove_element(CMediaElement *pipeline, CMediaElement *element)
{
	static_cast<CMediaPipeline*>(pipeline)->RemoveElement(element);
}

bool pipeline_connect_element(CMediaElement *pipeline, CMediaElement *up, CMediaElement *down)
{
	return static_cast<CMediaPipeline*>(pipeline)->LinkElement(up, down);
}

void pipeline_disconnect_element(CMediaElement *pipeline, CMediaElement *up, CMediaElement *down)
{
	static_cast<CMediaPipeline*>(pipeline)->UnLinkElement(up, down);
}
