#ifndef _MEDIA_PORT_H_
#define _MEDIA_PORT_H_

#include "MediaDefs.h"
#include "../config.h"

#include "../Common/SThread.h"
#include "../Common/SEvent.h"

#include <string>
#include <map>


enum PortDirection {
	PORT_DIR_UNKNOWN,
	PORT_DIR_IN,
	PORT_DIR_OUT
};

enum StreamMode {
	STREAMING_NONE = 0,
	STREAMING_PUSH,
	STREAMING_PULL
};

enum PortTaskState {
	TASK_STATE_NONE = 0,
	TASK_STATE_STARTED,
	TASK_STATE_PAUSED,
	TASK_STATE_STOPED
};

class CMediaElement;
class CMediaBuffer;


class API_EXPORT CMediaPort
{
public:
	CMediaPort();
	CMediaPort(CMediaElement *host, PortDirection dir, TRACKID id = 0, bool enable = true);
	CMediaPort(CMediaElement *host, PortDirection dir, TRACKID id, bool active, StreamMode mode, bool enable = true);
	virtual ~CMediaPort();

	void SetHost(CMediaElement *element);
	CMediaElement* GetHost();
	PortDirection	GetDirection();
	TRACKID GetIndex() const;
	MediaType GetMediaType();

	int LinkPeerPort(CMediaPort *peer);
	void SetPeerPort(CMediaPort *peer);
	void UnLinkPeerPort();
	CMediaPort* GetPeerPort();
	CMediaElement* GetPeerElement();
	bool IsLinked();

	bool IsEnable();
	void SetEnable(bool enable);
	virtual void GetPropertyDatas(std::vector<MetaData> &datas);
	virtual void SetProperty(const MetaData &prop);
	virtual int GetProperty(const std::string &key, MetaData &prop);
	virtual int UpdateProperty(const MetaData &prop);
	void CopyPropFromPeer(const MetaData &pro);

	virtual void PushBufferToDownStream(CMediaBuffer *buf);
	virtual void PullBufferFromUpStream(CMediaBuffer **buf);

	//int FindProp(const std::string &key,std::list<std::string> &valList);
	bool CheckPropCompatible(CMediaPort *peer);

	CMediaPort* GetMirrorTarget();
	void SetMirrorTarget(CMediaPort *port);

	bool SetActiveMode(bool active, StreamMode mode);
	bool StartTask();
	bool PauseTask();
	bool StopTask();



private:
	void ResetProps();
	void CopyPropsToPeer();

	static void ThreadFunction(void *pUserData);
	
private:
	CMediaElement *m_hostElement;	//所属对象
	PortDirection m_dir;	//端口类型
	CMediaPort *m_peerPort;	//与之相连的对端端口
	std::map<std::string, MetaData> m_propLists;	//接口对应的媒体属性列表
	bool m_bEnabled;	//是否可用	
	TRACKID m_trackID;
	CMediaPort *m_pMirrorTarget;

	bool m_bActive;
	StreamMode m_transMod;
	CSThread *m_pTask;
	PortTaskState m_state;
	CSEvent m_taskPauseEvent;
};

#endif  //_MEDIA_PORT_H_