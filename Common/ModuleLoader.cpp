#include "ModuleLoader.h"

#include "LogManager.h"

#define LOG_FILTER	"ModuleLoader"

#ifdef _WIN32
HINSTANCE CModuleLoader::m_hInstLib = NULL;

bool CModuleLoader::Load(const std::string &libFile)
{
	m_hInstLib = LoadLibrary(libFile.c_str());
	if(m_hInstLib == NULL)
	{
		LOG_ERR("Open library error(%d)!", GetLastError());
		return false;
	}

	return true;
}

bool CModuleLoader::Load(const std::string &libFile, int flag = 0)
{
	m_hInstLib = LoadLibraryEx(libFile.c_str(), NULL, flag);
	if(m_hInstLib == NULL)
	{
		LOG_ERR("Open library error(%d)!", GetLastError());
		return false;
	}

	return true;
}

int CModuleLoader::GetSymbol(const std::string &sym, void **ptr)
{
	*ptr = GetProcAddress(m_hInstLib, sym.c_str());
	if(*ptr == NULL)
	{
		LOG_ERR("Can not load symbol %s", sym.c_str());
		return -1;
	}

	return 0;
}

void CModuleLoader::UnLoad()
{
	FreeLibrary(m_hInstLib);
	m_hInstLib = NULL;
}

#else
void* CModuleLoader::m_hInstLib = NULL;

bool CModuleLoader::Load(const std::string &libFile)
{
	return Load(libFile, RTLD_NOW);
}

bool CModuleLoader::Load(const std::string &libFile, int flag = 0)
{
	m_hInstLib = dlopen(libFile.c_str(), flag);
	if(m_hInstLib == NULL)
	{
		LOG_ERR("Open library error!");
		return false;
	}

	return true;
}

int CModuleLoader::GetSymbol(const std::string &sym, void **ptr)
{
	*ptr = dlsym(m_hInstLib, sym.c_str());
	if(*ptr == NULL)
	{
		LOG_ERR("Can not load symbol %s", sym.c_str());
		return -1;
	}

	return 0;
}

void CModuleLoader::UnLoad()
{
	dlclose(m_hInstLib);
	m_hInstLib = NULL;
}
#endif