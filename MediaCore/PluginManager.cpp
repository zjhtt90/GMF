#include "PluginManager.h"

#include "../config.h"

#include "tinyxml2.h"

#include "LogManager.h"

#define LOG_FILTER	"PluginManager"

PLUGIN_STATIC_DECLARE(base);

CPluginManager CPluginManager::m_Instance;

CPluginManager::CPluginManager()
{
}


CPluginManager::~CPluginManager()
{
}

CPluginManager& CPluginManager::GetInstance()
{
	return m_Instance;
}


void CPluginManager::LoadPlugins()
{
	PLUGIN_STATIC_REGISTER(base);

	std::string confPath = PLUGIN_CONFIG_FILE;

	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(confPath.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement *root = doc.FirstChildElement("Plugin");
		if(root)
		{
			tinyxml2::XMLElement *item = root->FirstChildElement("item");
			while(item != NULL)
			{
				std::string libPath = "";
				const char* pluginName = item->Attribute("name");
				const char* temp = item->Attribute("libpath");
				if (temp != NULL)
				{
					libPath = temp;
				}
				else
				{
					temp = item->Attribute("libname");
					if (temp == NULL)
					{
						LOG_WARN("invalid plugin item %s", pluginName);
						break;
					}
					else
					{
						libPath = DEFAULT_PLUGIN_DIR + std::string(temp);
					}
				}
				LOG_DEBUG("read plugin item %s, path:%s", pluginName, libPath.c_str());

				CPlugin *plugin = new CPlugin();
				plugin->Init(libPath);
				AddPlugin(pluginName, plugin);
				
				item = item->NextSiblingElement("item");
			}

		}
		doc.DeleteChildren();
	}
	doc.Clear();
}

void CPluginManager::CleanUpPlugins()
{
	std::map<std::string, CPlugin*>::iterator ite = m_plugins.begin();
	for(; ite != m_plugins.end(); ite++)
	{
		CPlugin *plugin = (*ite).second;
		plugin->UnInit();
		delete plugin;
	}
	m_plugins.clear();
}

void CPluginManager::Reload()
{

}

void CPluginManager::AddPlugin(const std::string &name, CPlugin *plugin)
{
	m_plugins[name] = plugin;
}

IElementFactory* CPluginManager::GetElementFactory(const std::string &factoryName)
{
	IElementFactory *factory = NULL;
	std::map<std::string, CPlugin*>::iterator ite = m_plugins.begin();
	for(; ite != m_plugins.end(); ite++)
	{
		CPlugin *plugin = (*ite).second;
		if((factory = plugin->GetElementFactory(factoryName)) != NULL)
		{
			return factory;
		}
	}

	return NULL;
}