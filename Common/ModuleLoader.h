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
	static bool Load(const std::string &libFile);
	static bool Load(const std::string &libFile, int flag);
	static int GetSymbol(const std::string &sym, void **ptr);
	static void UnLoad();

private:
#ifdef _WIN32
	static HINSTANCE m_hInstLib;
#else
	static void* m_hInstLib;
#endif
};


#endif  //_MODULE_LOADER_H_