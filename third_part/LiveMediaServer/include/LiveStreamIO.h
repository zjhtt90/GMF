#ifndef _LIVE_STREAM_IO_H
#define _LIVE_STREAM_IO_H

#ifdef _WIN32
#include <windows.h>
#elif
#include <pthread.h>
#include <unistd.h>
#endif
#include <string>

typedef void(*LiveStreamSrcCallback)(int state, void* pObj);

class LiveStreamIO
{
public:
	enum ParamKey
	{
		PARAM_KEY_FORMAT = 0,
		PARAM_KEY_AUDIO_SAMPLERATE,
		PARAM_KEY_AUDIO_CHANNEL,
		PARAM_KEY_AUDIO_BITPERSAMPLE,
		PARAM_KEY_AUDIO_FRAMESIZE,
		PARAM_KEY_MAX
	};

	LiveStreamIO();
	LiveStreamIO(int capacity);
	~LiveStreamIO();

	void SetCallback(LiveStreamSrcCallback pCb, void* pObj);

	int Open();
	int Close();
	int Write(const unsigned char* pData, int len);
	int Read(unsigned char* pData, int maxLen);

	void SetExtParam(int key, int val);
	void SetExtParam(int key, std::string& val);

	int GetExtParam(int key, int& val);
	int GetExtParam(int key, std::string& val);
private:
	void Lock();
	void UnLock();
	int GetValideLength();
	int Reset();
	int Rewind();

private:
	unsigned char* m_buffer;
	int m_capacity;
	int	m_writePos;
	int m_readPos;

	bool m_bOpen;
#ifdef _WIN32
	CRITICAL_SECTION m_lock;
#elif
	pthread_mutex_t m_lock;
#endif
	std::string m_paramVec[PARAM_KEY_MAX];

	LiveStreamSrcCallback m_pCb;
	void* m_pObj;
};


#endif //_LIVE_STREAM_IO_H