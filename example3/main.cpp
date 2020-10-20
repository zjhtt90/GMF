#include <stdio.h>
#include <stdlib.h>

#include "../MediaCore/Media.h"

#include <windows.h>

int main(int argc, char **argv)
{
	media_core_init();

	CMediaElement *fileSrc = NULL;
	CMediaElement *encoder = NULL;
	CMediaElement *filesink = NULL;
	CMediaElement *pipeline = NULL;

	fileSrc = element_create("FileSrc", "RawFileSrc");

	encoder = element_create("H264Encoder", "Encode");

	filesink = element_create("FileSink", "h264file");

	MetaData data(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	element_set_parame(fileSrc, data);
	element_set_parame(encoder, data);
	element_set_parame(filesink, data);

	data = MetaData(META_KEY_FILE_PATH, "E:\\work\\Walking_1280x720_50_300_1M.yuv", META_DATA_VAL_TYPE_STRING);
	element_set_parame(fileSrc, data);

	data = MetaData(META_KEY_VIDEO_WIDTH, "1280", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(encoder, data);

	data = MetaData(META_KEY_VIDEO_HEIGHT, "720", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(encoder, data);

	data = MetaData(META_KEY_PIX_FORMAT, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(encoder, data);

	data = MetaData(META_KEY_TRUNK_SIZE, "1382400", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);


	data = MetaData(META_KEY_FILE_PATH, "11.h264", META_DATA_VAL_TYPE_STRING);
	element_set_parame(filesink, data);


	data = MetaData(META_KEY_MIN_QVALUE, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_MAX_QVALUE, "10", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);
	data = MetaData(META_KEY_CODEC_PRESET, "0", META_DATA_VAL_TYPE_INT);
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
	data = MetaData(META_KEY_CODEC_QUALITY, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(encoder, data);



	element_set_state(fileSrc, MEDIA_ELEMENT_STATE_READY);
	element_set_state(encoder, MEDIA_ELEMENT_STATE_READY);
	element_set_state(filesink, MEDIA_ELEMENT_STATE_READY);

	pipeline = element_create("Pipeline", "encoder");
	pipeline_add_element(pipeline, fileSrc);
	pipeline_add_element(pipeline, encoder);
	pipeline_add_element(pipeline, filesink);
	pipeline_connect_element(pipeline, fileSrc, encoder);
	pipeline_connect_element(pipeline, encoder, filesink);
	element_set_state(pipeline, MEDIA_ELEMENT_STATE_READY);

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_OPEN);

	while(1)
	{
		Sleep(1000);
	}


	media_core_uninit();
	return 0;
}