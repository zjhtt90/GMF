#include "AudioFrameBuffer.h"

#include "../MediaBuffer.h"
#include "../../Common/Util.h"
#include "../../Common/LogManager.h"

#define LOG_FILTER	"AudioFrameBuffer"

CAudioFrameBuffer::CAudioFrameBuffer() : m_swapbuffer(NULL), m_samplecount(1024), m_swapsamplecount(0),
	m_durPerSample(0)
{
	Init("AudioFrameBuffer");
}

CAudioFrameBuffer::CAudioFrameBuffer(const std::string &name) : m_swapbuffer(NULL), m_samplecount(1024), 
	m_swapsamplecount(0), m_durPerSample(0)
{
	Init(name);
}

CAudioFrameBuffer::~CAudioFrameBuffer()
{
}

void CAudioFrameBuffer::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN);
	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	m_desc.SetElementPortCount(1, 1);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);
}

bool CAudioFrameBuffer::Init(unsigned int samplesize, unsigned int channel, unsigned int samplerate,
		  unsigned int samplecount)
{
	Free();

	m_linesize = samplesize * channel;
	// 分配交换缓冲区
	m_swapbuffer = new unsigned char[m_linesize*samplecount];
	if(!m_swapbuffer) 
	{
		return false;
	}

	m_durPerSample = 1000.0 / samplerate;

	m_bInit = true;
	return true;
}

void CAudioFrameBuffer::Free()
{
	if(m_bInit)
	{
		while(!m_datanodes.empty())
		{
			CMediaBuffer *buffer = m_datanodes.front();
			m_datanodes.pop();
			delete buffer;
		}

		delete[]m_swapbuffer;
		m_swapbuffer = NULL;
		m_swapsamplecount = 0;
		//m_samplecount = 0;
		m_durPerSample = 0;
		m_bInit = false;
	}
}

bool CAudioFrameBuffer::IsEmpty() const
{
	return m_datanodes.empty();
}

void CAudioFrameBuffer::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		int sampleRate;
		int channel;
		int sampleFmt;

		if(m_inPorts[0]->GetProperty(META_KEY_SAMPLE_RATE, data) == MEDIA_ERR_NONE)
		{
			sampleRate = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_inPorts[0]->GetProperty(META_KEY_CHANNEL_NUM, data) == MEDIA_ERR_NONE)
		{
			channel = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_inPorts[0]->GetProperty(META_KEY_SAMPLE_FORMAT, data) == MEDIA_ERR_NONE)
		{
			sampleFmt = CUtil::convert<int, std::string>(data.mValue);
		}

		if(m_desc.GetMetaData(META_KEY_FRAME_SAMPLE_COUNT, data) == MEDIA_ERR_NONE)
		{
			m_samplecount = CUtil::convert<int, std::string>(data.mValue);
			m_outPorts[0]->SetProperty(data);
		}

		Init(GetBytesPerAudioSample((AudioSampleFormat)sampleFmt), channel, sampleRate, m_samplecount);
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		m_outPorts[0]->StartTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_RESUME)
	{
		m_outPorts[0]->StartTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();
	}
}

void CAudioFrameBuffer::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer != NULL)
	{
		Push(*buffer, m_linesize, buffer->GetDataSize()/m_linesize);
	}
}

int CAudioFrameBuffer::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	m_lock.Lock();
	if(m_datanodes.empty())
	{
		m_lock.UnLock();
		return MEDIA_ERR_READ_FAILED;
	}

	*buffer = m_datanodes.front();
	m_datanodes.pop();
	m_lock.UnLock();
	return MEDIA_ERR_NONE;
}

void CAudioFrameBuffer::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);

	m_desc.SetExternData(data);

	m_outPorts[index]->UpdateProperty(data);
}


bool CAudioFrameBuffer::Push(const CMediaBuffer &item, unsigned int linesize, unsigned int samplecount)
{
	return Push(item.GetData(), linesize, samplecount, item.GetPts());
}

bool CAudioFrameBuffer::Push(const unsigned char *data, unsigned int linesize, unsigned int samplecount, unsigned long long ts)
{
	// 校验数据合法性
	if(data == NULL ) 
	{
		LOG_ERR("Push back invalid data");
		return false;
	}
	// 如果push的数据比较采样块大, 则依次处理
	while(samplecount > m_samplecount) 
	{
		unsigned int diff_sample = m_samplecount - m_swapsamplecount;
		memcpy(m_swapbuffer+m_swapsamplecount*m_linesize, data, diff_sample*m_linesize);
		uint64_t cur_ts = ts - static_cast<uint64_t>((samplecount - diff_sample) * m_durPerSample);
		if(!PushData(m_swapbuffer, m_linesize, m_samplecount, cur_ts))
			return false;
		data += diff_sample * m_linesize;
		samplecount -=  diff_sample; 
		m_swapsamplecount = 0;
	}
	// 处理剩下的采样数据
	if(m_swapsamplecount > 0) 
	{
		if((samplecount+m_swapsamplecount) >= m_samplecount) 
		{
			unsigned int diff_sample = m_samplecount - m_swapsamplecount;
			memcpy(m_swapbuffer+m_swapsamplecount*m_linesize, data, diff_sample*m_linesize);
			uint64_t cur_ts = ts - static_cast<uint64_t>((samplecount - diff_sample) * m_durPerSample);
			if(!PushData(m_swapbuffer, m_linesize, m_samplecount, cur_ts))
				return false;
			data += diff_sample * m_linesize;
			samplecount -=  diff_sample; 
			m_swapsamplecount = 0;
		}
	}
	if(samplecount > 0) 
	{
		memcpy(m_swapbuffer+m_swapsamplecount*m_linesize, data, samplecount*m_linesize);
		m_swapsamplecount += samplecount;
	}

	return true;
}

bool CAudioFrameBuffer::PushData(const unsigned char *data, unsigned int linesize, unsigned int samplecount, unsigned long long ts)
{
	m_lock.Lock();
	if(m_datanodes.size() >= MAX_BUF_NODE_LEN)
	{
		LOG_WARN("Buffer full");
		CMediaBuffer *buf = m_datanodes.front();
		m_datanodes.pop();
		delete buf;
	}

	m_datanodes.push(new CMediaBuffer(data, linesize*samplecount, ts, ts, 0));
	m_lock.UnLock();

	return true;
}