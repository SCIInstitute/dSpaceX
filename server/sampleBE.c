/*
 *  dSpaceX - sample dynamicly loaded back-end
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define getcwd   _getcwd
#define PATH_MAX _MAX_PATH
#else
#include <unistd.h>
#include <limits.h>
#endif


/* image file reader */
extern int dsx_getThumbNail( const char *Name, int *Width, int *Height,
                             unsigned char **Image );


/* storage location for everything we allocate
   we must free the memory -- Windows DLL! */
typedef struct {
  int           nParam;
  int           nQoI;
  double        *cases;
  double        *qois;
  char          **pNames;
  char          **qNames;
  unsigned char *thumb;
  char          currentPath[PATH_MAX];
} BEstorage;

static BEstorage save;
static int       first = -1;


/* function that gets invoked to return the data for the design setting */
int
dsxInitialize(int *nCases, int *nParams, double **params, char ***PNames,
              int *nQoI, double **QoIs, char ***QNames)
{
  int    i, j, m, n, len;
  double *cases, *qois;
  char   text[133], **pNames, **qNames;
  FILE   *fp;
  
  if (first == -1) {
    save.nParam = 0;
    save.nQoI   = 0;
    save.cases  = NULL;
    save.qois   = NULL;
    save.pNames = NULL;
    save.qNames = NULL;
    save.thumb  = NULL;
  }

  *nCases = *nParams = *nQoI = 0;
  *params = *QoIs    = NULL;
  *PNames = *QNames  = NULL;
  
  fp = fopen("aeroDB_ALL.dat", "r");
  if (fp == NULL) return -1;
  
  fscanf(fp, "%d %d %d", nParams, nQoI, nCases);
  
  len    = *nCases;
  cases  = (double *) malloc(*nParams*len*sizeof(double));
  qois   = (double *) malloc(*nQoI*   len*sizeof(double));
  pNames = (char **)  malloc(*nParams    *sizeof(char *));
  qNames = (char **)  malloc(*nQoI       *sizeof(char *));
  if ((cases  == NULL) || (qois   == NULL) ||
      (pNames == NULL) || (qNames == NULL)) {
    if (cases  != NULL) free(cases);
    if (qois   != NULL) free(qois);
    if (pNames != NULL) free(pNames);
    if (qNames != NULL) free(qNames);
    return -2;
  }
  
  for (i = 0; i < *nParams; i++) {
    fscanf(fp, " \"%[^\"]\"", text);
    len = strlen(text)+1;
    pNames[i] = malloc(len*sizeof(char));
    if (pNames[i] == NULL) continue;
    for (j = 0; j < len; j++) pNames[i][j] = text[j];
  }
  for (i = 0; i < *nQoI; i++) {
    fscanf(fp, " \"%[^\"]\"", text);
    len = strlen(text)+1;
    qNames[i] = malloc(len*sizeof(char));
    if (qNames[i] == NULL) continue;
    for (j = 0; j < len; j++) qNames[i][j] = text[j];
  }
  
  for (m = n = j = 0; j < *nCases; j++) {
    for (i = 0; i < *nParams; i++, m++) fscanf(fp, "%lf", &cases[m]);
    for (i = 0; i < *nQoI;    i++, n++) fscanf(fp, "%lf", &qois[n]);
  }
  fclose(fp);
  
  /* get our current location */
  (void) getcwd(save.currentPath, PATH_MAX);
  printf("Path = %s\n", save.currentPath);
  
  save.nParam = *nParams;
  save.nQoI   = *nQoI;
  save.cases  = cases;
  save.qois   = qois;
  save.pNames = pNames;
  save.qNames = qNames;
  *params     = cases;
  *QoIs       = qois;
  *PNames     = pNames;
  *QNames     = qNames;
  
  first++;
  return 0;
}


/* function that returns a thumbnail of the specified case */
int
dsxThumbNail(/*@unused@*/ int caseIndex, int *width, int *height,
             unsigned char **image)
{
  int stat;
  
  *width = *height = 0;
  *image = NULL;
  if (save.thumb != NULL) free(save.thumb);
  save.thumb = NULL;

#ifdef WIN32
  stat = dsx_getThumbNail("mach_alpha\\n33T.jpg", width, height, image);
#else
  stat = dsx_getThumbNail("mach_alpha/n33T.jpg",  width, height, image);
#endif
  if (stat == 0) save.thumb = *image;
  
  return stat;
}


/* computes distance between the 2 specified cases */
int
dsxDistance(/*@unused@*/ int case1, /*@unused@*/ int case2, double *distance)
{
  *distance = 0.0;
  
  return 0;
}


/* cleans up any allocated memory */
void
dsxCleanup()
{
  int i;

  if (save.thumb  != NULL) free(save.thumb);
  if (save.cases  != NULL) free(save.cases);
  if (save.qois   != NULL) free(save.qois);
  if (save.pNames != NULL) {
    for (i = 0; i < save.nParam; i++)
      if (save.pNames[i] != NULL) free(save.pNames[i]);
    free(save.pNames);
  }
  if (save.qNames != NULL) {
    for (i = 0; i < save.nQoI; i++)
      if (save.qNames[i] != NULL) free(save.qNames[i]);
    free(save.qNames);
  }
}
