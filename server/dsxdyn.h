/*
 *  dSpaceX - dynamic loading header
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define DLL HINSTANCE
#define false 0
#define true  (!false)
#else
#include <stdbool.h>
#include <dlfcn.h>
#define DLL void *
#endif


typedef int  (*DLLFunc) (void);
typedef int  (*dsxI)    (int *, int *, double **, char ***, int *, double **,
                         char ***);
typedef int  (*dsxT)    (int, int *, int *, unsigned char **);
typedef int  (*dsxD)    (int, int, double *);
typedef void (*dsxC)    (void);

typedef struct {
  char *dsxName;
  DLL   dsxDLL;
  dsxI  dsxInit;
  dsxT  dsxThumb;
  dsxD  dsxDist;
  dsxC  dsxClean;
} dsxContext;


#ifdef __ProtoExt__
#undef __ProtoExt__
#endif
#ifdef __cplusplus
extern "C" {
#define __ProtoExt__
#else
#define __ProtoExt__ extern
#endif

/* load the DLL/so -- returns instIndex */
__ProtoExt__ int
dsx_Load(dsxContext *cntxt,
         const char *name);              /* name of DLL/so on disk */

/* get the set of cases -- path already set for fileIO */
__ProtoExt__ int
dsx_Initialize(dsxContext *cntxt,
               int        *nCases,        /* the number of cases */
               int        *nParams,       /* number of parameters */
               double     **parameters,   /* the nCases by nParams data */
               char       ***pNames,      /* parameter names (nParams in len) */
               int        *nQoI,          /* number of quantities of interest */
               double     **QoIs,         /* the nCases by nQoI data */
               char       ***qNames);     /* QoI names (nQoI in len) */

/* get thumbnail image */
__ProtoExt__ int
dsx_ThumbNail(dsxContext    *cntxt,
              int           caseIndex,    /* the case number (0-bias) */
              int           *width,       /* the returned image width */
              int           *height,      /* the returned image height */
              unsigned char **image);     /* the pointer to the image
                                             4*width*heigth -- rgba */

/* compute distance */
__ProtoExt__ int
dsx_Distance(dsxContext *cntxt,
             int        caseInd1,        /* the first case number */
             int        caseInd2,        /* the second case number */
             double     *distance);      /* the returned distance */

/* cleanup function */
__ProtoExt__ void
dsx_CleanupAll(dsxContext *cntxt);

#ifdef __cplusplus
}
#endif
