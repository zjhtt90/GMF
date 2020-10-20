#include "SEvent.h"

#ifdef _USE_C11
#include <chrono>

CSEvent::CSEvent() : m_signaled(false)
{
}


CSEvent::~CSEvent()
{
}

bool CSEvent::wait()
{
	return timedwait(-1);
}

bool CSEvent::trywait()
{
	return timedwait(0);
}

bool CSEvent::post()
{
	std::unique_lock<std::mutex> lk(m_mutex);
	m_signaled = true;
	m_cond.notify_all();

	return true;
}

bool CSEvent::timedwait(int millisec)
{
	bool ok = false;

    std::unique_lock<std::mutex> lk(m_mutex);

    while (!m_signaled)
    {
        if (millisec >= 0)
        {
			int ret = m_cond.wait_for(lk, std::chrono::milliseconds(millisec));
			if(ret == std::cv_status::timeout)
			{
				break;
			}
        }
        else
        {
			m_cond.wait(lk);
        }
    }

    ok = m_signaled;

    if (m_signaled)
    {
        m_signaled = false;
    }

    return ok;
}

void CSEvent::reset()
{
	m_signaled = false;
}

#else
#ifdef _WIN32

CSEvent::CSEvent()
{
	m_object = CreateEvent( NULL, TRUE, FALSE, NULL );
}


CSEvent::~CSEvent()
{
	CloseHandle( m_object );
	m_object = NULL;
}

bool CSEvent::wait()
{
	return timedwait(INFINITE);
}

bool CSEvent::trywait()
{
	return timedwait(0);
}

bool CSEvent::post()
{
	return ( 0 != SetEvent( m_object ) );
}

bool CSEvent::timedwait(int millisec)
{
	bool ok = false;
	DWORD ret = WaitForSingleObject( m_object, millisec );
	if ( WAIT_OBJECT_0 == ret )
	{
		ResetEvent( m_object );
		ok = true;
	}
	return ok;
}

void CSEvent::reset()
{
	ResetEvent( m_object );
}

#else

CSEvent::CSEvent()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&m_mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	pthread_cond_init(&m_cond, NULL);
	m_signaled = false;
}


CSEvent::~CSEvent()
{
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
}

bool CSEvent::wait()
{
	return timedwait(-1);
}

bool CSEvent::trywait()
{
	return timedwait(0);
}

bool CSEvent::post()
{
	pthread_mutex_lock(&m_mutex);
	m_signaled = true;
	pthread_cond_broadcast(&m_cond);
	pthread_mutex_unlock(&m_mutex);
	return true;
}

bool CSEvent::timedwait(int millisec)
{
	bool ok = false;

	pthread_mutex_lock(&m_mutex);

	struct timespec spec;
	while (!m_signaled)
	{
		if (millisec >= 0)
		{
			getTimeout(&spec, millisec);
			int rc = pthread_cond_timedwait(&m_cond, &m_mutex, &spec);
			if (rc == ETIMEDOUT)
			{
				break;
			}
		}
		else
		{
			pthread_cond_wait(&m_cond, &m_mutex);
		}
	}

	ok = m_signaled;

	if (m_signaled)
	{
		m_signaled = false;
	}

	pthread_mutex_unlock(&m_mutex);

	return ok;
}

void CSEvent::getTimeout(struct timespec *spec, int timer)
{
	struct timeval current;
	gettimeofday(&current, NULL);
	TIMEVAL_TO_TIMESPEC( &current, spec );
	spec->tv_sec = current.tv_sec + ((timer + current.tv_usec / 1000)
		/ 1000);
	spec->tv_nsec = ((current.tv_usec / 1000 + timer) % 1000) * 1000000;
}

void CSEvent::reset()
{
	m_signaled = false;
}


#endif

#endif