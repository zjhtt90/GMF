#ifndef _UTIL_H_
#define _UTIL_H_

#include "../config.h"

#include <sstream>

#ifdef _WIN32
#define INIT_NETWORK	CUtil::InitNet()
#define UNINIT_NETWORK	CUtil::DelNet()

#define snprintf(dst, count, strfmt, ...)	_snprintf(dst, count, strfmt, ##__VA_ARGS__);
#else
#define INIT_NETWORK
#define UNINIT_NETWORK
#endif


class API_EXPORT CUtil
{
public:
	/* 基本数据类型转换 */
	template<class out_type, class in_type>
	static out_type convert(const in_type &t)
	{
		std::stringstream ss;
		ss << t;
		out_type res;
		ss >> res;

		return res;
	}

	/* 消除字符串两边的空格 */
	static std::string& StrTrim(std::string &s);

#ifdef _WIN32
	/* 初始化win32网络环境 */
	static int InitNet();
	static void DelNet();
#endif

	/* 获取主机名 */
	static std::string GetHostName();

	/* 获取主机地址 */
	static std::string GetHotAddr(const std::string &name);

	static long long GetTimeStamp();
};


#endif  //_UTIL_H_
