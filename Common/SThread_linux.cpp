#include "SThread.h"

CSThread::CSThread() : m_bRunning(false), m_pFunc(NULL), m_pUserData(NULL)
{

}

CSThread::CSThread(THREAD_PROC func, void *pUserData) : m_bRunning(false), m_pFunc(func), m_pUserData(pUserData)
{

}

CSThread::~CSThread()
{
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

	unsigned threadID;
	int ret = pthread_create(&m_tObj, NULL, ThreadFunction, (void*)this);
	if(ret != 0)
	{
		return ERR_THREAD_CANTSTARTTHREAD;
	}

	/* Wait until 'running' is set */
	/*
	m_mutex.Lock();			
	while (!m_bRunning)
	{
		m_mutex.UnLock();
		sleep(1);
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
	int res = pthread_join(m_tObj, NULL);
	if(res != 0)
		return -6;
	return 0;
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
	usleep(millliseconds*1000);
}

void* CSThread::ThreadFunction(void *pThr)
{
	int reCode = 0;
	CSThread* pThread = reinterpret_cast<CSThread*>(pThr);

	pThread->m_mutex.Lock();
	pThread->m_bRunning = true;
	pThread->m_mutex.UnLock();

	if(pThread->m_pFunc != NULL)
	{
		pThread->m_pFunc(pThread->m_pUserData);
	}
	else
	{
		pThread->Run();
	}

	pThread->m_mutex.Lock();
	pThread->m_bRunning = false;
	pThread->m_mutex.UnLock();

	pthread_detach(pThread->m_tObj);
	pThread->m_tObj = (pthread_t)-1;
	pthread_exit(&reCode);
	return 0;
}
