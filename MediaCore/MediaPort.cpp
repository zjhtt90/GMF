#include "MediaPort.h"
#include "MediaElement.h"

#include "MediaBaseSrc.h"
#include "MediaBaseFilter.h"
#include "MediaBaseSink.h"

#include "MediaBuffer.h"

#include "LogManager.h"


CMediaPort::CMediaPort() 
	: m_hostElement(NULL),m_dir(PORT_DIR_UNKNOWN),m_bEnabled(true),m_peerPort(NULL),m_trackID(0), m_pMirrorTarget(NULL),
	m_bActive(false), m_transMod(STREAMING_NONE), m_pTask(NULL)
{
}

CMediaPort::CMediaPort(CMediaElement *host, PortDirection dir, TRACKID id, bool enable) 
	: m_hostElement(host),m_dir(dir),m_trackID(id),m_bEnabled(enable),m_peerPort(NULL), m_pMirrorTarget(NULL),
	m_bActive(false), m_transMod(STREAMING_NONE), m_pTask(NULL)
{
}

CMediaPort::CMediaPort(CMediaElement *host, PortDirection dir, TRACKID id, bool active, StreamMode mode, bool enable)
	: m_hostElement(host),m_dir(dir),m_trackID(id),m_bEnabled(enable),m_peerPort(NULL), m_pMirrorTarget(NULL),
	m_bActive(active), m_transMod(mode), m_state(TASK_STATE_NONE), m_pTask(NULL)
{
	SetActiveMode(active, mode);
}


CMediaPort::~CMediaPort()
{
	if(m_pTask)
	{
		delete m_pTask;
		m_pTask = NULL;
	}

	m_propLists.clear();
}

void CMediaPort::SetHost(CMediaElement *element)
{
	if(element != NULL)
	{
		m_hostElement = element;
	}
}

CMediaElement* CMediaPort::GetHost()
{
	return m_hostElement;
}

PortDirection CMediaPort::GetDirection()
{
	return m_dir;
}

TRACKID CMediaPort::GetIndex() const
{
	return m_trackID;
}

MediaType CMediaPort::GetMediaType()
{
	MetaData media;
	
	if(GetProperty(META_KEY_MEDIA, media) == MEDIA_ERR_NONE)
	{
		return MediaStrToType(media.mValue);
	}

	return MEDIA_TYPE_NONE;
}


