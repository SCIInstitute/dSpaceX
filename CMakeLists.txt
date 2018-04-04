CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0005 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(DSPACEX)

OPTION(BUILD_HDVIZ_IMAGE "Build hdviz for image data" OFF)
OPTION(BUILD_HDVIZ_CLI "Build HDViz preprocessing tool" OFF)
OPTION(BUILD_HDVIZ_GUI "Build HDViz visualization tool" OFF)
OPTION(BUILD_TESTS "Build tests" ON)
OPTION(BUILD_SERVER_LIB "Builder server lib" ON)
OPTION(BUILD_SERVER "Build server" ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all executables.")

ADD_DEFINITIONS ("-w")

INCLUDE_DIRECTORIES(
  lib/
  lib/hdprocess
  lib/dimred/lib
  lib/annmod/lib
  lib/annmod/lib/ann
  lib/flinalg/commandline/
  ${ANN_INCLUDE_FILES})

ADD_SUBDIRECTORY(lib/annmod/)
ADD_SUBDIRECTORY(lib/jsoncpp)
ADD_SUBDIRECTORY(lib/hdprocess)

if(BUILD_SERVER_LIB)
  ADD_SUBDIRECTORY(lib/serverlib)
endif(BUILD_SERVER_LIB)

if(BUILD_SERVER)
  ADD_SUBDIRECTORY(server)
endif(BUILD_SERVER)

if(BUILD_HDVIZ_GUI)
  ADD_SUBDIRECTORY(lib/gle)
  INCLUDE_DIRECTORIES(lib/gle/lib)
  ADD_SUBDIRECTORY(gui)
endif(BUILD_HDVIZ_GUI)

if(BUILD_HDVIZ_CLI)
  ADD_SUBDIRECTORY(cli)
endif(BUILD_HDVIZ_CLI)

if(BUILD_TESTS)
  INCLUDE(CTest)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)
endif(BUILD_TESTS)
