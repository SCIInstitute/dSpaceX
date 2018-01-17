/*
 *      The Web Viewer
 *
 *             FORTRAN Bindings
 *
 *      Copyright 2011-2018, Massachusetts Institute of Technology
 *      Licensed under The GNU Lesser General Public License, version 2.1
 *      See http://www.opensource.org/licenses/lgpl-2.1.php
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libwebsockets.h"
#include "wsserver.h"

#define INT8 unsigned long long

#ifdef WIN32
  extern void BROWSERMESSAGE ( INT8 *wsi, char *test, int textLen );
#else
  extern void browsermessage_( INT8 *wsi, char *test, int textLen );
#endif



static /*@null@*/ char *IV_f2c(const char *name, int nameLen)
{
  char *string;
  int  i, len;

  for (len = nameLen; len >= 1; len--) if (name[len-1] != ' ') break;
  if (len == 0) return NULL;
  string = (char *) malloc((len+1)*sizeof(char));
  if (string != NULL) {
    for (i = 0; i < len; i++) string[i] = name[i];
    string[len] = 0;
  }
  return string;
}


int
#ifdef WIN32
IV_USLEEP (int *micsec)
#else
iv_usleep_(int *micsec)
#endif
{
  int ret;

#ifdef WIN32
  usleep(*micsec);
  ret = *micsec;
#else
  ret = usleep(*micsec);
#endif
  return ret;
}


void
#ifdef WIN32
IV_CREATECONTEXT (int *bias, float *fov, float *zNear, float *zFar, float *eye,
                  float *center, float *up, INT8 *cntxt)
#else
iv_createcontext_(int *bias, float *fov, float *zNear, float *zFar, float *eye,
                  float *center, float *up, INT8 *cntxt)
#endif
{
  wvContext *context;

  context = wv_createContext(*bias, *fov, *zNear, *zFar, eye, center, up);
  *cntxt  = (INT8) context;
}


int
#ifdef WIN32
IV_STATUSSERVER (int *index)
#else
iv_statusserver_(int *index)
#endif
{
  return wv_statusServer(*index);
}


int
#ifdef WIN32
IV_NCLIENTSERVER (int *index)
#else
iv_nclientserver_(int *index)
#endif
{
  return wv_nClientServer(*index);
}


int
#ifdef WIN32
IV_ACTIVEINTERFACES (int *index, int *nwsi, INT8 *fwsi)
#else
iv_activeinterfaces_(int *index, int *nwsi, INT8 *fwsi)
#endif
{
  int  i, n, stat;
  void **wsi;
  
  n    = *nwsi;
  stat = wv_activeInterfaces(*index, nwsi, &wsi);
  if (stat != 0) return stat;
  if (*nwsi > n) return -999;
  for (i = 0; i < *nwsi; i++) fwsi[i]  = (INT8) wsi[i];
  return stat;
}

void
#ifdef WIN32
IV_KILLINTERFACE (int *index, INT8 *fwsi)
#else
iv_killinterface_(int *index, INT8 *fwsi)
#endif
{
  void *wsi;
  
  wsi = (void *) *fwsi;
  wv_killInterface(*index, wsi);
}


int
#ifdef WIN32
IV_STARTSERVER (int *port, char *interf, char *path, char *key, int *opts,
                INT8 *cntxt, int interfLen, int pathLen, int keyLen)
#else
iv_startserver_(int *port, char *interf, char *path, char *key, int *opts,
                INT8 *cntxt, int interfLen, int pathLen, int keyLen)
#endif
{
  int       stat;
  char      *interface, *cert_path, *key_path;
  wvContext *context;

  interface = IV_f2c(interf, interfLen);
  cert_path = IV_f2c(path,   pathLen);
  key_path  = IV_f2c(key,    keyLen);
  context   = (wvContext *) *cntxt;

  stat = wv_startServer(*port, interface, cert_path, key_path, *opts, context);
  if (interface != NULL) free(interface);
  if (cert_path != NULL) free(cert_path);
  if (key_path  != NULL) free(key_path);
  return stat;
}


