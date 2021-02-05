#ifndef _FF_VIDEO_SCALER_H_
#define _FF_VIDEO_SCALER_H_

#include "MediaBaseFilter.h"

extern "C"
{
#include "libswscale/swscale.h"
};

class CFFVideoScaler : public CMediaBaseFilter
{
public:
	CFFVideoScaler();
	CFFVideoScaler(const std::string &name);
	virtual ~CFFVideoScaler();

	int Open();
	void Close();
	virtual void SetState(MediaElementState state);
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);
	virtual int FillOutBuffer(TRACKID &id, CMediaBuffer **buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);
private:
	void Init(const std::string &name);

private:
	int m_srcWidth;
	int m_srcHeight;
	int m_srcPixFmt;
	int m_dstWidth;
	int m_dstHeight;
	int m_dstPixFmt;

	SwsContext *m_pSwsCtx;
};


class CFFVideoScalerFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CFFVideoScaler();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CFFVideoScaler(name);
	}
};



#endif  //_VIDEO_SCALER_H_