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
/*gmf库初始化.全局调用一次
/*int logtype 日志输出类型 0为控制台输出  2为文件输出
/*int loglevel 日志输出级别 1为debug 2为info 3为warn 4为error
/*const char *logpath 日志文件路径，可为NULL
/************************************************************************/
API_EXPORT void gmf_init(int logtype, int loglevel, const char *logpath);
/*gmf库反初始化，对应gmf_init*/
API_EXPORT void gmf_uninit();

/*设置日志输出回调*/
API_EXPORT void gmf_set_log_func(log_func func);

/************************************************************************/
/*创建一个player句柄
/*name 是player的标识名称，可为NULL
/*pipelineFile 媒体管道的描述文件，APP可通过描述文件创建处理管道。若为NULL，则按照默认配置创建player
/*返回player句柄
/************************************************************************/
API_EXPORT gmf_player_t* gmf_player_create(const char* name, const char* pipelineFile);

/************************************************************************/
/*销毁player句柄 
/*gmf_player_t* hPlayer player句柄
/************************************************************************/
API_EXPORT void gmf_player_destory(gmf_player_t* hPlayer);

/************************************************************************/
/*打开媒体源
/*gmf_player_t* hPlayer player句柄
/*const char* url 媒体源的地址。如rtsp://127.0.0.1/live/0
/*void* hWnd  视频显示的窗口句柄
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_open(gmf_player_t* hPlayer, const char* url, void* hWnd);

/************************************************************************/
/*停止播放并关闭媒体源
/*gmf_player_t* hPlayer player句柄
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_close(gmf_player_t* hPlayer);

/************************************************************************/
/*开始播放
/*gmf_player_t* hPlayer player句柄
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_play(gmf_player_t* hPlayer);

/************************************************************************/
/*暂停播放
/*gmf_player_t* hPlayer player句柄
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_pause(gmf_player_t* hPlayer);

/************************************************************************/
/*恢复播放
/*gmf_player_t* hPlayer player句柄
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_resume(gmf_player_t* hPlayer);

/************************************************************************/
/*设置播放位置（针对本地文件播放）
/*gmf_player_t* hPlayer player句柄
/*unsigned long pos 跳转位置(单位：s)
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_seek(gmf_player_t* hPlayer, unsigned long pos);

/************************************************************************/
/*设置播放速率（针对本地文件播放）
/*gmf_player_t* hPlayer player句柄
/*float rate 倍速（2倍速、4倍速、8倍速、16倍速）
/*返回：0 成功，-1 失败
/************************************************************************/
API_EXPORT int gmf_player_rate(gmf_player_t* hPlayer, float rate);

#ifdef __cplusplus
}
#endif

#endif  //GMF_MEDIA_PLAYER_H