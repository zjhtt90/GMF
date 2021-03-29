#ifndef _MODULE_LOADER_H_
#define _MODULE_LOADER_H_

#include "../config.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <string>

class API_EXPORT CModuleLoader
{
public:
	CModuleLoader();
	~CModuleLoader();
	bool Load(const std::string &libFile);
	bool Load(const std::string &libFile, int flag);
	bool IsLoaded();
	int GetSymbol(const std::string &sym, void **ptr);
	void UnLoad();

private:
#ifdef _WIN32
	HINSTANCE m_hInstLib;
#else
	void* m_hInstLib;
#endif
};


#endif  //_MODULE_LOADER_H_