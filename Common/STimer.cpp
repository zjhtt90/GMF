#include "STimer.h"
#include <signal.h>
#include <string.h>
#include "LogManager.h"

#define LOG_FILTER	"CSThread"

CSTimer::CSTimer() : m_timerID(0), m_callback(NULL), m_pObj(NULL)
{

}


CSTimer::~CSTimer()
{
}

#ifdef _WIN32
int CSTimer::Create(OnTimerCallback cb, void* pObj)
{
	m_callback = cb;
	m_pObj = pObj;

	m_event.reset();
	if (!CreateTimerQueueTimer(&m_timerID, NULL, TimerRoutine, this, -1, 0, WT_EXECUTEDEFAULT))
	{
		return -1;
	}

	return 0;
}

int CSTimer::Destory()
{
	if (m_timerID)
	{
		m_event.wait();
		if (!DeleteTimerQueueTimer(NULL, m_timerID, INVALID_HANDLE_VALUE))
		{
			return -1;
		}
		m_timerID = NULL;
	}

	return 0;
}

int CSTimer::Schedule(int dueTime, int period)
{
	if (!ChangeTimerQueueTimer(NULL, m_timerID, dueTime, period))
	{
		return -1;
	}

	return 0;
}

VOID CALLBACK CSTimer::TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    CSTimer* pThis = reinterpret_cast<CSTimer*>(lpParam);
    if (pThis->m_callback)
    {
        pThis->m_callback(pThis->m_pObj);
    }
    else
    {
        pThis->OnTimer();
    }

    pThis->m_event.post();
}
#else
int CSTimer::Create(OnTimerCallback cb, void* pObj)
{
    m_callback = cb;
    m_pObj = pObj;

    m_event.reset();

    struct sigevent evp;
    memset(&evp, 0, sizeof(struct sigevent));
    evp.sigev_value.sival_ptr = this;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = TimerThreadFunc;

    if(timer_create(CLOCK_REALTIME, &evp, &m_timerID) == -1)
    {
        LOG_ERR("fail to timer_create");
        return -1;
    }

    return 0;
}

int CSTimer::Destory()
{
    if (m_timerID)
    {
        m_event.wait();
        timer_delete(m_timerID);
        m_timerID = NULL;
    }

    return 0;
}

int CSTimer::Schedule(int dueTime, int period)
{
    struct itimerspec it;
    it.it_interval.tv_sec = period/1000;
    it.it_interval.tv_nsec = (period%1000)*1000;
    it.it_value.tv_sec = dueTime/1000;
    it.it_value.tv_nsec = (dueTime%1000)*1000;
    if(timer_settime(m_timerID, 0, &it, NULL) == -1)
    {
        LOG_ERR("fail to timer_settime");
        return -1;
    }

    return 0;
}

void CSTimer::TimerThreadFunc(union sigval v)
{
    CSTimer* pThis = reinterpret_cast<CSTimer*>(v.sival_ptr);
    if (pThis->m_callback)
    {
        pThis->m_callback(pThis->m_pObj);
    }
    else
    {
        pThis->OnTimer();
    }

    pThis->m_event.post();
}

#endif

void CSTimer::OnTimer()
{

}


