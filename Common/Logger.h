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

	/// ��־����
    enum Level
    {
        kNone =  0,
        kDebug = 1,
        kInfo,
        kWarning,
        kError
    };

    /// ��־�������
    enum LoggerType
    {
        LOG_COUT = 0,  /// ����̨���
        LOG_SYSLOG,    /// ϵͳ��־
        LOG_FILE,      /// ѭ���ļ����
        LOG_DEBUGWINDOW,   /// windowsϵͳ�µĵ���������� OutputDebugString

        LOG_EXTERNAL,      /// �ⲿ���

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
     * д���Լ������־
     * @param format ��־��ʽ
     */
    void debug(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * д��Ϣ�������־
     * @param fmt ��־��ʽ
     */
    void info(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * д���漶�����־
     * @param fmt ��־��ʽ
     */
    void warning(const location_info_t* a_locinfo,const char* format, va_list args);

    /**
     * д���󼶱����־
     * @param fmt ��־��ʽ
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

