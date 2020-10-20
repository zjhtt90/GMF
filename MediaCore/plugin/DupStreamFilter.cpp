#include "DupStreamFilter.h"
#include "../MediaBuffer.h"
#include "../../Common/Util.h"

#define LOG_FILTER	"DupStreamFilter"

CDupStreamFilter::CDupStreamFilter()
{
	Init("DupStreamFilter");
}

CDupStreamFilter::CDupStreamFilter(const std::string &name)
{
	Init(name);
}

CDupStreamFilter::~CDupStreamFilter()
{

}

void CDupStreamFilter::Init(const std::string &name)
{
	m_desc.SetElementType(ELEMENT_TYPE_FILTER);
	m_desc.SetElementName(name);

	m_inPorts[0] = new CMediaPort(this, PORT_DIR_IN, 0);
}

void CDupStreamFilter::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData mediaProp;
		m_desc.GetMetaData(META_KEY_MEDIA, mediaProp);

		if(m_inPorts[0])
			m_inPorts[0]->SetProperty(mediaProp);
	}
}

void CDupStreamFilter::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(buffer != NULL)
	{
		unsigned inCnt, outCnt = 0;
		m_desc.GetElementPortCount(inCnt, outCnt);
		for(int i = 0; i < outCnt; i++)
		{
			CMediaBuffer *outBuf = new CMediaBuffer(*buffer);
			m_outPorts[i]->PushBufferToDownStream(outBuf);
			delete outBuf;
		}
	}
}

int CDupStreamFilter::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	return MEDIA_ERR_NONE;
}

void CDupStreamFilter::PortSettingChanged(TRACKID index, const std::string &key)
{
	MetaData data;
	m_inPorts[index]->GetProperty(key, data);

	unsigned inCnt, outCnt = 0;
	m_desc.GetElementPortCount(inCnt, outCnt);
	for(int i = 0; i < outCnt; i++)
	{
		m_outPorts[i]->UpdateProperty(data);
	}
}

void CDupStreamFilter::UseParame(const MetaData &data)
{
	SetPrivateData(data);

	if(data.mKey == META_KEY_OUTPORT_COUNT)
	{
		int outPortCnt = CUtil::convert<int, std::string>(data.mValue);

		for(int i = 0; i < outPortCnt; i++)
		{
			m_outPorts[i] = new CMediaPort(this, PORT_DIR_OUT, i);
		}

		m_desc.SetElementPortCount(1, outPortCnt);
	}
}