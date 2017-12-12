/*
 *  dSpaceX - dynamic loader
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsxdyn.h"


/* ************************* Utility Functions ***************************** */

static /*@null@*/ DLL dsxDLopen(const char *name)
{
  int  i, len;
  DLL  dll;
  char *full;
  
  if (name == NULL) {
    printf(" Information: Dynamic Loader invoked with NULL name!\n");
    return NULL;
  }
  len  = strlen(name);
  full = (char *) malloc((len+5)*sizeof(char));
  if (full == NULL) {
    printf(" Information: Dynamic Loader MALLOC Error!\n");
    return NULL;
  }
  for (i = 0; i < len; i++) full[i] = name[i];
  full[len  ] = '.';
#ifdef WIN32
  full[len+1] = 'D';
  full[len+2] = 'L';
  full[len+3] = 'L';
  full[len+4] =  0;
  dll = LoadLibrary(full);
#else
  full[len+1] = 's';
  full[len+2] = 'o';
  full[len+3] =  0;
  dll = dlopen(full, RTLD_NOW /* RTLD_LAZY */);
#endif
  
  if (!dll) {
    printf(" Information: Dynamic Loader Error for %s\n", full);
#ifndef WIN32
/*@-mustfreefresh@*/
    printf("              %s\n", dlerror());
/*@+mustfreefresh@*/
#endif
    free(full);
    return NULL;
  }
  
  free(full);
  return dll;
}


static void dsxDLclose(/*@unused@*/ /*@only@*/ DLL dll)
{
#ifdef WIN32
  FreeLibrary(dll);
#else
/* 
  need to keep it open so valgrind can find addresses!
  dlclose(dll);  
 */
#endif
}


static DLLFunc dsxDLget(DLL dll, const char *symname, const char *name)
{
  DLLFunc data;
  
#ifdef WIN32
  data = (DLLFunc) GetProcAddress(dll, symname);
#else
/*@-castfcnptr@*/
  data = (DLLFunc) dlsym(dll, symname);
/*@+castfcnptr@*/
#endif
  if ((data == NULL) && (name != NULL))
    printf(" dSpaceX Info: Couldn't get symbol %s in %s\n", symname, name);
  return data;
}


static int dsxDYNload(dsxContext *cntxt, const char *name)
{
  int i, len;
  DLL dll;
  
  dll = dsxDLopen(name);
  if (dll == NULL) return -3;
  cntxt->dsxInit  = (dsxI) dsxDLget(dll, "dsxInitialize", name);
  cntxt->dsxThumb = (dsxT) dsxDLget(dll, "dsxThumbNail",  name);
  cntxt->dsxDist  = (dsxD) dsxDLget(dll, "dsxDistance",   name);
  cntxt->dsxClean = (dsxC) dsxDLget(dll, "dsxCleanup",    name);
  if ((cntxt->dsxInit == NULL) || (cntxt->dsxThumb == NULL) ||
      (cntxt->dsxDist == NULL) || (cntxt->dsxClean == NULL)) {
    dsxDLclose(dll);
    return -4;
  }
  len  = strlen(name) + 1;
  cntxt->dsxName = (char *) malloc(len*sizeof(char));
  if (cntxt->dsxName == NULL) {
    dsxDLclose(dll);
    return -1;
  }
  for (i = 0; i < len; i++) cntxt->dsxName[i] = name[i];
  cntxt->dsxDLL   = dll;

  return 0;
}


/* ************************* Exposed Functions ***************************** */


int
dsx_Load(dsxContext *cntxt,
         const char *soName)
{
  return dsxDYNload(cntxt, soName);
}


int
dsx_Initialize(dsxContext *cntxt,
               int        *nCases,        /* the number of cases */
               int        *nParams,       /* number of parameters */
               double     **parameters,   /* the nCases by nParams data */
               char       ***pNames,      /* parameter names (nParams in len) */
               int        *nQoI,          /* number of quantities of interest */
               double     **QoIs,         /* the nCases by nQoI data */
               char       ***qNames)      /* QoI names (nQoI in len) */
{
  return cntxt->dsxInit(nCases, nParams,parameters,pNames, nQoI,QoIs,qNames);
}


int
dsx_ThumbNail(dsxContext    *cntxt,
              int           caseIndex,    /* the case number (0-bias) */
              int           *width,       /* the returned image width */
              int           *height,      /* the returned image height */
              unsigned char **image)      /* the pointer to the image
                                             4*width*heigth -- rgba */
{
  return cntxt->dsxThumb(caseIndex, width, height, image);
}


int
dsx_Distance(dsxContext *cntxt,
              int       caseInd1,         /* the first case number (0-bias) */
              int       caseInd2,         /* the second case number (0-bias) */
              double    *distance)        /* the returned distance */
{
  return cntxt->dsxDist(caseInd1, caseInd2, distance);
}


void
dsx_CleanupAll(dsxContext *cntxt)
{
  if (cntxt->dsxClean == NULL) return;
  cntxt->dsxClean();
  
  free(cntxt->dsxName);
  dsxDLclose(cntxt->dsxDLL);
}
