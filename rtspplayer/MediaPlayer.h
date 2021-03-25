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
	打开URL
	strURL	媒体源URL地址
	hWnd	视频显示窗口
	成功返回0，失败返回-1
	*/
	int Open(const std::string& strURL, void* hWnd);

	/*
	开始播放
	seektime	开始播放的时间位置(单位：秒)，默认从文件开始
	rate		播放速率，默认原始倍速
	*/
	int Play(unsigned long seektime = 0, float rate = 1.0);

	/*
	跳转到指定的时间
	*/
	int Seek(unsigned long seektime);

	/*
	倍速播放（2倍速、4倍速、8倍速、16倍速）
	*/
	int Rate(float rate);

	/*
	暂停播放
	*/
	int Pause();

	/*
	恢复播放
	*/
	int Resume();

	/*
	停止播放
	*/
	int Stop();

	/*
	关闭媒体源
	*/
	int Close();


	/*获取当前播放位置*/
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
	CMediaElement* m_pPipeline; //媒体播放处理管线
	unsigned long long m_curPlayTime; //当前播放时间（s）
	
	CMediaElement* m_pSrc;
	CMediaElement* m_pScaler;
	CMediaElement* m_pVideoRender;
};

}

#endif	//MEDIA_PLAYER_H