int CMediaPort::LinkPeerPort(CMediaPort *peer)
{
	if(peer == NULL)
	{
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	if(m_peerPort != NULL && m_peerPort == peer)
		return MEDIA_ERR_NONE;

	SetPeerPort(peer);

	m_peerPort->ResetProps();
	CopyPropsToPeer();

	peer->SetPeerPort(this);
	
	return MEDIA_ERR_NONE;
}

void CMediaPort::SetPeerPort(CMediaPort *peer)
{
	m_peerPort = peer;

	if(peer == NULL)
	{
		ResetProps();
	}
}

void CMediaPort::UnLinkPeerPort()
{
	if(m_peerPort != NULL)
	{
		m_peerPort->SetPeerPort(NULL);
	}
	m_peerPort = NULL;
}

CMediaPort* CMediaPort::GetPeerPort()
{
	return m_peerPort;
}

CMediaElement* CMediaPort::GetPeerElement()
{
	if(m_peerPort != NULL)
		return m_peerPort->GetHost();

	return NULL;
}


bool CMediaPort::IsLinked()
{
	return (m_peerPort != NULL);
}

bool CMediaPort::IsEnable()
{
	return m_bEnabled;
}

void CMediaPort::SetEnable(bool enable)
{
	m_bEnabled = enable;
}

void CMediaPort::PushBufferToDownStream(CMediaBuffer *buf)
{
	if(m_dir == PORT_DIR_OUT)
	{
		if(m_pMirrorTarget != NULL)
		{
			m_pMirrorTarget->PushBufferToDownStream(buf);
		}
		else
		{
			m_peerPort->PushBufferToDownStream(buf);
		}
	}
	else
	{
		ElementType type = m_hostElement->GetType();
		if(type >= ELEMENT_TYPE_FILTER)
		{
			//TODO element fill data
			if(type < ELEMENT_TYPE_SINK)
			{
				static_cast<CMediaBaseFilter*>(m_hostElement)->DrainInputBuffer(m_trackID, buf);
			}
			else
			{
				static_cast<CMediaBaseSink*>(m_hostElement)->DrainInputBuffer(m_trackID, buf);
			}
		}
	}

}

void CMediaPort::PullBufferFromUpStream(CMediaBuffer **buf)
{
	if(m_dir == PORT_DIR_OUT)
	{
		ElementType type = m_hostElement->GetType();
		if(type < ELEMENT_TYPE_SINK)
		{
			//TODO element fill data
			if(type < ELEMENT_TYPE_FILTER)
			{
				static_cast<CMediaBaseSrc*>(m_hostElement)->FillOutBuffer(m_trackID, buf);
			}
			else
			{
				static_cast<CMediaBaseFilter*>(m_hostElement)->FillOutBuffer(m_trackID, buf);
			}
		}
	}
	else
	{
		if(m_pMirrorTarget != NULL)
		{
			m_pMirrorTarget->PullBufferFromUpStream(buf);
		}
		else
		{
			m_peerPort->PullBufferFromUpStream(buf);
		}
	}

}



void CMediaPort::ResetProps()
{
	m_propLists.clear();
}

void CMediaPort::GetPropertyDatas(std::vector<MetaData> &datas)
{
	std::map<std::string, MetaData>::iterator ite;
	for(ite = m_propLists.begin(); ite != m_propLists.end(); ite++)
	{
		datas.push_back(ite->second);
	}
}

void CMediaPort::SetProperty(const MetaData &prop)
{
	m_propLists[prop.mKey] = prop;

	if(m_dir == PORT_DIR_IN)
	{
		m_hostElement->PortSettingChanged(m_trackID, prop.mKey);
	}
	else
	{
		if(m_pMirrorTarget != NULL)
		{
			m_pMirrorTarget->SetProperty(prop);
		}
		else if(m_peerPort != NULL)
		{
			m_peerPort->CopyPropFromPeer(prop);
		}
	}

}

int CMediaPort::GetProperty(const std::string &key, MetaData &prop)
{
	std::map<std::string, MetaData>::iterator ite = m_propLists.find(key);
	if(ite != m_propLists.end())
	{
		prop = (*ite).second;
		return MEDIA_ERR_NONE;
	}

	return MEDIA_ERR_NOT_FOUND;
}

int CMediaPort::UpdateProperty(const MetaData &prop)
{
	m_propLists[prop.mKey] = prop;

	if(m_dir == PORT_DIR_IN)
	{
		m_hostElement->PortSettingChanged(m_trackID, prop.mKey);
	}
	else
	{
		if(m_pMirrorTarget != NULL)
		{
			m_pMirrorTarget->UpdateProperty(prop);
		}
		else if(m_peerPort != NULL)
		{
			m_peerPort->UpdateProperty(prop);
		}
	}
	
	return MEDIA_ERR_NONE;
}

void CMediaPort::CopyPropFromPeer(const MetaData &pro)
{
	if(m_dir == PORT_DIR_IN)
	{
		this->SetProperty(pro);
	}
}
/*
int CMediaPort::FindProp(const std::string &key, std::list<std::string> &valList)
{
	std::list<MetaData>::iterator ite = m_propLists.begin();
	for(; ite != m_propLists.end(); ite++)
	{
		MetaData pro = *ite;
		if(pro.mKey == key)
		{
			valList.push_back(pro.mValue);
		}
	}

	if(!valList.empty())
		return MEDIA_ERR_NONE;


	if(ite == m_propLists.end())
		return MEDIA_ERR_NOT_FOUND;

	return MEDIA_ERR_NONE;
}
*/
bool CMediaPort::CheckPropCompatible(CMediaPort *peer)
{
	//TODO

	return false;
}

CMediaPort* CMediaPort::GetMirrorTarget()
{
	return m_pMirrorTarget;
}

void CMediaPort::SetMirrorTarget(CMediaPort *port)
{
	m_pMirrorTarget = port;
	/*
	if(m_pMirrorTarget != NULL)
	{
		std::map<std::string, MetaData>::iterator ite = m_propLists.begin();
		for(; ite != m_propLists.end(); ite++)
		{
			MetaData pro = (*ite).second;
			m_pMirrorTarget->SetProperty(pro);
		}
	}*/
}


void CMediaPort::CopyPropsToPeer()
{
	if(m_peerPort != NULL)
	{
		std::map<std::string, MetaData>::iterator ite = m_propLists.begin();
		for(; ite != m_propLists.end(); ite++)
		{
			MetaData pro = (*ite).second;
			m_peerPort->CopyPropFromPeer(pro);
		}
	}
}



bool CMediaPort::SetActiveMode(bool active, StreamMode mode)
{
	m_state = TASK_STATE_NONE;
	if(active)
	{
		if((m_dir == PORT_DIR_OUT && mode == STREAMING_PUSH)
			|| (m_dir == PORT_DIR_IN && mode == STREAMING_PULL))
		{
			m_pTask = new CSThread(CMediaPort::ThreadFunction, this);
		}
	}
	else
	{
		if(m_pTask != NULL && m_pTask->IsRunning())
		{
			StopTask();

			m_pTask->Kill();
			delete m_pTask;
			m_pTask = NULL;
		}

		m_transMod = STREAMING_NONE;
	}

	return true;
}


bool CMediaPort::StartTask()
{
	if(m_state == TASK_STATE_NONE || m_state == TASK_STATE_STOPED)
	{
		m_pTask->Start();
		m_taskPauseEvent.reset();	
	}
	else if(m_state == TASK_STATE_PAUSED)
	{
		m_taskPauseEvent.post();
	}

	m_state = TASK_STATE_STARTED;
	return true;
}

bool CMediaPort::PauseTask()
{
	m_state = TASK_STATE_PAUSED;

	return true;
}

bool CMediaPort::StopTask()
{
	if(m_state == TASK_STATE_PAUSED)
	{
		m_taskPauseEvent.post();
	}

	m_state = TASK_STATE_STOPED;
	m_pTask->Kill();

	return true;
}


void CMediaPort::ThreadFunction(void *pUserData)
{
	CMediaPort *port = reinterpret_cast<CMediaPort*>(pUserData);
	while(1)
	{
		if(port->m_state == TASK_STATE_STARTED)
		{
			CMediaBuffer *buf = NULL;
			port->PullBufferFromUpStream(&buf);
			if(buf)
			{
				port->PushBufferToDownStream(buf);

				delete buf;
			}

			CSThread::Sleep(1);
		}
		else if(port->m_state == TASK_STATE_PAUSED)
		{
			port->m_taskPauseEvent.wait();
		}
		else if(port->m_state == TASK_STATE_STOPED)
		{
			break;
		}
	}
}