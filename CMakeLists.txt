CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(DSPACEX)

OPTION(BUILD_HDVIZ_IMAGE "Build hdviz for image data" OFF)
OPTION(BUILD_HDVIZPROCESSING "Build HDViz preprocessing tool" OFF)
OPTION(BUILD_HDVIZ "Build HDViz visualizatoin tool" OFF)
OPTION(BUILD_TESTS "Build tests" ON)
OPTION(BUILD_SERVER_LIB "Builder server lib" ON)
OPTION(BUILD_SERVER "Build server" ON)



SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all executables.")

ADD_DEFINITIONS ("-w")

INCLUDE_DIRECTORIES(
  include/
  lib/analysis
  external/
  external/gle/lib 
  external/dimred/lib 
  external/graph/lib 
  external/imageutils/lib 
  external/morsesmale/lib 
  external/morsesmale/external/annmod/lib 
  external/morsesmale/external/annmod/lib/ann 
  external/morsesmale/external/utils/lib 
  external/morsesmale/external/kernelstats/lib 
  external/morsesmale/external/kernelstats/external/metrics/lib 
  external/morsesmale/external/kernelstats/external/flinalg/lib 
  external/morsesmale/external/kernelstats/external/flinalg/commandline/
  ${ANN_INCLUDE_FILES} 
)


ADD_SUBDIRECTORY(external/morsesmale/external/annmod/)
ADD_SUBDIRECTORY(lib/analysis)
if(BUILD_SERVER_LIB)
  ADD_SUBDIRECTORY(lib/serverlib)
endif(BUILD_SERVER_LIB)

if(BUILD_HDVIZ)
  ADD_SUBDIRECTORY(external/gle)
  ADD_SUBDIRECTORY(gui)
endif(BUILD_HDVIZ)

if(BUILD_HDVIZPROCESSING)
  ADD_SUBDIRECTORY(cli)
endif(BUILD_HDVIZPROCESSING)

if(BUILD_TESTS)
  INCLUDE(CTest)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)
endif(BUILD_TESTS)