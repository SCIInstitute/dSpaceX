PROJECT(ServerLib)

SET(SERVER_LIB_INCLUDE_FILES 
  extension-deflate-stream.h 
  libwebsockets.h
  extension-x-google-mux.h 
  private-libwebsockets.h 
  ../../include/wstypes.h
  ../../include/wst.h)

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

ADD_DEFINITIONS(-DLWS_NO_FORK)


# output library
ADD_LIBRARY(libwsserver ${SERVER_LIB_INCLUDE_FILES} ${SERVER_LIB_SOURCE_FILES})
