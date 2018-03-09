/*
 *      dSpaceX server
 */
#include "HDGenericProcessor.h"
#include "SimpleHDVizDataImpl.h"
#include "TopologyData.h"
#include "LegacyTopologyDataImpl.h"
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "util/DenseVectorSample.h"
#include "util/csv/loaders.h"

#include <cstdlib>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>		// usleep

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#define strtok_r strtok_s
#endif

#include "wst.h"

#include "dsxdyn.h"

/* globals used in these functions */
static wstContext *cntxt;
static dsxContext dsxcntxt;


/* NOTE: the data read must me moved to the back-end and then
         these globals cn be made local */
std::vector<DenseVectorSample*> samples;
FortranLinalg::DenseVector<Precision> y;



int main(int argc, char *argv[])
{
  int   stat, port = 7681;
  char  *startapp;  

  /* get our starting application line
   *
   * for example on a Mac:
   * setenv DSX_START "open -a /Applications/Firefox.app ../client/dSpaceX.html"
   */
  startapp = getenv("DSX_START");

  if ((argc != 1) && (argc != 2)) {
    printf("\n Usage: dSpaceX [port]\n\n");
    return 1;
  }
  if (argc == 2) port = atoi(argv[1]);

  /* initialize the back-end subsystem
     assume we have started where we can file the DLL/so */
  stat = dsx_Load(&dsxcntxt, "dSpaceXbe");
  if (stat < 0) {
    printf(" failed to Load Back-End %d!\n", stat);
    return 1;
  }

  /* create the Web Socket Transport context */
  cntxt = wst_createContext();
  if (cntxt == NULL) {
    printf(" failed to create wstContext!\n");
    return -1;
  }

  // start the server code  
  stat = 0;
  if (wst_startServer(7681, NULL, NULL, NULL, 0, cntxt) == 0) {
    
    /* we have a single valid server */
    while (wst_statusServer(0)) {
      usleep(500000);
      if (stat == 0) {
        if (startapp != NULL) system(startapp);
        stat++;
      }
      if (wst_nClientServer(0) == 0) {
        continue;
      }
      // wst_broadcastText("I'm here!");
    }
  }

  /* finish up */
  wst_cleanupServers();
  return 0;
}


/* call-back invoked when a message arrives from the browser
 *   - each client in multi-browser setting is handled by a differnt thread
 *   - thread IDs are not used, but text websocket interface pointers are
 *   - this code must keep track of individual client differences
 *   - all data in the binary protocol is handled via brodcasts
 */
extern "C" void browserMessage(void *wsi, char *text, int lena)
{
  int           i, j, width, height, stat, color_only = 1;
  char          copy[133], *word, *word1, *lasts, sep[2] = " ";
  static float  lims[2] = {-1.0, +1.0};
  unsigned char *image;
  static int    nCases, nParams, nQoIs, nCrystal, minP, maxP, persistence, iCrystal;
  static int    icase = 1, key = -1;
  static double *cases = NULL, *QoIs = NULL;
  static char   **pNames, **qNames;

  static HDProcessResult *result = nullptr;
  static HDVizData *data = nullptr;
  static TopologyData *topoData = nullptr;
  static FortranLinalg::DenseMatrix<Precision>  lCrystal, layout, distances;
  static std::vector<unsigned int> edgeIndices, eCrystal;
  static FortranLinalg::DenseVector<int> parts, pCrystal, jCrystal;
  static FortranLinalg::DenseVector<Precision>  yCrystal;

  printf(" pWSI = %lx\n", (unsigned long) wsi);
  {
    int  n;
    void **interfaces;

    stat = wst_activeTextInterfaces(0, &n, &interfaces);
    if (interfaces != NULL)
      for (i = 0; i < n; i++)
        printf(" iWSI = %lx\n", (unsigned long) interfaces[i]);
  }

  /* get token */
  strncpy(copy, text, 132);
  word = strtok_r(copy, sep, &lasts);
  if (word == NULL) return;


  printf(" Unknown Token: %s (%d)\n", text, lena);
}

extern "C" void browserData(void *wsi, void *data)
{
  // TODO:  Implement
}

extern "C" void browserText(void *wsi, char *text, int lena)
{
  // TODO:  Implement 
}