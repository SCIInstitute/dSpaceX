/*
 *	The WebSocket Transport Header
 *
 */

#ifndef _WST_H_
#define _WST_H_

#include "wstypes.h"


#ifdef __ProtoExt__
#undef __ProtoExt__
#endif
#ifdef __cplusplus
extern "C" {
#define __ProtoExt__
#else
#define __ProtoExt__ extern
#endif

  __ProtoExt__ /*@null@*/ wstContext *wst_createContext( );
  __ProtoExt__ /*@null@*/ wstData *wst_createData( const char *name,
                                                   enum TypedArray type,
                                                   int hlen, const int *header,
                                                   int len, void *array );
  __ProtoExt__ void wst_freeData( wstData *data );
  
  __ProtoExt__ int  wst_startServer( int port, /*@null@*/ char *interface,
                                    /*@null@*/ char *cert_path,
                                    /*@null@*/ char *key_path, int opts,
                                    /*@only@*/ wstContext *WSTcontext );
  
  __ProtoExt__ int  wst_statusServer( int index );
  
  __ProtoExt__ int  wst_nClientServer( int index );
  
  __ProtoExt__ int  wst_activeTextInterfaces( int index, int *Nwsi, void ***wsi );
  
  __ProtoExt__ void wst_killInterface( int index, void *wsi );
  
  __ProtoExt__ int  wst_handShake( wstContext *cntxt );
  
#ifndef STANDALONE
  __ProtoExt__ void wst_sendText( void *wsi, char *text );
  
  __ProtoExt__ void wst_sendData( void *wsi, wstData *data );
#endif
  
  __ProtoExt__ void wst_broadcastText( char *text );
  
  __ProtoExt__ void wst_broadcastData( wstData *data );
  
  __ProtoExt__ void wst_cleanupServers( void );
  
#ifdef __cplusplus
}
#endif

#endif  /*_WST_H_*/