void
#ifdef WIN32
IV_CLEANUPSERVERS ()
#else
iv_cleanupservers_()
#endif
{
  wv_cleanupServers();
}


int
#ifdef WIN32
IV_HANDSHAKE (INT8 *cntxt)
#else
iv_handshake_(INT8 *cntxt)
#endif
{
  wvContext *context;
  
  context = (wvContext *) *cntxt;
  return wv_handShake(context);
}


void
#ifdef WIN32
IV_REMOVEALL (INT8 *cntxt)
#else
iv_removeall_(INT8 *cntxt)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  wv_removeAll(context);
}


void
#ifdef WIN32
IV_REMOVEGPRIM (INT8 *cntxt, int *index)
#else
iv_removegprim_(INT8 *cntxt, int *index)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  wv_removeGPrim(context, *index);
}


void
#ifdef WIN32
IV_PRINTGPRIM (INT8 *cntxt, int *index)
#else
iv_printgprim_(INT8 *cntxt, int *index)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  wv_printGPrim(context, *index);
}


void
#ifdef WIN32
IV_SETPSIZE (INT8 *cntxt, int *index, float *psize)
#else
iv_setpsize_(INT8 *cntxt, int *index, float *psize)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  if (context == NULL) return;
  if (context->gPrims == NULL) return;
  if ((*index >= context->nGPrim) || (*index < 0)) return;

  context->gPrims[*index].pSize = *psize;
}


void
#ifdef WIN32
IV_SETLWIDTH (INT8 *cntxt, int *index, float *lwidth)
#else
iv_setlwidth_(INT8 *cntxt, int *index, float *lwidth)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  if (context == NULL) return;
  if (context->gPrims == NULL) return;
  if ((*index >= context->nGPrim) || (*index < 0)) return;

  context->gPrims[*index].lWidth = *lwidth;
}


int
#ifdef WIN32
IV_INDEXGPRIM (INT8 *cntxt, char *name, int nameLen)
#else
iv_indexgprim_(INT8 *cntxt, char *name, int nameLen)
#endif
{
  int       stat;
  char      *gname;
  wvContext *context;

  gname = IV_f2c(name, nameLen);
  if (gname == NULL) return -1;
  context = (wvContext *) *cntxt;
  stat    = wv_indexGPrim(context, gname);
  free(gname);
  return stat;
}


int
#ifdef WIN32
IV_ADDGPRIM (INT8 *cntxt, char *name, int *gtype, int *attrs, int *nitems,
             INT8 *items, int nameLen)
#else
iv_addgprim_(INT8 *cntxt, char *name, int *gtype, int *attrs, int *nitems,
             INT8 *items, int nameLen)
#endif
{
  int       i, stat;
  char      *gname;
  wvContext *context;
  wvData    *gitems, *item;

  if (*nitems <= 0) return -5;
  gname = IV_f2c(name, nameLen);
  if (gname == NULL) return -1;
  context = (wvContext *) *cntxt;
  gitems  = (wvData *) malloc(*nitems*sizeof(wvData));
  if (gitems == NULL) {
    free(gname);
    return -1;
  }
  for (i = 0; i < *nitems; i++) {
    item = (wvData *) items[i];
    gitems[i].dataType = item->dataType;
    gitems[i].dataLen  = item->dataLen;
    gitems[i].dataPtr  = item->dataPtr;
    gitems[i].data[0]  = item->data[0];
    gitems[i].data[1]  = item->data[1];
    gitems[i].data[2]  = item->data[2];
    free(item);
    items[i] = 0;
  }
  stat = wv_addGPrim(context, gname, *gtype, *attrs, *nitems, gitems);
  free(gitems);
  free(gname);
  return stat;
}


