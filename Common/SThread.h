#ifndef _SERVER_THREAD_H_
#define _SERVER_THREAD_H_

#include "SMutex.h"
#ifdef _USE_C11
#include <thread>
typedef std::thread* ThreadHandle;
#else
#ifdef _WIN32
#include <windows.h>
#include <process.h>
typedef HANDLE ThreadHandle;
#else
#include <pthread.h>
typedef pthread_t ThreadHandle;
#endif
#endif

#define ERR_THREAD_CANTINITMUTEX						-1
#define ERR_THREAD_CANTSTARTTHREAD						-2
#define ERR_THREAD_THREADFUNCNOTSET						-3
#define ERR_THREAD_NOTRUNNING							-4
#define ERR_THREAD_ALREADYRUNNING						-5


typedef void (*THREAD_PROC)(void *pUserData);
class API_EXPORT CSThread
{
public:
	CSThread();
	CSThread(THREAD_PROC func, void *pUserData);
	virtual ~CSThread();

	int Start();
	int Kill();
	void Join();
	bool IsRunning();

	static void Sleep(unsigned long millliseconds);

protected:
	virtual void Run()
	{

	}

private:
	/// disable copy and assign
	CSThread(const CSThread &thr);
	CSThread& operator = (const CSThread& thr);

#ifdef _USE_C11
	static void ThreadFunction(CSThread *pThr);
#else
#ifdef _WIN32
	static unsigned __stdcall ThreadFunction(void *pThr);
#else
	static void* ThreadFunction(void *pThr);
#endif
#endif

private:
	ThreadHandle m_tObj;
	bool m_bRunning;
	CSMutex m_mutex;

	THREAD_PROC m_pFunc;
	void *m_pUserData;
};


#endif  //_SERVER_THREAD_H_