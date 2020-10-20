#ifndef _FF_MEDIASTORAGE_SINK_H_
#define _FF_MEDIASTORAGE_SINK_H_

#include "../MediaCore/MediaBaseSink.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
}

class CFFMediaStorage : public CMediaBaseSink
{
public:
	CFFMediaStorage();
	CFFMediaStorage(const std::string &name);
	virtual ~CFFMediaStorage();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);
	int Open();
	void Close();
	void AddStream(TRACKID index);
private:
	int m_iPortCnt;
	MediaMetaInfo *m_iMediaInfo[MAX_SIDE_PORT];
	AVFormatContext* m_pOFmtCtx;
};


#endif //_FF_MEDIASTORAGE_SINK_H_