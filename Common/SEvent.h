#ifndef _SEVENT_H_
#define _SEVENT_H_

#include "../config.h"

#ifdef _USE_C11
#include <mutex>
#include <condition_variable>
#else
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#endif
#endif

class API_EXPORT CSEvent
{
public:
	CSEvent();
	~CSEvent();

	bool post();
	bool wait();
	bool trywait();
	bool timedwait(int millisec);
	void reset();

private:
#ifdef _USE_C11
    std::mutex m_mutex;
	std::condition_variable m_cond;
#else
#ifdef _WIN32
	HANDLE m_object;
#else
	static void getTimeout(struct timespec *spec, int timer);
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
#endif
#endif
    bool m_signaled;
};

#endif	//_TEVENT_H_

