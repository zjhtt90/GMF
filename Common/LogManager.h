#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

//#define USE_LOG4CPP
#include <string>

#ifdef USE_LOG4CPP
#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include <log4cpp/PropertyConfigurator.hh>

#define LOG_LEVEL_DEBUG	log4cpp::Priority::DEBUG
#define LOG_LEVEL_INFO	log4cpp::Priority::INFO
#define LOG_LEVEL_WARN	log4cpp::Priority::WARN
#define LOG_LEVEL_ERROR	log4cpp::Priority::ERROR

#define LOG_FILTER	"Unknow"
#define FILTE_CATER	log4cpp::Category::getInstance(std::string(LOG_FILTER))

#ifdef _TRACE_DEBUG
#define LOG_DEBUG(fmt, ...)	FILTE_CATER.debug(fmt, ##__VA_ARGS__)
#define LOG_DEBUG_S(str)	FILTE_CATER.debug(str)
#define LOG_INFO(fmt, ...)	FILTE_CATER.info(fmt, ##__VA_ARGS__)
#define LOG_INFO_S(str)	FILTE_CATER.info(str)
#define LOG_WARN(fmt, ...)	FILTE_CATER.warn(fmt, ##__VA_ARGS__)
#define LOG_WARN_S(str)	FILTE_CATER.warn(str)
#define LOG_ERR(fmt, ...)	FILTE_CATER.error(fmt, ##__VA_ARGS__)
#define LOG_ERR_S(str)	FILTE_CATER.error(str)
#else
#define LOG_DEBUG(fmt, ...)
#define LOG_DEBUG_S(str)
#define LOG_INFO(fmt, ...)
#define LOG_INFO_S(str)
#define LOG_WARN(fmt, ...)
#define LOG_WARN_S(str)
#define LOG_ERR(fmt, ...)
#define LOG_ERR_S(str)
#endif
#else
#include "Logger.h"

class CLogManager;

#define LOG_TRACE(level, fmt, ...) do{ \
	const location_info_t locinfo = LOCATION_INFO_INITIALIZER(NULL); \
	CLogManager::log(level, &locinfo, fmt, ##__VA_ARGS__); \
}while(0);

#define LOG_TRACE_S(level, str) do{ \
	const location_info_t locinfo = LOCATION_INFO_INITIALIZER(NULL); \
	CLogManager::log(level, &locinfo, str); \
}while(0);

#define LOG_DEBUG(fmt, ...)	LOG_TRACE(CLogger::kDebug, fmt, ##__VA_ARGS__)
#define LOG_DEBUG_S(str)		LOG_TRACE_S(CLogger::kDebug, str)

#define LOG_INFO(fmt, ...)	LOG_TRACE(CLogger::kInfo, fmt, ##__VA_ARGS__)
#define LOG_INFO_S(str)		LOG_TRACE(CLogger::kInfo, str)

#define LOG_WARN(fmt, ...)	LOG_TRACE(CLogger::kWarning, fmt, ##__VA_ARGS__)
#define LOG_WARN_S(str)		LOG_TRACE(CLogger::kWarning, str)

#define LOG_ERR(fmt, ...)	LOG_TRACE(CLogger::kError, fmt, ##__VA_ARGS__)
#define LOG_ERR_S(str)		LOG_TRACE(CLogger::kError, str)

#endif

class CLogManager 
{
public:

	inline static void setLoggerInfo(enum CLogger::LoggerType type = CLogger::LOG_COUT, enum CLogger::Level level = CLogger::kNone)
	{
		CLogger::getInstance().setLoggerInfo(type, level);
	}

	inline static void setLogPath(const char *file)
	{
		CLogger::getInstance().setLogPath(file);
	}

	inline static int startup()
	{
#ifdef USE_LOG4CPP
		log4cpp::PropertyConfigurator::configure(initFileName);
		return 0;
#else
		return CLogger::getInstance().startup();
#endif
	}

	inline static void cleanup()
	{
#ifdef USE_LOG4CPP
		log4cpp::Category::shutdown();
#else
		CLogger::getInstance().cleanup();
#endif
	}

	inline static void log(int level, const location_info_t* a_locinfo, const std::string &content)
	{
		log(level, a_locinfo, content.c_str());
	}

	inline static void log(int level, const location_info_t* a_locinfo, const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		CLogger::getInstance().log(level, a_locinfo, format, va);
		va_end(va);
	}

	inline static void debug(const location_info_t* a_locinfo, const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		CLogger::getInstance().debug(a_locinfo, format, va);
		va_end(va);
	}

	inline static void info(const location_info_t* a_locinfo, const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		CLogger::getInstance().info(a_locinfo, format, va);
		va_end(va);
	}

	inline static void warning(const location_info_t* a_locinfo, const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		CLogger::getInstance().warning(a_locinfo, format, va);
		va_end(va);
	}

	inline static void error(const location_info_t* a_locinfo, const char* format, ...)
	{
		va_list va;
		va_start(va, format);
		CLogger::getInstance().error(a_locinfo, format, va);
		va_end(va);
	}
};




#endif  //LOGMANAGER_H_