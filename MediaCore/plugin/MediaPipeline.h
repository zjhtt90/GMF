#ifndef _MEDIA_PIPELINE_H_
#define _MEDIA_PIPELINE_H_

#include "MediaBin.h"


class CMediaPipeline : public CMediaBin
{
public:
	CMediaPipeline();
	CMediaPipeline(const std::string &name);
	virtual ~CMediaPipeline();

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

	void SetMessageWatcher(MessageWatcher *watcher);

private:
	void Init(const std::string &name);

private:
	static short m_pipeID;

	MessageWatcher *m_pWatcher;
};



class CPipelineFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CMediaPipeline();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CMediaPipeline(name);
	}
};


#endif  //_MEDIA_PIPELINE_H_