CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW) # NEW: /full/path/to/lib.so won't break search paths specified by link_directories command
  CMAKE_POLICY(SET CMP0005 OLD) # OLD: don't escape data defs added via add_definitions command
ENDIF(COMMAND CMAKE_POLICY)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    macro(FetchContent_MakeAvailable NAME)
        FetchContent_GetProperties(${NAME})
        if(NOT ${NAME}_POPULATED)
            FetchContent_Populate(${NAME})
            add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
        endif()
    endmacro()
endif()

PROJECT(DSPACEX)

OPTION(BUILD_HDVIZ_IMAGE "Build hdviz for image data (broken)" OFF)
OPTION(BUILD_HDVIZ_CLI "Build HDViz preprocessing tool" OFF)
OPTION(BUILD_HDVIZ_GUI "Build HDViz visualization tool" OFF)
OPTION(BUILD_TESTS "Build tests" ON)
OPTION(BUILD_SERVER_LIB "Builder server lib" ON)
OPTION(BUILD_SERVER "Build server" ON)
OPTION(SHOW_COMPILER_WARNINGS "compiler warnings" OFF)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" "${PROJECT_SOURCE_DIR}/cmake/" "${PROJECT_SOURCE_DIR}/cmake/Modules/")
include(DefaultBuildType)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
if (SHOW_COMPILER_WARNINGS)
  add_definitions("-Wall") # show all warning messages
else()
  add_definitions("-w")    # inhibit all warning messages
endif()

IF(WIN32)
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -/MT")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -/MTd")
ENDIF()
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
  "Single output directory for building all executables.")

INCLUDE_DIRECTORIES(${CMAKE_PREFIX_PATH}/include)

find_package(Eigen3 3.3.7 REQUIRED)
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})
INCLUDE_DIRECTORIES(lib)
INCLUDE_DIRECTORIES(ExternalLibs)
INCLUDE_DIRECTORIES(ExternalLibs/libigl-2.1.0/include)

ADD_SUBDIRECTORY(ExternalLibs/lodepng)
ADD_SUBDIRECTORY(lib/annmod)
ADD_SUBDIRECTORY(ExternalLibs/base64)
ADD_SUBDIRECTORY(lib/hdprocess)
ADD_SUBDIRECTORY(lib/imageutils)
ADD_SUBDIRECTORY(ExternalLibs/jsoncpp)
ADD_SUBDIRECTORY(ExternalLibs/tinyply)
ADD_SUBDIRECTORY(lib/utils)
ADD_SUBDIRECTORY(lib/pmodels)
ADD_SUBDIRECTORY(lib/dspacex)
ADD_SUBDIRECTORY(python)            # dspacex python module

if(BUILD_SERVER_LIB)
  INCLUDE_DIRECTORIES(ExternalLibs/boost)
  ADD_SUBDIRECTORY(ExternalLibs/boost)
  ADD_SUBDIRECTORY(lib/serverlib)
endif()

if(BUILD_SERVER)
  ADD_SUBDIRECTORY(server)
endif()

if(BUILD_HDVIZ_GUI)
  ADD_SUBDIRECTORY(ExternalLibs/gle)
  INCLUDE_DIRECTORIES(ExternalLibs/gle/lib)
  ADD_SUBDIRECTORY(gui)
endif()

if(BUILD_HDVIZ_CLI)
  ADD_SUBDIRECTORY(cli)
endif()

if(BUILD_TESTS)
  ADD_SUBDIRECTORY(test)
endif()
