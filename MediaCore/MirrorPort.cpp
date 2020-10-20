#include "MirrorPort.h"


CMirrorPort::CMirrorPort() : CMediaPort(NULL, PORT_DIR_UNKNOWN),m_pBasePort(NULL)
{
}

CMirrorPort::CMirrorPort(CMediaPort *port) : CMediaPort(NULL, PORT_DIR_UNKNOWN)
{
	SetBasePort(port);
}

CMirrorPort::CMirrorPort(CMediaPort *port, CMediaElement *host, PortDirection dir, TRACKID id, bool enable)
	: CMediaPort(host, dir, id, enable)
{
	SetBasePort(port);
}

CMirrorPort::~CMirrorPort()
{
}

void CMirrorPort::SetBasePort(CMediaPort *port)
{
	m_pBasePort = port;

	port->SetMirrorTarget(this);
}

void CMirrorPort::GetPropertyDatas(std::vector<MetaData> &datas)
{
	if(m_pBasePort != NULL)
	{
		m_pBasePort->GetPropertyDatas(datas);
	}
}

int CMirrorPort::GetProperty(const std::string &key, MetaData &prop)
{
	int res = MEDIA_ERR_NONE;
	if(m_pBasePort != NULL)
	{
		res = m_pBasePort->GetProperty(key, prop);
	}

	return res;
}

void CMirrorPort::SetProperty(const MetaData &prop)
{
	if(m_pBasePort != NULL && m_pBasePort->GetDirection() == PORT_DIR_IN)
	{
		m_pBasePort->SetProperty(prop);
	}
	else
	{
		CMediaPort *target = NULL;
		if((target = GetMirrorTarget()) != NULL)
		{
			target->SetProperty(prop);
		}
		else
		{
			CMediaPort *peer = GetPeerPort();
			if(peer != NULL)
				peer->CopyPropFromPeer(prop);
		}
	}
}

int CMirrorPort::UpdateProperty(const MetaData &prop)
{
	int res = MEDIA_ERR_NONE;
	if(m_pBasePort != NULL && m_pBasePort->GetDirection() == PORT_DIR_IN)
	{
		res = m_pBasePort->UpdateProperty(prop);
	}
	else
	{
		CMediaPort *target = NULL;
		if((target = GetMirrorTarget()) != NULL)
		{
			target->SetProperty(prop);
		}
		else
		{
			CMediaPort *peer = GetPeerPort();
			if(peer != NULL)
				res = peer->UpdateProperty(prop);
		}
	}

	return res;
}

void CMirrorPort::PushBufferToDownStream(CMediaBuffer *buf)
{
	if(GetDirection() == PORT_DIR_OUT)
	{
		CMediaPort *target = NULL;
		if((target = GetMirrorTarget()) != NULL)
		{
			target->PushBufferToDownStream(buf);
		}
		else
		{
			CMediaPort *peer = GetPeerPort();
			if(peer != NULL)
				peer->PushBufferToDownStream(buf);
		}
	}
	else
	{
		m_pBasePort->PushBufferToDownStream(buf);
	}
}

void CMirrorPort::PullBufferFromUpStream(CMediaBuffer **buf)
{
	if(GetDirection() == PORT_DIR_OUT)
	{
		m_pBasePort->PullBufferFromUpStream(buf);
	}
	else
	{
		CMediaPort *peer = GetPeerPort();
		if(peer != NULL)
			peer->PullBufferFromUpStream(buf);
	}
}