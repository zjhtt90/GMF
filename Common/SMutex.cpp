#include "SMutex.h"

CSMutex::CSMutex()
{
#ifndef _USE_C11
#ifdef _WIN32
	m_mutext = CreateMutex( NULL, false, NULL );
#else
	pthread_mutex_init(&m_mutext, NULL);
#endif
#endif
}


CSMutex::~CSMutex()
{
#ifndef _USE_C11
#ifdef _WIN32
	CloseHandle( m_mutext );
#else
	pthread_mutex_destroy(&m_mutext);
#endif
#endif
}

void CSMutex::Lock()
{
#ifdef _USE_C11
	m_mutext.lock();
#else
#ifdef _WIN32
	DWORD ret = WaitForSingleObject(m_mutext, INFINITE);
#else
	pthread_mutex_lock(&m_mutext);
#endif
#endif
}

bool CSMutex::TryLock()
{
#ifdef _USE_C11
	return m_mutext.try_lock();
#else
#ifdef _WIN32
	DWORD ret = WaitForSingleObject(m_mutext, 0);
	return (WAIT_OBJECT_0 == ret);
#else
	return (0 == pthread_mutex_trylock(&m_mutext));
#endif
#endif
}

void CSMutex::UnLock()
{
#ifdef _USE_C11
	m_mutext.unlock();
#else
#ifdef _WIN32
	ReleaseMutex(m_mutext);
#else
	pthread_mutex_unlock(&m_mutext);
#endif
#endif
}