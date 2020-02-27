CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW) # NEW: /full/path/to/lib.so won't break search paths specified by link_directories command
  CMAKE_POLICY(SET CMP0005 OLD) # OLD: don't escape preprocessor defs added via add_definitions command
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

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
IF(WIN32)
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -/MT")
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -/MTd")
ENDIF()
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/" "${PROJECT_SOURCE_DIR}/cmake/" "${PROJECT_SOURCE_DIR}/cmake/Modules/")
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
  "Single output directory for building all executables.")

include(DefaultBuildType)
  
ADD_DEFINITIONS ("-w")

INCLUDE_DIRECTORIES(lib)
INCLUDE_DIRECTORIES(lib/yaml-cpp/include)

ADD_SUBDIRECTORY(lib/annmod/)
ADD_SUBDIRECTORY(lib/base64/)
ADD_SUBDIRECTORY(lib/hdprocess)
ADD_SUBDIRECTORY(lib/imageutils)
ADD_SUBDIRECTORY(lib/jsoncpp)
ADD_SUBDIRECTORY(lib/tinyply)
ADD_SUBDIRECTORY(lib/yaml-cpp)
ADD_SUBDIRECTORY(lib/utils)
ADD_SUBDIRECTORY(lib/export)
if(1)
  # libigl
  # <ctc> setting these only works if we're building libigl, not for find_package
  # set(LIBIGL_EIGEN_VERSION 3.3.7 CACHE STRING "Eigen version")
  # set(LIBIGL_WITH_PNG ON CACHE BOOL "Libigl w/ PNG support")
  # find_package(LIBIGL REQUIRED QUIET)
  # ...and since right now we're only using Eigen, let's postpone installing igl
  find_package(Eigen3 3.3.7 REQUIRED)
  #message(STATUS "EIGEN3_INCLUDE_DIR outside find_package is " ${EIGEN3_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(EIGEN3_INCLUDE_DIR)
  
  ADD_SUBDIRECTORY(lib/lodepng)
  ADD_SUBDIRECTORY(lib/pmodels)
endif()

if(BUILD_SERVER_LIB)
  INCLUDE_DIRECTORIES(lib/boost)
  ADD_SUBDIRECTORY(lib/boost)
  ADD_SUBDIRECTORY(lib/serverlib)
endif()

if(BUILD_SERVER)
  ADD_SUBDIRECTORY(server)
endif()

if(BUILD_HDVIZ_GUI)
  ADD_SUBDIRECTORY(lib/gle)
  INCLUDE_DIRECTORIES(lib/gle/lib)
  ADD_SUBDIRECTORY(gui)
endif()

if(BUILD_HDVIZ_CLI)
  ADD_SUBDIRECTORY(cli)
endif()

if(BUILD_TESTS)
  INCLUDE(CTest)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(test)
endif()
