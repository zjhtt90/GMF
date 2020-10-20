#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H

#if defined(_WIN32)
#ifdef EXPORTS
#define API_EXPORT __declspec(dllexport)
#else
#define API_EXPORT
#endif
#else /* not WIN32 */
#define API_EXPORT
#endif

#ifdef _cplusplus
extern "C" {
#endif

int API_EXPORT media_server_init(int port, const char* user, const char* password);
void API_EXPORT media_server_add_live(const char* streamName, void* pVSrc, void* pASrc);
void API_EXPORT media_server_run();
int API_EXPORT media_server_uninit();

#ifdef _cplusplus
}
#endif

#endif  //MEDIA_SERVER_H
