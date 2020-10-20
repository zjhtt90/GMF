#include "FileSrc.h"
#include "DupStreamFilter.h"
#include "AudioFrameBuffer.h"
#include "FileSink.h"
#ifdef SYSTEM_WIN32
#include "MicphoneCapture.h"
#include "SpeakerCapture.h"
#include "DSoundRender.h"
#include "GDIVideoCapture.h"
#include "D3DVideoRender.h"
#endif
#include "MediaBin.h"
#include "MediaPipeline.h"

#include "../Plugin.h"

static int plugin_init(CPlugin *plugin)
{
	plugin->AddElement("FileSrc", new CFileSrcFactory());
	plugin->AddElement("DupStreamFilter", new CDupStreamFilterFactory());
	plugin->AddElement("AudioFrameBuffer", new CAudioFrameBufferFactory());
	plugin->AddElement("FileSink", new CFileSinkFactory());
#ifdef SYSTEM_WIN32
	plugin->AddElement("MicphoneCapture", new CMicphoneCaptureFactory());
	plugin->AddElement("SpeakerCapture", new CSpeakerCaptureFactory());
	plugin->AddElement("DSoundRender", new CDSoundRenderFactory());
	plugin->AddElement("GDIVideoCapture", new CGDIVideoCaptureFactory());
	plugin->AddElement("D3DVideoRender", new CD3DVideoRenderFactory());
#endif

	plugin->AddElement("Bin", new CMediaBinFactory());
	plugin->AddElement("Pipeline", new CPipelineFactory());

	return MEDIA_ERR_NONE;
}

static void plugin_uninit(CPlugin *plugin)
{
	plugin->RemoveElement("FileSrc");
	plugin->RemoveElement("DupStreamFilter");
	plugin->RemoveElement("AudioFrameBuffer");
	plugin->RemoveElement("FileSink");
#ifdef SYSTEM_WIN32
	plugin->RemoveElement("MicphoneCapture");
	plugin->RemoveElement("SpeakerCapture");
	plugin->RemoveElement("DSoundRender");
	plugin->RemoveElement("GDIVideoCapture");
	plugin->RemoveElement("D3DVideoRender");
#endif

	plugin->RemoveElement("Bin");
	plugin->RemoveElement("Pipeline");
}

PLUGIN_DEFINE_STASIC(Core, "Media Core plugin", plugin_init, plugin_uninit)