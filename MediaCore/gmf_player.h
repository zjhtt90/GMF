#ifndef GMF_MEDIA_PLAYER_H
#define GMF_MEDIA_PLAYER_H

#if defined(_WIN32)
#ifdef EXPORTS
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT __declspec(dllimport)
#endif
#else /* not WIN32 */
#define API_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*log_func)(int level, const char* msg);
typedef struct _gmf_player gmf_player_t;

/************************************************************************/
/*gmf���ʼ��.ȫ�ֵ���һ��
/*int logtype ��־������� 0Ϊ����̨���  2Ϊ�ļ����
/*int loglevel ��־������� 1Ϊdebug 2Ϊinfo 3Ϊwarn 4Ϊerror
/*const char *logpath ��־�ļ�·������ΪNULL
/************************************************************************/
API_EXPORT void gmf_init(int logtype, int loglevel, const char *logpath);
/*gmf�ⷴ��ʼ������Ӧgmf_init*/
API_EXPORT void gmf_uninit();

/*������־����ص�*/
API_EXPORT void gmf_set_log_func(log_func func);

/************************************************************************/
/*����һ��player���
/*name ��player�ı�ʶ���ƣ���ΪNULL
/*pipelineFile ý��ܵ��������ļ���APP��ͨ�������ļ���������ܵ�����ΪNULL������Ĭ�����ô���player
/*����player���
/************************************************************************/
API_EXPORT gmf_player_t* gmf_player_create(const char* name, const char* pipelineFile);

/************************************************************************/
/*����player��� 
/*gmf_player_t* hPlayer player���
/************************************************************************/
API_EXPORT void gmf_player_destory(gmf_player_t* hPlayer);

/************************************************************************/
/*��ý��Դ
/*gmf_player_t* hPlayer player���
/*const char* url ý��Դ�ĵ�ַ����rtsp://127.0.0.1/live/0
/*void* hWnd  ��Ƶ��ʾ�Ĵ��ھ��
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_open(gmf_player_t* hPlayer, const char* url, void* hWnd);

/************************************************************************/
/*ֹͣ���Ų��ر�ý��Դ
/*gmf_player_t* hPlayer player���
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_close(gmf_player_t* hPlayer);

/************************************************************************/
/*��ʼ����
/*gmf_player_t* hPlayer player���
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_play(gmf_player_t* hPlayer);

/************************************************************************/
/*��ͣ����
/*gmf_player_t* hPlayer player���
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_pause(gmf_player_t* hPlayer);

/************************************************************************/
/*�ָ�����
/*gmf_player_t* hPlayer player���
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_resume(gmf_player_t* hPlayer);

/************************************************************************/
/*���ò���λ�ã���Ա����ļ����ţ�
/*gmf_player_t* hPlayer player���
/*unsigned long pos ��תλ��(��λ��s)
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_seek(gmf_player_t* hPlayer, unsigned long pos);

/************************************************************************/
/*���ò������ʣ���Ա����ļ����ţ�
/*gmf_player_t* hPlayer player���
/*float rate ���٣�2���١�4���١�8���١�16���٣�
/*���أ�0 �ɹ���-1 ʧ��
/************************************************************************/
API_EXPORT int gmf_player_rate(gmf_player_t* hPlayer, float rate);

#ifdef __cplusplus
}
#endif

#endif  //GMF_MEDIA_PLAYER_H