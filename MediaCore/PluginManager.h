#ifndef _PLUGIN_MANAGER_H_
#define _PLUGIN_MANAGER_H_

#include "Plugin.h"
#include <map>
#include <string>

class CPluginManager
{
public:
	static CPluginManager& GetInstance();
	~CPluginManager();

	void LoadPlugins();
	void CleanUpPlugins();
	void Reload();
	void AddPlugin(const std::string &name, CPlugin *plugin);
	IElementFactory* GetElementFactory(const std::string &factoryName);

private:
	CPluginManager();
private:
	static CPluginManager m_Instance;
	std::map<std::string, CPlugin*> m_plugins;

};

#endif  //_PLUGIN_MANAGER_H_