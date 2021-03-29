#include "FFCameraCapture.h"
#include "FFStreamingSrc.h"
#include "FFVideoScaler.h"
#include "FFH264Encoder.h"
#include "FFAACEncoder.h"
#include "FFVideoDecoder.h"
#include "FFAudioResample.h"
#include "FFAudioDecoder.h"
#include "FFScreenCapture.h"
#include "FFSimpleFilter.h"
#include "FFComplexFilter.h"
#include "FFRTSPServer.h"

#include "Plugin.h"

static int plugin_init(CPlugin *plugin)
{
	av_register_all();
	avdevice_register_all();
	avcodec_register_all();
	avfilter_register_all();

	//av_log_set_level(AV_LOG_TRACE);

	plugin->AddElement("FFScreenCapture", new CFFScreenCaptureFactory());
	plugin->AddElement("FFCameraCapture", new CFFCameraCaptureFactory());
	plugin->AddElement("FFStreamingSrc", new CFFStreamingSrcFactory());

	plugin->AddElement("FFSimpleFilter", new CFFSimpleFilterFactory());
	plugin->AddElement("FFComplexFilter", new CFFComplexFilterFactory());

	plugin->AddElement("FFH264Encoder", new CFFH264EncoderFactory());
	plugin->AddElement("FFAACEncoder", new CFFAACEncoderFactory());
	plugin->AddElement("FFVideoScaler", new CFFVideoScalerFactory());

	plugin->AddElement("FFAudioResample", new CFFAudioResampleFactory());
	plugin->AddElement("FFVideoDecoder", new CFFVideoDecoderFactory());
	plugin->AddElement("FFAudioDecoder", new CFFAudioDecoderFactory());

	plugin->AddElement("FFRTSPServer", new CFFRTSPServerFactory());

	return MEDIA_ERR_NONE;
}

static void plugin_uninit(CPlugin *plugin)
{
	plugin->RemoveElement("FFScreenCapture");
	plugin->RemoveElement("FFCameraCapture");
	plugin->RemoveElement("FFStreamingSrc");
	plugin->RemoveElement("FFSimpleFilter");
	plugin->RemoveElement("FFComplexFilter");
	plugin->RemoveElement("FFH264Encoder");
	plugin->RemoveElement("FFAACEncoder");
	plugin->RemoveElement("FFVideoScaler");
	plugin->RemoveElement("FFAudioResample");
	plugin->RemoveElement("FFVideoDecoder");
	plugin->RemoveElement("FFAudioDecoder");
	plugin->RemoveElement("FFRTSPServer");
}

PLUGIN_DEFINE(ffmpeg, "Media FFmpeg plugin", plugin_init, plugin_uninit)