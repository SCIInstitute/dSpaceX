PROJECT(ServerLib)

SET(SERVER_LIB_INCLUDE_FILES 
  extension-deflate-stream.h 
  libwebsockets.h
  extension-x-google-mux.h 
  private-libwebsockets.h 
  wstypes.h
  wst.h)

SET(SERVER_LIB_SOURCE_FILES 
  base64-decode.c 
  handshake.c 
  client-handshake.c 
  libwebsockets.c
  extension-deflate-stream.c
  md5.c 
  extension-x-google-mux.c
  parsers.c 
  extension.c
  sha-1.c 
  server.c 
  wst.c)

if(WIN32)
  list(APPEND SERVER_LIB_INCLUDE_FILES ./win32helpers/websock-w32.h)
  list(APPEND SERVER_LIB_SOURCE_FILES websock-w32.c)
  list(APPEND SERVER_LIB_SOURCE_FILES gettimeofday.c)
  INCLUDE_DIRECTORIES(win32helpers)
endif()

ADD_DEFINITIONS(-DLWS_NO_FORK)


# output library
ADD_LIBRARY(libwsserver ${SERVER_LIB_INCLUDE_FILES} ${SERVER_LIB_SOURCE_FILES})
if(WIN32)
TARGET_LINK_LIBRARIES(libwsserver zlib wsock32 ws2_32)
endif()
