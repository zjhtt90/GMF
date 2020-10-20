set(CORE_HEADER "AudioCodec.h"
				"IElementFactory.h"
				"Media.h"
				"MediaBaseFilter.h"
				"MediaBaseSink.h"
				"MediaBaseSrc.h"
				"MediaBuffer.h"
				"MediaBus.h"
				"MediaDefs.h"
				"MediaElement.h"
				"MediaMessage.h"
				"MediaPort.h"
				"MessageBus.h"
				"MirrorPort.h"
				"Plugin.h"
				"PluginManager.h"
				"VideoCodec.h"
				"VideoRender.h"
				"plugin/AudioFrameBuffer.h"
				"plugin/DupStreamFilter.h"
				"plugin/FileSink.h"
				"plugin/FileSrc.h"
				"plugin/MediaBin.h"
				"plugin/MediaPipeline.h")	


set(CORE_SRC "AudioCodec.cpp"
			"Media.cpp"
			"MediaBaseFilter.cpp"
			"MediaBaseSink.cpp"
			"MediaBaseSrc.cpp"
			"MediaBuffer.cpp"
			"MediaBus.cpp"
			"MediaElement.cpp"
			"MediaMessage.cpp"
			"MediaPort.cpp"
			"MessageBus.cpp"
			"MirrorPort.cpp"
			"Plugin.cpp"
			"PluginManager.cpp"
			"VideoCodec.cpp"
			"VideoRender.cpp"
			"plugin/AudioFrameBuffer.cpp"
			"plugin/DupStreamFilter.cpp"
			"plugin/FileSink.cpp"
			"plugin/FileSrc.cpp"
			"plugin/MediaBin.cpp"
			"plugin/MediaPipeline.cpp")
				
if (WIN32)
	set(CORE_HEADER "plugin/D3DVideoRender.h"
				"plugin/DSoundRender.h"
				"plugin/GDIVideoCapture.h"
				"plugin/MicphoneCapture.h"
				"plugin/SpeakerCapture.h")

	set(CORE_SRC "plugin/D3DVideoRender.cpp"
				"plugin/DSoundRender.cpp"
				"plugin/GDIVideoCapture.cpp"
				"plugin/MicphoneCapture.cpp"
				"plugin/SpeakerCapture.cpp")
endif ()

add_library(${MediaCore_TARGET} SHARED ${CORE_SRC} ${CORE_HEADER})

target_compile_definitions(${MediaCore_TARGET} PUBLIC EXPORTS __STDC_CONSTANT_MACROS)

target_link_libraries(${MediaCore_TARGET} "-Wl,-Bsymbolic,--no-undefined" ${Common_TARGET})