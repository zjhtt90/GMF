#ifndef _AUDIO_FRAME_BUFFER_H_
#define _AUDIO_FRAME_BUFFER_H_

#include "MediaBaseFilter.h"
#include "SMutex.h"

#include <array>
#include <queue>
#include <forward_list>

#define MAX_BUF_NODE_LEN	10

class CAudioFrameBuffer : public CMediaBaseFilter
{
public:
	CAudioFrameBuffer();
	CAudioFrameBuffer(const std::string &name);
	virtual ~CAudioFrameBuffer();

	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);
private:
	void Init(const std::string &name);
	bool Init(unsigned int samplesize, unsigned int channel, unsigned int samplerate,
		unsigned int samplecount);
	void Free();
	bool IsEmpty() const;
	bool Push(const CMediaBuffer &item, unsigned int linesize, unsigned int samplecount);
	bool Push(const unsigned char *data, unsigned int linesize, unsigned int samplecount, unsigned long long ts);
	bool PushData(const unsigned char *data, unsigned int linesize, unsigned int samplecount, unsigned long long ts);

private:
	std::queue<CMediaBuffer*> m_datanodes; // 已使用的音频节点
	CSMutex m_lock; // buffer线程锁
	unsigned char *m_swapbuffer; // 交换缓冲区
	unsigned int m_samplecount; // 每块音频数据的采样数
	unsigned int m_swapsamplecount; // 交换缓冲采样数
	float m_durPerSample; // 每个采样点的时间(毫秒)
	bool m_bInit;
	unsigned int m_linesize;
};


class CAudioFrameBufferFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CAudioFrameBuffer();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CAudioFrameBuffer(name);
	}

};



#endif  //_AUDIO_FRAME_BUFFER_H_