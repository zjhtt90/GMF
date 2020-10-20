cmake_minimum_required( VERSION 2.8.0 )

project ( GMMF C CXX )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	message(STATUS "Use -DCMAKE_BUILD_TYPE=debug to enable debugging")
    set(CMAKE_BUILD_TYPE "Release")
endif()

message ( STATUS "Build type: ${CMAKE_BUILD_TYPE}" )

### binary and library output ###
set (OUTPUT_BASE_PATH "${PROJECT_SOURCE_DIR}/output")
set (EXECUTABLE_OUTPUT_PATH "${OUTPUT_BASE_PATH}/bin")
set (LIBRARY_OUTPUT_PATH "${OUTPUT_BASE_PATH}/lib")

### General compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -W -Wno-write-strings -fpermissive -fPIC")
#check C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" SUPPORTS_CXX11)
if (SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Thrid part tool
set (THIRD_PART_TOOL_DIR "${PROJECT_SOURCE_DIR}/third_part")
set(TINYXML_INCLUDE_DIR ${THIRD_PART_TOOL_DIR}/tinyxml2/include)

include_directories(${PROJECT_SOURCE_DIR} 
					${PROJECT_SOURCE_DIR}/Common
					${PROJECT_SOURCE_DIR}/MediaCore)

add_definitions(EXPORTS)

set (Common_DIR ${PROJECT_SOURCE_DIR}/Common)
set (MediaCore_DIR ${PROJECT_SOURCE_DIR}/MediaCore)
set (Base_DIR ${PROJECT_SOURCE_DIR}/MediaBasePlugin)


set (Common_TARGET GMMF_Common)
set (MediaCore_TARGET GMMF_Core)
set (Base_TARGET GMMF_Base)


add_subdirectory(${Common_DIR})
add_subdirectory(${MediaCore_DIR})
add_subdirectory(${Base_DIR})