#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SYSTEM_WIN32

//#define SYS_LINUX


#if defined(SYSTEM_WIN32)
#ifdef EXPORTS
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT __declspec(dllimport)
#endif
#else /* not WIN32 */
#define API_EXPORT
#endif

# if defined(SYSTEM_WIN32)
#   define STDCALL __stdcall
# else
#   define STDCALL
# endif

//#define _USE_C11

#define PLUGIN_CONFIG_FILE	"./plugins.xml"

#if defined(SYSTEM_WIN32)
#define DEFAULT_PLUGIN_DIR	".\\plugins\\"
#else
#define DEFAULT_PLUGIN_DIR	"./plugins/"
#endif

#define PASTESTR(A, B) A##B

#endif  //__CONFIG_H__