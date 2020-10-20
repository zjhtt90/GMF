#include <stdio.h>
#include <stdlib.h>

#include "../MediaCore/Media.h"

#include <windows.h>

int main(int argc, char **argv)
{
	MetaData data;

	media_core_init(0,0,NULL);

	CMediaElement *capture = NULL;
	CMediaElement *scaler = NULL;
	CMediaElement *encoder = NULL;
	CMediaElement *filesink = NULL;
	CMediaElement *pipeline = NULL;

	capture = element_create("FFScreenCapture", "DesktopCap");

	//scaler = element_create("VideoScaler", "Scaler");

	//encoder = element_create("H264Encoder", "Encode");

	filesink = element_create("FileSink", "rgbfile");

	//data = MetaData(META_KEY_INPORT_COUNT, "1", META_DATA_VAL_TYPE_INT);
	//element_use_parame(rtspsink, data);


	pipeline = element_create("Pipeline", "encoder");
	pipeline_add_element(pipeline, capture);
	//pipeline_add_element(pipeline, scaler);
	//pipeline_add_element(pipeline, encoder);
	pipeline_add_element(pipeline, filesink);
	pipeline_connect_element(pipeline, capture, filesink);
	//pipeline_connect_element(pipeline, scaler, encoder);
	//pipeline_connect_element(pipeline, encoder, rtspsink);


	//data = MetaData(META_KEY_FILE_PATH, "HD Pro Webcam C920", META_DATA_VAL_TYPE_STRING);
	//element_set_parame(capture, data);
	//data = MetaData(META_KEY_VIDEO_WINDOW, "1181838", META_DATA_VAL_TYPE_INT);
	//element_set_parame(capture, data);
	//AV_PIX_FMT_RGB555LE  46
	//AV_PIX_FMT_YUYV422	1
	data = MetaData(META_KEY_PIX_FORMAT, "27", META_DATA_VAL_TYPE_INT);
	element_set_parame(capture, data);

	data = MetaData(META_KEY_VIDEO_FPS, "30", META_DATA_VAL_TYPE_INT);
	element_set_parame(capture, data);

	/*
	data = MetaData(META_KEY_VIDEO_WIDTH, "1280", META_DATA_VAL_TYPE_INT);
	element_set_parame(scaler, data);
	data = MetaData(META_KEY_VIDEO_HEIGHT, "720", META_DATA_VAL_TYPE_INT);
	element_set_parame(scaler, data);
	data = MetaData(META_KEY_PIX_FORMAT, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(scaler, data);
	*/

	data = MetaData(META_KEY_FILE_PATH, "screen.rgb", META_DATA_VAL_TYPE_STRING);
	element_set_parame(filesink, data);
	/*
	data = MetaData(META_KEY_VIDEO_FPS, "10", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_MIN_QVALUE, "15", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_MAX_QVALUE, "25", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_CODEC_PRESET, "2", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_CODEC_PROFILE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_I_FRAME_INTERVAL, "30", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);

	data = MetaData(META_KEY_BITRATE_TYPE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_CURRENT_BITRATE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_MIN_BITRATE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_MAX_BITRATE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_CODEC_QUALITY, "3", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	*/
	
	element_set_state(pipeline, MEDIA_ELEMENT_STATE_READY);

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_OPEN);

	while(1)
	{
		Sleep(1000);
	}


	media_core_uninit();
	return 0;
}