#ifndef _MIRROR_PORT_H_
#define _MIRROR_PORT_H_

#include "MediaPort.h"
class CMirrorPort : public CMediaPort
{
public:
	CMirrorPort();
	CMirrorPort(CMediaPort *port);
	CMirrorPort(CMediaPort *port, CMediaElement *host, PortDirection dir, TRACKID id = 0, bool enable = true);
	virtual ~CMirrorPort();

	virtual void PushBufferToDownStream(CMediaBuffer *buf);
	virtual void PullBufferFromUpStream(CMediaBuffer **buf);
	virtual void GetPropertyDatas(std::vector<MetaData> &datas);
	virtual int GetProperty(const std::string &key, MetaData &prop);
	virtual void SetProperty(const MetaData &prop);
	virtual int UpdateProperty(const MetaData &prop);

private:
	void SetBasePort(CMediaPort *port);

private:
	CMediaPort *m_pBasePort;
};


#endif  //_MIRROR_PORT_H_