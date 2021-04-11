#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

#include "../config.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define LOG_FILTER	"Unkonw"
#define TOSTR(x) #x
#define TOFILTER(x) TOSTR(x)

typedef struct 
{
	const char* loc_filter;
    const char* loc_file;
    int loc_line;
    const char* loc_function;
    void* loc_data;

}location_info_t;

/**
 * location_info_t initializer 
 **/
#define LOCATION_INFO_INITIALIZER(user_data) {LOG_FILTER, __FILE__, __LINE__, __FUNCTION__, user_data }

typedef void(*log_output_cb)(int level, const char* msg);

class API_EXPORT CLogger
{
public:

	/// 日志级别
    enum Level
    {
        kNone =  0,
        kDebug = 1,
        kInfo,
        kWarning,
        kError
    };

    /// 日志输出类型
    enum LoggerType
    {
        LOG_COUT = 0,  /// 控制台输出
        LOG_SYSLOG,    /// 系统日志
        LOG_FILE,      /// 循环文件输出
        LOG_DEBUGWINDOW,   /// windows系统下的调试输出，即 OutputDebugString

        LOG_EXTERNAL,      /// 外部输出

        LOG_MAXLOGGER,
		LOG_C_F
    };

    enum FormatFlag
	{
    	DATETIME = 0x01,
		TIME 	 = 0x02,
	};

	CLogger();
	CLogger(LoggerType type, Level level);
	~CLogger();

	static CLogger& getInstance();

	void setLoggerInfo(enum LoggerType type = LOG_COUT, enum Level level = kNone);

	void setLogPath(const char *file);

	void setLogCallback(log_output_cb fn);

	int startup();

    void cleanup();

	void log(int level, const location_info_t* a_locinfo, const char* format, va_list args);

	/**
     * 写调试级别的日志
     * @param format 日志格式
     */
    void debug(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * 写信息级别的日志
     * @param fmt 日志格式
     */
    void info(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * 写警告级别的日志
     * @param fmt 日志格式
     */
    void warning(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * 写错误级别的日志
     * @param fmt 日志格式
     */
    void error(const location_info_t* a_locinfo,const char* format, va_list args);

private:
	void logv(int level, const location_info_t* a_locinfo,const char* format, va_list args);
	void ConsoleOut(int level, const char *content);
	void FileOut(int level, const char *content);
	void FileClean();

private:
	static CLogger m_Instance;
	enum LoggerType m_type;
	enum Level	m_level;

	char m_filePath[128];
	FILE *m_filePtr;

	log_output_cb m_pLogFn;

#ifdef _WIN32
	HANDLE m_hConsoleOut;
#endif
};

#endif //LOGGER_H_

