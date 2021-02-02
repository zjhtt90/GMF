#include "FFCameraCapture.h"
#include "FFVideoScaler.h"
#include "FFH264Encoder.h"
#include "FFAACEncoder.h"
#include "FFVideoDecoder.h"
#include "FFAudioResample.h"
#include "FFAudioDecoder.h"
//#include "SDLVideoRender.h"
#include "FFScreenCapture.h"
#include "FFSimpleFilter.h"
#include "FFComplexFilter.h"
#include "FFRTSPServer.h"
#include "LiveRTSPSink.h"

#include "../MediaCore/Plugin.h"

static int plugin_init(CPlugin *plugin)
{
	av_register_all();
	avdevice_register_all();
	avcodec_register_all();
	avfilter_register_all();

	//av_log_set_level(AV_LOG_TRACE);

	plugin->AddElement("FFScreenCapture", new CFFScreenCaptureFactory());
	plugin->AddElement("FFCameraCapture", new CFFCameraCaptureFactory());

	plugin->AddElement("FFSimpleFilter", new CFFSimpleFilterFactory());
	plugin->AddElement("FFComplexFilter", new CFFComplexFilterFactory());

	plugin->AddElement("FFH264Encoder", new CFFH264EncoderFactory());
	plugin->AddElement("FFAACEncoder", new CFFAACEncoderFactory());
	plugin->AddElement("FFVideoScaler", new CFFVideoScalerFactory());

	plugin->AddElement("FFAudioResample", new CFFAudioResampleFactory());
	plugin->AddElement("FFVideoDecoder", new CFFVideoDecoderFactory());
	plugin->AddElement("FFAudioDecoder", new CFFAudioDecoderFactory());

	plugin->AddElement("FFRTSPServer", new CFFRTSPServerFactory());

	plugin->AddElement("LiveRTSPSink", new CLiveRTSPSinkFactory());

	//SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	//plugin->AddElement("SDLVideoRender", new CSDLVideoRenderFactory());

	return MEDIA_ERR_NONE;
}

static void plugin_uninit(CPlugin *plugin)
{
	plugin->RemoveElement("FFScreenCapture");
	plugin->RemoveElement("FFCameraCapture");
	plugin->RemoveElement("FFSimpleFilter");
	plugin->RemoveElement("FFComplexFilter");
	plugin->RemoveElement("FFH264Encoder");
	plugin->RemoveElement("FFAACEncoder");
	plugin->RemoveElement("FFVideoScaler");
	plugin->RemoveElement("FFAudioResample");
	plugin->RemoveElement("FFVideoDecoder");
	plugin->RemoveElement("FFAudioDecoder");
	plugin->RemoveElement("FFRTSPServer");
	plugin->RemoveElement("LiveRTSPSink");

	//plugin->RemoveElement("SDLVideoRender");
	//SDL_QuitSubSystem(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
}

PLUGIN_DEFINE(Base, "Media Base plugin", plugin_init, plugin_uninit)