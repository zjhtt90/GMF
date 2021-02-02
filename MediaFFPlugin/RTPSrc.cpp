#include "RTPSrc.h"

#include "../Common/LogManager.h"
#include "../MediaCore/MediaBuffer.h"

#include "jrtplib3/rtppacket.h"
#include "jrtplib3/rtpudpv4transmitter.h"
#include "jrtplib3/rtpipv4address.h"
#include "jrtplib3/rtpsessionparams.h"
#include "jrtplib3/rtperrors.h"

#include <sstream>

#include <assert.h>

#define LOG_FILTER	"RTPSrc"

short CRTPSrc::m_ID = 0;

CRTPSrc::CRTPSrc()
{
	std::stringstream ss;
	ss.setf(std::ios::dec);
	ss.width(2);
	ss.fill('0');
	ss << m_ID;
	std::string name = "RTPSrc" + ss.str();	

	Init(name);
}

CRTPSrc::CRTPSrc(const std::string &name)
{
	Init(name);
}

CRTPSrc::~CRTPSrc()
{
	m_ID--;
	assert(m_ID >= 0);
}

void CRTPSrc::Init(const std::string &name)
{
	m_ID++;

	m_desc.SetElementType(ELEMENT_TYPE_SRC);
	m_desc.SetElementName(name);
	m_desc.SetElementPortCount(0, 1);

	m_outPorts[0] = new CMediaPort(this, PORT_DIR_OUT);

	m_outPorts[0]->SetActiveMode(true, STREAMING_PUSH);

}

int CRTPSrc::Open()
{
	int res = MEDIA_ERR_NONE;
	MetaData data;
	res = m_desc.GetMetaData(META_KEY_PORT_BASE, data);
	if(res == MEDIA_ERR_NONE)
	{
		int localport = 0;
		std::stringstream ss;
		ss << data.mValue;
		ss >> localport;
		res = Open(localport);
	}

	return res;
}

int CRTPSrc::Open(int portbase)
{
	int status;
	jrtplib::RTPUDPv4TransmissionParams transparams;
	jrtplib::RTPSessionParams sessparams;

	// IMPORTANT: The local timestamp unit MUST be set, otherwise
	//            RTCP Sender Report info will be calculated wrong
	// In this case, we'll be just use 8000 samples per second.
	sessparams.SetOwnTimestampUnit(1.0/8000.0);		
	
	sessparams.SetAcceptOwnPackets(false);
	transparams.SetPortbase(portbase);
	status = m_rtpSess.Create(sessparams,&transparams);	
	if(status < 0)
	{
		LOG_ERR(jrtplib::RTPGetErrorString(status).c_str());
		return MEDIA_ERR_RTP_FAILED;
	}

	if(m_rtpSess.WaitForIncomingData(jrtplib::RTPTime(5,0)) < 0)
	{
		return MEDIA_ERR_RTP_TIMEOUT;
	}

	return MEDIA_ERR_NONE;
}

void CRTPSrc::Close()
{
	m_rtpSess.BYEDestroy(jrtplib::RTPTime(10,0),0,0);
}

void CRTPSrc::SetState(MediaElementState state)
{
	CMediaElement::SetState(state);

	if(state == MEDIA_ELEMENT_STATE_READY)
	{
		MetaData data;
		m_desc.GetMetaData(META_KEY_MEDIA, data);
		m_outPorts[0]->SetProperty(data);
		if(data.mValue == "Video")
		{
			if(m_desc.GetMetaData(META_KEY_VIDEO_WIDTH, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
			if(m_desc.GetMetaData(META_KEY_VIDEO_HEIGHT, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
			if(m_desc.GetMetaData(META_KEY_PIX_FORMAT, data) == MEDIA_ERR_NONE)
				m_outPorts[0]->SetProperty(data);
		}
		else if(data.mValue == "Audio")
		{
			//TODO for audio param
		}

		//m_desc.Print();
	}
	else if(state == MEDIA_ELEMENT_STATE_OPEN)
	{
		if(Open() == MEDIA_ERR_NONE)
		{
			m_outPorts[0]->StartTask();
		}
	}
	else if(state == MEDIA_ELEMENT_STATE_PAUSE)
	{
		m_outPorts[0]->PauseTask();
	}
	else if(state == MEDIA_ELEMENT_STATE_STOP)
	{
		m_outPorts[0]->StopTask();

		Close();
	}
}


int CRTPSrc::FillOutBuffer(TRACKID &id, CMediaBuffer **buffer)
{
	int res = MEDIA_ERR_NONE;

	id = 0;
	m_rtpSess.Poll();

	m_rtpSess.BeginDataAccess();
		
	// check incoming packets
	if (m_rtpSess.GotoFirstSourceWithData())
	{
		jrtplib::RTPPacket *pack = nullptr;
				
		if((pack = m_rtpSess.GetNextPacket()) != nullptr)
		{
			// You can examine the data here
					
			// we don't longer need the packet, so
			// we'll delete it
			
			*buffer = new CMediaBuffer(pack->GetPacketData(), 
				pack->GetPacketLength(), pack->GetTimestamp(), pack->GetReceiveTime().GetMicroSeconds(), 0);
			m_rtpSess.DeletePacket(pack);
		}
		else
		{
			res = MEDIA_ERR_READ_FAILED;
		}
	}
		
	m_rtpSess.EndDataAccess();

	return res;
}