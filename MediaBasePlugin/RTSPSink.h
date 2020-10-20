#ifndef _RTSP_SINK_H_
#define _RTSP_SINK_H_

#include "../MediaCore/MediaBaseSink.h"

#include "h_xtmediaserver.h"

#define MEDIA_SEVICE_CHANNELS	3

struct RTSPTrait
{
	unsigned short snd_port_start; // ������ʼ�˿�
	bool is_demux; // ��·����
	std::string mul_ip_start; // �鲥��ʼ��ַ
	unsigned short msg_listen_port; // ˽�м����˿�
	unsigned short rtsp_listen_port; // rtsp�����˿�
	unsigned short tcp_listen_port; // tcp����������Ͷ˿�
	unsigned short udp_listen_port; // udp�����˿�
	bool is_std_rtp; // �Ƿ��ͱ�׼��
	bool is_single_sink; // �Ƿ�һת��
	bool use_traffic_shaping; // �Ƿ�����������
	bool is_resend; // �Ƿ񶪰��ش�

	RTSPTrait() : snd_port_start(40001), is_demux(false), mul_ip_start("239.0.0.1"),
		msg_listen_port(20001), rtsp_listen_port(1554), tcp_listen_port(20000),
		udp_listen_port(19900), is_std_rtp(true), is_single_sink(false), use_traffic_shaping(false),
		is_resend(false)
	{}
};

class CRTSPSink : public CMediaBaseSink
{
public:
#ifdef _USE_RTP_SEND_CONTROLLER
	struct NetworkCostItem
	{
		CRTSPSink *rtsp;
		bool is_got; // �Ƿ��Ѿ�ȡ�ò���
		time_t got_ts; // ȡ�õ�ʱ��
		unsigned int suggested_bitrate; // ��������
		unsigned int fraction_lost; // ������, ��Χ0-255
		unsigned int rtt; // �����ӳ�ʱ��(����)

		NetworkCostItem() : rtsp(NULL), is_got(false), got_ts(0), suggested_bitrate(0),
			fraction_lost(0), rtt(0)
		{}
	};
#endif


	CRTSPSink();
	CRTSPSink(const std::string &name);
	virtual ~CRTSPSink();

	virtual void UseParame(const MetaData &data);
	virtual void SetState(MediaElementState state);
	virtual int Open();
	virtual void Close();
	virtual void DrainInputBuffer(TRACKID id, CMediaBuffer *buffer);

	virtual void PortSettingChanged(TRACKID index, const std::string &key);

	virtual void MessageHandle(void *sender, const MediaMessage &msg);

private:
	void Init(const std::string &name);

	void GetMediaServerCfg(MS_CFG &ms_cfg);
	void CreateSrc();
	void SetupHeartbeatCheck();
	void SetSdp();


	// XTMediaServer�ص�����
	static void MEDIASERVER_STDCALL WriteMediaServerLog(char* logname, log_level_type level,
		char* log_ctx, uint32_t log_ctx_len);
	static int MEDIASERVER_STDCALL WriteLinkStateEvent(const LINK_STATE_EVENT evnt, const int srno);
	static int MEDIASERVER_STDCALL WriteRtspPlay(int srcno, int trackid, long chid,  double npt,
		float scale, uint32_t *rtp_pkt_seq,  uint32_t *rtp_pkt_timestamp);
	static int MEDIASERVER_STDCALL WriteRtspPause(int srcno, int trackid, long chid);
	static int MEDIASERVER_STDCALL WriteTcpPlay(int srcno, long chid, double npt, float scale,
		uint32_t *rtp_pkt_timestamp);
	static int MEDIASERVER_STDCALL WriteTcpPause(int srcno, long chid);
	static void MEDIASERVER_STDCALL RtcpForceIframe(const int srcno);
#ifdef _USE_RTP_SEND_CONTROLLER
	static void MEDIASERVER_STDCALL NetworkChanged(void *ctx, uint32_t bitrate,
		uint32_t fraction_lost, uint32_t rtt);
#endif

private:
	RTSPTrait m_cfg;  // RTSP���ò���
	int m_channelCnt;  // ͨ����
	int	m_eMdieaServerAlive;
	int m_srcno[MEDIA_SEVICE_CHANNELS];
#ifdef _USE_RTP_SEND_CONTROLLER
	struct NetworkCostItem m_network_cost[MEDIA_SEVICE_CHANNELS];
#endif
	unsigned int m_vCount; // ��Ƶ֡������
	unsigned int m_aCount; // ��Ƶ֡������

	int m_nDataType;

	int m_sdpMediaCnt;
	std::string m_baseSDP;
	std::string m_aSDP;
	std::string m_vSDP;

	int m_sampleRate;
};



class CRTSPSinkFactory : public IElementFactory
{
public:
	virtual CMediaElement* CreateElement()
	{
		return new CRTSPSink();
	}

	virtual CMediaElement* CreateElement(const std::string &name)
	{
		return new CRTSPSink(name);
	}
};


#endif  //_RTSP_SINK_H_