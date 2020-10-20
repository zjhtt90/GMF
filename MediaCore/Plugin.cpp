#include "Plugin.h"

#include "../Common/ModuleLoader.h"
#include "../Common/LogManager.h"
#include "PluginManager.h"

#include <sys/stat.h>

CPlugin::CPlugin()
	: m_fileName(""), m_baseName(""), m_fileSize(0)
{

}

CPlugin::~CPlugin()
{
	if(!m_elements.empty())
	{
		m_desc.plugin_uinit(this);
	}
}

bool CPlugin::Init(const std::string &filePath)
{
	bool res = true;
	m_fileName = filePath;
	int pos = 0;
#ifdef _WIN32
	if((pos = filePath.find_last_of("\\\\")) != std::string::npos)
#else
	if((pos = filePath.find_last_of('/')) != std::string::npos)
#endif
	{
		m_baseName = filePath.substr(pos+1);
	}

	struct stat buf;
	if(stat(filePath.c_str(), &buf) != 0)
	{
		LOG_ERR("%s is not exist!", filePath.c_str());
		return false;
	}
	else
	{
		m_fileSize = buf.st_size;
	}

	void *ptr = NULL;
	if(CModuleLoader::Load(filePath))
	{
		if(CModuleLoader::GetSymbol("g_plugin_desc", &ptr) == 0 && ptr != NULL)
		{
			m_desc = *(PluginDesc*)ptr;
			if(m_desc.plugin_init != NULL)
			{
				m_desc.plugin_init(this);
			}
		}
		else
		{
			res = false;
		}

		//CModuleLoader::UnLoad();
	}
	else
	{
		res = false;
	}

	return res;
}

void CPlugin::UnInit()
{
	m_desc.plugin_uinit(this);
	m_elements.clear();

	CModuleLoader::UnLoad();
}

std::string CPlugin::GetName() const
{
	return std::string(m_desc.name);
}

void CPlugin::AddElement(const std::string &name, IElementFactory *factory)
{
	m_elements[name] = factory;
}

void CPlugin::RemoveElement(const std::string &name)
{
	IElementFactory *factory = GetElementFactory(name);
	if(factory != NULL)
	{
		m_elements.erase(name);

		delete factory;
	}
}

IElementFactory* CPlugin::GetElementFactory(const std::string &factoryName)
{
	std::map<std::string, IElementFactory*>::iterator ite = m_elements.find(factoryName);

	if(ite != m_elements.end())
	{
		return (*ite).second;
	}

	return NULL;
}

const PluginDesc& CPlugin::GetDesc() const
{
	return m_desc;
}

void CPlugin::SetDesc(const PluginDesc &desc)
{
	m_desc = desc;
}


bool CPlugin::RegistStatic(const std::string &name, const std::string &desc, PluginInitFunc func1, PluginUnInitFunc func2)
{
	if(name == "" || func1 == NULL || func2 == NULL)
		return false;
	PluginDesc pdesc = {name.c_str(), desc.c_str(), func1, func2};
	CPlugin *plugin = new CPlugin();
	plugin->SetDesc(pdesc);
	pdesc.plugin_init(plugin);
	CPluginManager::GetInstance().AddPlugin(name, plugin);
}