int
#ifdef WIN32
IV_MODGPRIM (INT8 *cntxt, int *index, int *nitems, INT8 *items)
#else
iv_modgprim_(INT8 *cntxt, int *index, int *nitems, INT8 *items)
#endif
{
  int       i, stat;
  wvContext *context;
  wvData    *gitems, *item;

  if (*nitems <= 0) return -5;
  context = (wvContext *) *cntxt;
  gitems  = (wvData *) malloc(*nitems*sizeof(wvData));
  if (gitems == NULL) return -1;
  for (i = 0; i < *nitems; i++) {
    item = (wvData *) items[i];
    gitems[i].dataType = item->dataType;
    gitems[i].dataLen  = item->dataLen;
    gitems[i].dataPtr  = item->dataPtr;
    gitems[i].data[0]  = item->data[0];
    gitems[i].data[1]  = item->data[1];
    gitems[i].data[2]  = item->data[2];
    free(item);
    items[i] = 0;
  }
  stat = wv_modGPrim(context, *index, *nitems, gitems);
  free(gitems);
  return stat;
}


int
#ifdef WIN32
IV_SETDATA (int *type, int *len, void *ptr, int *vtype, INT8 *item)
#else
iv_setdata_(int *type, int *len, void *ptr, int *vtype, INT8 *item)
#endif
{
  int    stat;
  wvData *ditem;

  *item = 0;
  ditem = (wvData *) malloc(sizeof(wvData));
  if (ditem == NULL) return -1;
  stat  = wv_setData(*type, *len, ptr, *vtype, ditem);
  if (stat == 0) {
    *item = (INT8) ditem;
  } else {
    free(ditem);
  }
  return stat;
}


int
#ifdef WIN32
IV_ADDARROWHEADS (INT8 *cntxt, int *index, float *size, int *nHeads, int *heads)
#else
iv_addarrowheads_(INT8 *cntxt, int *index, float *size, int *nHeads, int *heads)
#endif
{
  wvContext *context;

  context = (wvContext *) *cntxt;
  return wv_addArrowHeads(context, *index, *size, *nHeads, heads);
}


int
#ifdef WIN32
IV_SETKEY (INT8 *cntxt, int *nCol, float *colors, float *beg, float *end,
           char *title, int titleLen)
#else
iv_setkey_(INT8 *cntxt, int *nCol, float *colors, float *beg, float *end,
           char *title, int titleLen)
#endif
{
  int       stat;
  char      *ititle;
  wvContext *context;
  
  ititle = IV_f2c(title, titleLen);
  if (ititle == NULL) return -1;
  context = (wvContext *) *cntxt;
  
  stat = wv_setKey(context, *nCol, colors, *beg, *end, ititle);
  free(ititle);
  return stat;
}


void
#ifdef WIN32
IV_BROADCASTTEXT (char *text, int textLen)
#else
iv_broadcasttext_(char *text, int textLen)
#endif
{
  char *name;

  name = IV_f2c(text, textLen);
  if (name == NULL) return;
  wv_broadcastText(name);
  free(name);
}


void
#ifdef WIN32
IV_SENDTEXT (INT8 *WSI, char *text, int textLen)
#else
iv_sendtext_(INT8 *WSI, char *text, int textLen)
#endif
{
  char                *name;
  struct libwebsocket *wsi;

  name = IV_f2c(text, textLen);
  if (name == NULL) return;
  wsi = (struct libwebsocket *) *WSI;
  wv_sendText(wsi, name);
  free(name);
}


void 
browserMessage(struct libwebsocket *wsi, char *text, int len)
{
  INT8 WSI;

  WSI = (INT8) wsi;
#ifdef WIN32
  BROWSERMESSAGE (&WSI, text, len);
#else
  browsermessage_(&WSI, text, len);
#endif
}


void
#ifdef WIN32
IV_ADJUSTVERTS (INT8 *item, float *focus)
#else
iv_adjustverts_(INT8 *item, float *focus)
#endif
{
  wvData *gitem;

  gitem = (wvData *) *item;
  wv_adjustVerts(gitem, focus);
}
