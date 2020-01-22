PROJECT(Server)

# For Server
FIND_PACKAGE(Threads)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(LAPACK REQUIRED)
FIND_PACKAGE(BLAS REQUIRED)

SET(SERVER_INCLUDE_FILES
  Controller.h
  DatasetLoader.h
  dsxdyn.h)

SET(SERVER_SOURCE_FILES
  main.cpp
  Controller.cpp
  DatasetLoader.cpp
  dsxdyn.c)

# Add library includes
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(../lib/hdprocess)

MESSAGE(STATUS "BLAS_LIBRARIES: ${BLAS_LIBRARIES}")
MESSAGE(STATUS "LAPACK_LIBRARIES: ${LAPACK_LIBRARIES}")
MESSAGE(STATUS "ZLIB_LIBRARIES: ${ZLIB_LIBRARIES}")

ADD_EXECUTABLE(dSpaceX ${SERVER_INCLUDE_FILES} ${SERVER_SOURCE_FILES} ${ZLIB_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES (dSpaceX
  base64
  blas
  boostparts
  dl
  hdprocess
  imageutils
  jsoncpp
  pmodels
  dspacex_utils
  libwsserver
  lapack
  pthread
  ${ZLIB_LIBRARIES}
  yaml-cpp)
