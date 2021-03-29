#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "../config.h"

#include "ModuleLoader.h"

#include <map>

class IElementFactory;
class CPlugin;

typedef int (*PluginInitFunc)(CPlugin *plugin);
typedef void (*PluginUnInitFunc)(CPlugin *plugin);

typedef struct _PluginDesc 
{
	const char *name;
	const char *description;
	PluginInitFunc plugin_init;
	PluginUnInitFunc plugin_uinit;
}PluginDesc;

class API_EXPORT CPlugin
{
public:
	CPlugin();
	~CPlugin();
	bool Init(const std::string &filePath);
	void UnInit();
	std::string GetName() const;
	void AddElement(const std::string &name, IElementFactory *factory);
	void RemoveElement(const std::string &name);
	IElementFactory* GetElementFactory(const std::string &factoryName);
	void SetDesc(const PluginDesc &desc);
	static bool RegistStatic(const std::string&, const std::string&, PluginInitFunc, PluginUnInitFunc);

private:
	const PluginDesc& GetDesc() const;
private:
	std::string m_fileName;  //plugin full path
	std::string m_baseName;  // base name (non-dir part) of plugin path
	long m_fileSize;
	CModuleLoader m_loader;
	PluginDesc m_desc;

	std::map<std::string, IElementFactory*> m_elements;
};


#define PLUGIN_STATIC_DECLARE(name) \
	extern void PASTESTR(plugin_register_, name) (void)

#define PLUGIN_STATIC_REGISTER(name) PASTESTR(plugin_register_, name) ()

#define PLUGIN_DEFINE_STASIC(name,description,init,uinit)	\
	API_EXPORT void PASTESTR(plugin_register_, name) (); \
	void PASTESTR(plugin_register_, name) (void) \
	{ \
		CPlugin::RegistStatic(#name, description, init, uinit); \
	}


#define PLUGIN_DEFINE(name,description,init,uinit)	\
	extern "C" API_EXPORT PluginDesc g_plugin_desc = {	\
	#name,                       \
	description,				\
	init,						\
	uinit						\
}; 



#endif  //_PLUGIN_H_