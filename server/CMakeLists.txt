PROJECT(Server)

# For Server
FIND_PACKAGE(Threads)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(LAPACK REQUIRED)
FIND_PACKAGE(BLAS REQUIRED)
find_package(pybind11 REQUIRED)

SET(SERVER_INCLUDE_FILES
  Controller.h
  dsxdyn.h)

SET(SERVER_SOURCE_FILES
  server.cpp
  Controller.cpp
  dsxdyn.c)

ADD_EXECUTABLE(dspacex_server ${SERVER_INCLUDE_FILES} ${SERVER_SOURCE_FILES})
target_include_directories(dspacex_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)
TARGET_LINK_LIBRARIES(dspacex_server
  base64
  ${BLAS_LIBRARIES}
  pybind11::embed
  boostparts
  dl
  hdprocess
  imageutils
  jsoncpp
  pmodels
  dspacex_utils
  dataset
  libwsserver
  ${LAPACK_LIBRARIES}
  pthread
  ${ZLIB_LIBRARIES}
  )

set_target_properties(dspacex_server PROPERTIES
  OUTPUT_NAME dSpaceX
  )

