#include "SThread.h"
#include "LogManager.h"
#include <iostream>

#define LOG_FILTER	"CSThread"

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
	m_tObj = (HANDLE)_beginthreadex(NULL, 0, CSThread::ThreadFunction, (void*)this, 0, &threadID);
	if(m_tObj == 0)
	{
		return ERR_THREAD_CANTSTARTTHREAD;
	}

	/* Wait until 'running' is set */
	/*
	m_mutex.Lock();			
	while (!m_bRunning)
	{
		m_mutex.UnLock();
		std::cout << "In thread::start, sleep 1 s, m_bRunning = " << m_bRunning << std::endl;
		Sleep(1000);
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
	DWORD res = WaitForSingleObject(m_tObj, INFINITE);
	if(res != WAIT_OBJECT_0)
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
	::Sleep(millliseconds);
}

unsigned __stdcall CSThread::ThreadFunction(void* pThr)
{
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
	//LOG_INFO("Thread proc end");
	pThread->m_mutex.Lock();
	pThread->m_bRunning = false;
	pThread->m_mutex.UnLock();

	CloseHandle(pThread->m_tObj);
	pThread->m_tObj = 0;
	_endthreadex(0);
	return 0;
}
