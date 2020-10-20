set(COMMON_HEADER "Logger.h"
				"LogManager.h"
				"ModuleLoader.h"
				"SEvent.h"
				"SMutex.h"
				"SThread.h"
				"STimer.h"
				"tinyxml2.h"
				"Tree.h"
				"Util.h")	


set(COMMON_SRC "Logger.cpp"
				"ModuleLoader.cpp"
				"SEvent.cpp"
				"SMutex.cpp"
				"STimer.cpp"
				"tinyxml2.cpp"
				"Util.cpp")
				
if (SUPPORTS_CXX11)
	set(COMMON_SRC "SThread_c11.cpp")
else ()
	if (WIN32)
		set(COMMON_SRC "SThread_win.cpp")
	else ()
		set(COMMON_SRC "SThread_linux.cpp")
	endif ()
endif ()

add_library(${Common_TARGET} SHARED ${COMMON_SRC} ${COMMON_HEADER})

target_compile_definitions(${Common_TARGET} PUBLIC TINYXML2_EXPORT)

#target_link_libraries(${Common_TARGET} "-Wl,-Bsymbolic,--no-undefined" "")