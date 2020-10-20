#ifndef _VIDEO_CODEC_H_
#define _VIDEO_CODEC_H_

#include "MediaBaseFilter.h"

class API_EXPORT CVideoCodec : public CMediaBaseFilter
{
public:
	CVideoCodec();
	virtual ~CVideoCodec();

	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void PortSettingChanged(TRACKID index, const std::string &key);
	virtual int ProcessFrame(CMediaBuffer *srcBuf, CMediaBuffer **dstBuf);

protected:
	void ExportCodecInfo(std::vector<MetaData>& infos);
protected:
	int m_codecID;
	int m_width;
	int m_height;
	int m_pixformat;
	int m_srcFps;
	int m_dstFps;
};


#endif  //_VIDEO_CODEC_H_