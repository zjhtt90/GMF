#ifndef STIMER_H
#define STIMER_H

#include "SEvent.h"

typedef void(*OnTimerCallback)(void* pObj);
class CSTimer
{
public:
	CSTimer();
	~CSTimer();
	int Create(OnTimerCallback cb, void* pObj);
	int Destory();
	int Schedule(int dueTime, int period);

protected:
	virtual void OnTimer();

private:
#ifdef _WIN32
	static VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired);
#else
    static void TimerThreadFunc(union sigval v);
#endif

private:
#ifdef _WIN32
	HANDLE m_timerID;
#else
	timer_t m_timerID;
#endif
	OnTimerCallback m_callback;
	void* m_pObj;
	CSEvent m_event;
};

#endif //STIMER_H
