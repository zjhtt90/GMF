#include "SThread.h"


CSThread::CSThread() : m_tObj(NULL), m_bRunning(false), m_pFunc(NULL), m_pUserData(NULL)
{

}

CSThread::CSThread(THREAD_PROC func, void *pUserData) : m_tObj(NULL), m_bRunning(false), m_pFunc(func), m_pUserData(pUserData)
{

}

CSThread::~CSThread()
{
	delete m_tObj;
}

int CSThread::Start()
{
	m_mutex.Lock();
	if (m_bRunning)
	{
		m_mutex.UnLock();
		return ERR_THREAD_ALREADYRUNNING;
	}
	m_mutex.UnLock();

	m_tObj = new std::thread(CSThread::ThreadFunction, this);

	m_tObj->detach();

	/* Wait until 'running' is set */
	/*
	m_mutex.Lock();			
	while (!m_bRunning)
	{
		m_mutex.UnLock();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		m_mutex.Lock();
	}
	m_mutex.UnLock();
	*/
	return 0;
}

int CSThread::Kill()
{
	m_mutex.Lock();			
	if (!m_bRunning)
	{
		m_mutex.UnLock();
		return ERR_THREAD_NOTRUNNING;
	}
	m_bRunning = false;
	m_mutex.UnLock();

	m_tObj->join();
	return 0;
}

void CSThread::Join()
{
	m_tObj->join();
}

bool CSThread::IsRunning()
{
	bool r;

	m_mutex.Lock();
	r = m_bRunning;
	m_mutex.UnLock();
	return r;
}

void CSThread::Sleep(unsigned long millliseconds)
{
	std::this_thread::sleep_for(std::chrono::millseconds(millliseconds));
}

void CSThread::ThreadFunction(CSThread *pThr)
{
	pThr->m_mutex.Lock();
	pThr->m_bRunning = true;
	pThr->m_mutex.UnLock();

	if(pThr->m_pFunc != NULL)
	{
		pThr->m_pFunc(pThr->m_pUserData);
	}
	else
	{
		pThr->Run();
	}

	pThr->m_mutex.Lock();
	pThr->m_bRunning = false;
	pThr->m_mutex.UnLock();
}
