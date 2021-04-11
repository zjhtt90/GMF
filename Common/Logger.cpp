#include "Logger.h"

#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <Shlwapi.h>
#endif

#define LOG_HEADER_LENGTH	512
#define LOG_CONTENT_LENGTH	1024
#define LOG_LENGTH	2048

const char *g_level_name[] = {"VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR"};
const char *g_default_file = "output.log";


#ifdef _WIN32
const static WORD LOG_COLOR[CLogger::kError+ 2] = {
	FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE,
	FOREGROUND_INTENSITY|FOREGROUND_BLUE,
	FOREGROUND_GREEN,
	FOREGROUND_GREEN|FOREGROUND_RED,
	FOREGROUND_RED,
	FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
};
#else
const static char LOG_COLOR[CLogger::kError+ 2][50] = {
	"\e[0m",
	"\e[34m\e[1m",//hight blue
	"\e[32m", //green
	"\e[33m", //yellow
	"\e[31m", //red
	"\e[35m" 
};
#endif


CLogger CLogger::m_Instance;

CLogger::CLogger() : m_type(LOG_COUT),m_level(kNone), m_pLogFn(NULL)
{
	strcpy(m_filePath, g_default_file);

	m_filePtr = NULL;
}

CLogger::CLogger(LoggerType type, Level level) : m_type(type),m_level(level), m_pLogFn(NULL)
{

}


CLogger::~CLogger()
{

}

CLogger& CLogger::getInstance()
{
	return m_Instance;
}

void CLogger::setLoggerInfo(enum LoggerType type, enum Level level)
{
	m_type = type;
	m_level = level;
}

void CLogger::setLogPath(const char *file)
{
	if(file != NULL && strlen(file) > 0)
	{
		memset(m_filePath, 0, sizeof(m_filePath));
		strcpy(m_filePath, file);

		FileClean();
	}
}

void CLogger::setLogCallback(log_output_cb fn)
{
	m_pLogFn = fn;
}


int CLogger::startup()
{
	if(m_type == LOG_COUT || m_type == LOG_C_F)
	{
#ifdef _WIN32
		m_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}
	return 0;
}

void CLogger::cleanup()
{

}

void CLogger::log(int level, const location_info_t* a_locinfo,const char* format, va_list args)
{
	logv(level,a_locinfo, format, args);
}


void CLogger::debug(const location_info_t* a_locinfo,const char* format, va_list args)
{
	if(m_level > kDebug)
	{
		return;
	}
	else
	{
		logv(kDebug, a_locinfo, format, args);
	}
}


void CLogger::info(const location_info_t* a_locinfo,const char* format, va_list args)
{
	if(m_level > kInfo)
	{
		return;
	}
	else
	{
		logv(kInfo, a_locinfo, format, args);
	}
}


void CLogger::warning(const location_info_t* a_locinfo,const char* format, va_list args)
{
	if(m_level > kWarning)
	{
		return;
	}
	else
	{
		logv(kWarning, a_locinfo, format, args);
	}
}


void CLogger::error(const location_info_t* a_locinfo,const char* format, va_list args)
{
	if(m_level > kError)
	{
		return;
	}
	else
	{
		logv(kError, a_locinfo, format, args);
	}
}


void CLogger::logv(int level, const location_info_t* a_locinfo,const char* format, va_list args)
{
	time_t t;
	char timeStr[64] = {0};
	time(&t);
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S",localtime(&t));

	const char *posN = NULL;
	const char *pos = a_locinfo->loc_file;
#ifdef _WIN32
	while((posN = strchr(pos, '\\')) != NULL)
#else
	while((posN = strchr(pos, '/')) != NULL)
#endif
	{
		pos = posN+1;
	}
	char locfile[64] = {0};
	if(pos != NULL)
		strcpy(locfile, pos);

	char log_header[LOG_HEADER_LENGTH] = {0};
	sprintf(log_header, "[%s][%s][%s]%s:%s:%d: ", a_locinfo->loc_filter, 
		g_level_name[level], timeStr,
		locfile, a_locinfo->loc_function, a_locinfo->loc_line);

	char log_content[LOG_CONTENT_LENGTH] = {0};
	vsnprintf(log_content, LOG_CONTENT_LENGTH, format, args);

	char logMsg[LOG_LENGTH] = {0};
	sprintf(logMsg, "%s%s", log_header, log_content);

	switch(m_type)
	{
	case LOG_FILE:
		FileOut(level, logMsg);
		break;
	case LOG_C_F:
		ConsoleOut(level, logMsg);
		FileOut(level, logMsg);
		break;
	case LOG_COUT:
	default:
		ConsoleOut(level, logMsg);
		break;
	}

	if (m_pLogFn)
	{
		m_pLogFn(level, logMsg);
	}
}

void CLogger::ConsoleOut(int level, const char *content)
{
#ifdef _WIN32
	SetConsoleTextAttribute(m_hConsoleOut, LOG_COLOR[level]);
	printf(content);
	printf("\r\n");
	SetConsoleTextAttribute(m_hConsoleOut, LOG_COLOR[0]);
#else
	printf("%s%s", LOG_COLOR[level], content.c_str());
	printf("\033[0m");
	printf("\n");
#endif
}

void CLogger::FileOut(int level, const char *content)
{
	FILE *filePtr = NULL;

	filePtr = fopen(m_filePath, "a+");
	if(filePtr)
	{
		fprintf(filePtr, content);
		fprintf(filePtr, "\r\n");

		fclose(filePtr);
	}
}

void CLogger::FileClean()
{
	FILE *filePtr = NULL;

	filePtr = fopen(m_filePath, "w");
	if (filePtr)
	{
		fprintf(filePtr, "");
		fclose(filePtr);
	}
}