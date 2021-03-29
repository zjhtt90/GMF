#ifndef _FF_STREAMING_SRC_H_
#define _FF_STREAMING_SRC_H_

#include "MediaBaseSrc.h"

extern "C"
{
#include <libavformat/avformat.h>
};

class CFFStreamingSrc : public CMediaBaseSrc, public CSThread
{
public:
	CFFStreamingSrc();
	CFFStreamingSrc(const std::string &name);
	virtual ~CFFStreamingSrc();

	virtual int Open();
	virtual void Close();
	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);

	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);
private:
	void Init(const std::string &name);

	virtual void Run();

	int GetMediaInfoByStreamID(int id, MediaMetaInfo** pMediaInfo);

private:
	static short m_eleID;
	AVFormatContext *m_fmtCtx;
	std::string m_strURI;
	int m_oPortCnt;
	MediaMetaInfo *m_oMediaInfo[MAX_SIDE_PORT];
	bool m_bGetKeyFrame;
	unsigned long long m_curPlayTime;
};



class CFFStreamingSrcFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFStreamingSrc();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFStreamingSrc(name);
	}
};




#endif  //_FF_STREAMING_SRC_H_