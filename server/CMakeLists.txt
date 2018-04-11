PROJECT(Server)

# For Server
FIND_PACKAGE(Threads)
FIND_PACKAGE(ZLIB REQUIRED)

IF (${CMAKE_VERSION} VERSION_GREATER "3.0.0") 
  CMAKE_POLICY(SET CMP0042 OLD)
ENDIF()


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

ADD_EXECUTABLE(dSpaceX ${SERVER_INCLUDE_FILES} ${SERVER_SOURCE_FILES} ${ZLIB_INCLUDE_DIRS}) 
TARGET_LINK_LIBRARIES (dSpaceX 
  libwsserver 
  hdprocess   
  blas 
  lapack 
  pthread 
  dl 
  ${ZLIB_LIBRARIES} 
  yaml-cpp 
  jsoncpp)