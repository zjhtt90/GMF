#include <stdio.h>
#include <stdlib.h>

#include "../MediaCore/Media.h"

#include <windows.h>

int main(int argc, char **argv)
{
	media_core_init();

	CMediaElement *fileSrc = NULL;
	CMediaElement *dupStream = NULL;
	CMediaElement *vidRender1 = NULL;
	CMediaElement *vidRender2 = NULL;
	CMediaElement *vidBin = NULL;
	CMediaElement *pipeline = NULL;

	fileSrc = element_create("FileSrc", "RawFileSrc");
	dupStream = element_create("DupStreamFilter", "dupstream");
	MetaData data(META_KEY_OUTPORT_COUNT, "2", META_DATA_VAL_TYPE_INT);
	element_set_parame(dupStream, data);

	vidRender1 = element_create("SDLVideoRender", "render1");
	vidRender2 = element_create("SDLVideoRender", "render2");

	data = MetaData(META_KEY_MEDIA, MediaTypeToStr(MEDIA_TYPE_VIDEO), META_DATA_VAL_TYPE_STRING);
	element_set_parame(fileSrc, data);
	element_set_parame(dupStream, data);
	data = MetaData(META_KEY_FILE_PATH, "E:\\work\\Walking_1280x720_50_300_1M.yuv", META_DATA_VAL_TYPE_STRING);
	element_set_parame(fileSrc, data);
	data = MetaData(META_KEY_VIDEO_WIDTH, "1280", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(dupStream, data);
	element_set_parame(vidRender1, data);
	element_set_parame(vidRender2, data);
	data = MetaData(META_KEY_VIDEO_HEIGHT, "720", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(dupStream, data);
	element_set_parame(vidRender1, data);
	element_set_parame(vidRender2, data);
	data = MetaData(META_KEY_PIX_FORMAT, "100", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);
	element_set_parame(dupStream, data);
	element_set_parame(vidRender1, data);
	element_set_parame(vidRender2, data);
	data = MetaData(META_KEY_TRUNK_SIZE, "1382400", META_DATA_VAL_TYPE_INT);
	element_set_parame(fileSrc, data);


	data = MetaData(META_KEY_WINDOW_X_POS, "300", META_DATA_VAL_TYPE_INT);
	element_set_parame(vidRender2, data);
	data = MetaData(META_KEY_WINDOW_Y_POS, "0", META_DATA_VAL_TYPE_INT);
	element_set_parame(vidRender2, data);
	//char vidWnd[32] = {0};
	//sprintf_s(vidWnd, "%ld", hWnd);
	//data = MetaData(META_KEY_VIDEO_WINDOW, vidWnd, META_DATA_VAL_TYPE_INT);
	//element_set_parame(vidRender, data);

	element_set_state(vidRender1, MEDIA_ELEMENT_STATE_READY);
	element_set_state(vidRender2, MEDIA_ELEMENT_STATE_READY);
	vidBin = element_create("Bin", "renderBin");
	data = MetaData(META_KEY_ELEMENT_LAYOUT, ELEMENT_LAYOUT_VERTICAL, META_DATA_VAL_TYPE_STRING);
	element_set_parame(vidBin, data);
	bin_add_element(vidBin, vidRender1);
	bin_add_element(vidBin, vidRender2);

	element_set_state(vidBin, MEDIA_ELEMENT_STATE_READY);
	element_set_state(dupStream, MEDIA_ELEMENT_STATE_READY);
	element_set_state(fileSrc, MEDIA_ELEMENT_STATE_READY);

	//element_connect(fileSrc, 0, vidRender, 0);

	pipeline = element_create("Pipeline", "display");
	pipeline_add_element(pipeline, fileSrc);
	pipeline_add_element(pipeline, dupStream);
	pipeline_add_element(pipeline, vidBin);
	pipeline_connect_element(pipeline, fileSrc, dupStream);
	pipeline_connect_element(pipeline, dupStream, vidBin);
	element_set_state(pipeline, MEDIA_ELEMENT_STATE_READY);

	element_set_state(pipeline, MEDIA_ELEMENT_STATE_OPEN);

	while(1)
	{
		Sleep(1000);
	}


	media_core_uninit();
	return 0;
}