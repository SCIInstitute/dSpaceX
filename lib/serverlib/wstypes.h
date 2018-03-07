/*
 *  The WebSocket Transport Structures
 *
 */

#ifndef _WSTYPE_H_
#define _WSTYPE_H_

/* JavaScript Typed Arrays */

enum TypedArray {WST_Int8, WST_Uint8, WST_Uint8Clamped, WST_Int16,
                 WST_Uint16, WST_Int32, WST_Uint32, WST_Float32, WST_Float64};

/*@unused@*/ static int TypedArrayBytes[9] = { 1, 1, 1, 2, 2, 4, 4, 4, 8 };


typedef struct {
  char            *name;         /* identifier */
  enum TypedArray type;          /* type */
  int             hlen;          /* int header length -- min of 1 */
  int             *header;       /* the header */
  int             len;           /* overall length in words */
  void            *data;         /* data stream -- len*Bytes(type) */
} wstData;



typedef struct {
  int             handShake;     /* larger scale handshaking */
  int             ioAccess;      /* IO currently has access */
  int             dataAccess;    /* data routines have access */
  int             sent;          /* send flag */
} wstContext;

#endif  /*_WSTYPE_H_*/
