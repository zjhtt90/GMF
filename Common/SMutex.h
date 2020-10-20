#ifndef _SEVER_MUTEXT_H_
#define _SEVER_MUTEXT_H_

#include "../config.h"

#ifdef _USE_C11
#include <mutex>
typedef std::mutex	MutexObj;
#else
#ifdef _WIN32
#include <windows.h>
typedef HANDLE	MutexObj;
#else
#include <pthread.h>
typedef pthread_mutex_t	MutexObj;
#endif
#endif

/* ª•≥‚À¯¿‡ */
class API_EXPORT CSMutex
{
public:
	CSMutex();
	~CSMutex();

	void Lock();
	bool TryLock();
	void UnLock();

private:
	/// disable copy and assign
	CSMutex( const CSMutex& mtx );
	CSMutex& operator = ( const CSMutex& mtx );

private:
	MutexObj m_mutext;
};


#endif  //_SEVER_MUTEXT_H_