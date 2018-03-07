/*
 *	The Web Socket Transport
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <unistd.h>

#include "wst.h"



/*@null@*/ /*@out@*/ /*@only@*/ void *
wst_alloc(int nbytes)
{
  if (nbytes <= 0) return NULL;
  return malloc(nbytes);
}


/*@null@*/ /*@only@*/ void *
wst_calloc(int nele, int size)
{
  if (nele*size <= 0) return NULL;
  return calloc(nele, size);
}


/*@null@*/ /*@only@*/ void *
wst_realloc(/*@null@*/ /*@only@*/ /*@returned@*/ void *ptr, int nbytes)
{
  if (nbytes <= 0) return NULL;
  return realloc(ptr, nbytes);
}


void
wst_free(/*@null@*/ /*@only@*/ void *ptr)
{
  if (ptr == NULL) return;
  free(ptr);
}


/*@null@*/ /*@only@*/ char *
wst_strdup(/*@null@*/ const char *str)
{
  int  i, len;
  char *dup;

  if (str == NULL) return NULL;

  len = strlen(str) + 1;
  dup = (char *) wst_alloc(len*sizeof(char));
  if (dup != NULL)
    for (i = 0; i < len; i++) dup[i] = str[i];

  return dup;
}


void 
wst_destroyContext(wstContext **context)
{
  wstContext *cntxt;

  cntxt = *context;
  wst_free(cntxt);
  *context = NULL;
}


/*@null@*/ wstContext *wst_createContext()
{
  wstContext *context;

  context = (wstContext *) wst_alloc(sizeof(wstContext));
  if (context == NULL) return NULL;

  context->handShake  = 0;
  context->ioAccess   = 0;
  context->dataAccess = 0;
  context->sent       = 0;

  return context;
}


int
wst_handShake(wstContext *cntxt)
{

  if (cntxt->handShake == 0) {
    while (cntxt->ioAccess != 0) usleep(10000);
    cntxt->handShake = cntxt->dataAccess = 1;
  } else {
    cntxt->sent = cntxt->handShake = cntxt->dataAccess = 0;
    /* wait for the send */
    while (cntxt->sent == 0) usleep(10000);
  }
  return cntxt->handShake;
}


void
wst_freeData(wstData *data)
{
  if (data == NULL) return;
  if (data->name   != NULL) wst_free(data->name);
  if (data->header != NULL) wst_free(data->header);
  if (data->data   != NULL) wst_free(data->data);
  wst_free(data);
}


/*@null@*/ wstData *
wst_createData(const char *name, enum TypedArray type, int hlen,
               const int *header, int len, void *array)
{
  int     i;
  size_t  alen;
  wstData *data;

  if (name == NULL) {
    printf(" wst_createData: name is NULL!\n");
    return NULL;
  }
  if (hlen < 1) {
    printf(" wst_createData: hlen < 1 (%d)!\n", hlen);
    return NULL;
  }
  if (header == NULL) {
    printf(" wst_createData: header is NULL!\n");
    return NULL;
  }
  if (array == NULL) {
    printf(" wst_createData: array is NULL!\n");
    return NULL;
  }
  
  data = (wstData *) wst_alloc(sizeof(wstData));
  if (data == NULL) {
    printf(" wst_createData: Malloc error on object!\n");
    return NULL;
  }
  data->name   = wst_strdup(name);
  if (data->name == NULL) {
    printf(" wst_createData: Malloc error on name!\n");
    wst_free(data);
    return NULL;
  }
  data->type   = type;
  data->hlen   = hlen;
  data->header = (int *) wst_alloc(hlen*sizeof(int));
  if (data->header == NULL) {
    printf(" wst_createData: Malloc error on %d header ints!\n", hlen);
    wst_free(data->name);
    wst_free(data);
    return NULL;
  }
  for (i = 0; i < hlen; i++) data->header[i] = header[i];
  data->len  = len;
  alen       = len*TypedArrayBytes[type];
  data->data = wst_alloc(alen);
  if (data->data == NULL) {
    printf(" wst_createData: Malloc error on array!\n");
    wst_free(data->header);
    wst_free(data->name);
    wst_free(data);
    return NULL;
  }
  memcpy(data->data, array, alen);
  
  return data;
}
