#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H
#include <string>

class CMediaElement;
namespace GMF {


class MediaPlayer
{
public:
	MediaPlayer();
	~MediaPlayer();

	static void InitCore();
	static void UninitCore();

	/*
	��URL
	strURL	ý��ԴURL��ַ
	hWnd	��Ƶ��ʾ����
	�ɹ�����0��ʧ�ܷ���-1
	*/
	int Open(const std::string& strURL, void* hWnd);

	/*
	��ʼ����
	seektime	��ʼ���ŵ�ʱ��λ��(��λ����)��Ĭ�ϴ��ļ���ʼ
	rate		�������ʣ�Ĭ��ԭʼ����
	*/
	int Play(unsigned long seektime = 0, float rate = 1.0);

	/*
	��ת��ָ����ʱ��
	*/
	int Seek(unsigned long seektime);

	/*
	���ٲ��ţ�2���١�4���١�8���١�16���٣�
	*/
	int Rate(float rate);

	/*
	��ͣ����
	*/
	int Pause();

	/*
	�ָ�����
	*/
	int Resume();

	/*
	ֹͣ����
	*/
	int Stop();

	/*
	�ر�ý��Դ
	*/
	int Close();


	/*��ȡ��ǰ����λ��*/
	unsigned long long GetCurrentPlayTime() const
	{
		return m_curPlayTime;
	}

private:
	int CreatePipeline();
	int DestroyPipeline();

private:
	std::string m_strURL;
	void* m_hWnd;
	CMediaElement* m_pPipeline; //ý�岥�Ŵ������
	unsigned long long m_curPlayTime; //��ǰ����ʱ�䣨s��
	
	CMediaElement* m_pSrc;
	CMediaElement* m_pScaler;
	CMediaElement* m_pVideoRender;
};

}

#endif	//MEDIA_PLAYER_H