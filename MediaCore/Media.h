#ifndef _MEDIA_HEADER_
#define _MEDIA_HEADER_

#include "../config.h"
#include "MediaDefs.h"

class CMediaElement;

API_EXPORT void media_core_init(int logtype, int loglevel, const char *logpath);
API_EXPORT void media_core_uninit();

API_EXPORT CMediaElement* element_create(const std::string &factoryName, const std::string &name);
API_EXPORT std::string element_get_name(CMediaElement* element);
API_EXPORT void element_destory(CMediaElement* element);

API_EXPORT bool element_connect(CMediaElement *up, unsigned int outPortIdx, CMediaElement *down, unsigned int inPortIdx);
API_EXPORT void element_disconnect(CMediaElement *up, unsigned int outPortIdx);
API_EXPORT int element_get_parame(CMediaElement *element, MetaData &param);
API_EXPORT void element_set_parame(CMediaElement *element, const MetaData &param);
API_EXPORT void element_use_parame(CMediaElement *element, const MetaData &param);
API_EXPORT void element_set_inport_parame(CMediaElement *element, unsigned int inPortIdx, const MetaData &param);
API_EXPORT void element_set_outport_parame(CMediaElement *element, unsigned int outPortIdx, const MetaData &param);
API_EXPORT void element_set_state(CMediaElement *element, MediaElementState state);
API_EXPORT MediaElementState element_get_state(CMediaElement *element);


API_EXPORT void bin_add_element(CMediaElement *bin,  CMediaElement *element);
API_EXPORT void bin_remove_element(CMediaElement *bin, CMediaElement *element);
API_EXPORT bool bin_connect_element(CMediaElement *bin, CMediaElement *up, CMediaElement *down);
API_EXPORT void bin_disconnect_element(CMediaElement *bin, CMediaElement *up, CMediaElement *down);

API_EXPORT void pipeline_set_watcher(CMediaElement *pipeline,  MessageWatcher *watcher);
API_EXPORT void pipeline_add_element(CMediaElement *pipeline,  CMediaElement *element);
API_EXPORT void pipeline_remove_element(CMediaElement *pipeline, CMediaElement *element);
API_EXPORT bool pipeline_connect_element(CMediaElement *pipeline, CMediaElement *up, CMediaElement *down);
API_EXPORT void pipeline_disconnect_element(CMediaElement *pipeline, CMediaElement *up, CMediaElement *down);


#endif  //_MEDIA_HEADER_