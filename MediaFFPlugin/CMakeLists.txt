set(BASE_HEADER "FFAACEncoder.h"
				"FFCameraCapture.h"
				"FFComplexFilter.h"
				"FFH264Encoder.h"
				"FFMediaStorage.h"
				"FFScreenCapture.h"
				"FFSimpleFilter.h"
				"FFVideoScaler.h"
				"LiveRTSPSink.h"
				"SDLVideoRender.h")	


set(BASE_SRC 	"BasePlugin.cpp"
				"FFAACEncoder.cpp"
				"FFCameraCapture.cpp"
				"FFComplexFilter.cpp"
				"FFH264Encoder.cpp"
				"FFMediaStorage.cpp"
				"FFScreenCapture.cpp"
				"FFSimpleFilter.cpp"
				"FFVideoScaler.cpp"
				"LiveRTSPSink.cpp"
				"SDLVideoRender.cpp")

## Find ffmpeg
find_package(FFMPEG)
if (FFMPEG_FOUND)
	message (STATUS "Find ffmpeg packge: ${FFMPEG_INCLUDE_DIR}   ${FFMPEG_LIBRARIES}")
else ()
	message (FATAL_ERROR "Can not find ffmpeg")
endif ()


add_library(${Base_TARGET} SHARED ${BASE_SRC} ${BASE_HEADER})

target_compile_definitions(${Base_TARGET} PUBLIC EXPORTS __STDC_CONSTANT_MACROS)

target_link_libraries(${Base_TARGET} "-Wl,-Bsymbolic,--no-undefined" ${MediaCore_TARGET} ${FFMPEG_LIBRARIES} ${Common_TARGET})