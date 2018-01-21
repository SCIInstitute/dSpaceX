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

#include "wsserver.h"

#include "dsxdyn.h"

/* globals used in these functions */
static wvContext  *cntxt;
static dsxContext dsxcntxt;


extern void dsx_drawKey(wvContext *cntxt, float *lims, /*@null@*/ char *name);
extern void dsx_draw3D(wvContext *cntxt, float *lims, int nCrystal, int key,
                       int flag);
extern void dsx_draw2D(wvContext *cntxt, FortranLinalg::DenseVector<Precision> y,
                       FortranLinalg::DenseMatrix<Precision> layout,
                       std::vector<unsigned int> &edgeIndices,
                       FortranLinalg::DenseVector<int> parts,
                       float *lims, int key, int flag);


/* NOTE: the data read must me moved to the back-end and then
         these globals cn be made local */
std::vector<DenseVectorSample*> samples;
FortranLinalg::DenseVector<Precision> y;



int main(int argc, char *argv[])
{
  int   stat, port = 7681;
  char  *startapp;
  float eye[3]    = {0.0, 0.0, 7.0};
  float center[3] = {0.0, 0.0, 0.0};
  float up[3]     = {0.0, 1.0, 0.0};

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

  /* create the WebViewer context */
  cntxt = wv_createContext(0, 30.0, 1.0, 10.0, eye, center, up);
  if (cntxt == NULL) {
    printf(" failed to create wvContext!\n");
  } else {

    /* start the server code -- at this time we only run a single server */
    stat = 0;
    if (wv_startServer(port, NULL, NULL, NULL, 0, cntxt) == 0) {

      /* we have a valid server -- stay alive a long as we have a client */
      while (wv_statusServer(0)) {
        usleep(500000);
        if (stat == 0) {
          if (startapp != NULL) system(startapp);
          stat++;
        }
      }
    }
    wv_cleanupServers();
  }

  /* finish up */
  dsx_CleanupAll(&dsxcntxt);
  return 0;
}


static void
sendCase(void *wsi, int icase, int nParams, double *cases, char **pNames,
                               int nQoIs,   double *QoIs,  char **qNames)
{
  // need to check for "strip mining" when ibuf > BUFLEN
  char *buf = new char[BUFLEN];
  buf[BUFLEN-1] = 0;

  snprintf(buf, BUFLEN-1, "Case:%5d:%4d:%4d", icase, nParams, nQoIs);
  int ibuf = 20;

  /* need to make sure we do not overflow! */
  for (int i = 0; i < nParams; i++) {
    snprintf(&buf[ibuf], BUFLEN-ibuf-1, ":%s:%lg",
             pNames[i], cases[nParams*(icase-1)+i]);
    ibuf += strlen(&buf[ibuf]);
  }

  for (int i = 0; i < nQoIs; i++) {
    snprintf(&buf[ibuf], BUFLEN-ibuf-1, ":%s:%lg",
             qNames[i], QoIs[nQoIs*(icase-1)+i]);
    ibuf += strlen(&buf[ibuf]);
  }

  wv_sendText(wsi, buf);
  delete [] buf;
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
  static int    nCases, nParams, nQoIs, nCrystal, maxP, persistence, iCrystal;
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

/*
  printf(" pWSI = %lx\n", (unsigned long) wsi);
  {
    int  n;
    void **interfaces;

    stat = wv_activeInterfaces(0, &n, &interfaces);
    if (interfaces != NULL)
      for (i = 0; i < n; i++)
        printf(" iWSI = %lx\n", (unsigned long) interfaces[i]);
  } */

  /* get token */
  strncpy(copy, text, 132);
  word = strtok_r(copy, sep, &lasts);
  if (word == NULL) return;

  /* initialize */
  if (strcmp(word,"initialize") == 0) {
    /* are we already loaded? */
    if (cases != NULL) {
      sendCase(wsi, icase, nParams, cases, pNames, nQoIs, QoIs, qNames);
      return;
    }

    // Begin HDProcess Block
    try {
      FortranLinalg::DenseMatrix<Precision> md =
          HDProcess::loadCSVMatrix("../../examples/truss/distances.csv");
      FortranLinalg::DenseVector<Precision> mv =
          HDProcess::loadCSVColumn("../../examples/truss/max_stress.txt");

      std::string xArg = "../../examples/gaussian2d/Geom.data.hdr";
      std::string fArg = "../../examples/gaussian2d/Function.data.hdr";

      FortranLinalg::DenseMatrix<Precision> x =
          FortranLinalg::LinalgIO<Precision>::readMatrix(xArg);
      // FortranLinalg::DenseVector<Precision>
      y = FortranLinalg::LinalgIO<Precision>::readVector(fArg);

      // Build Sample Vector from Input Data
      for (j = 0; j < x.N(); j++) {
        FortranLinalg::DenseVector<Precision> vector(x.M());
        for (i = 0; i < x.M(); i++) {
          vector(i) = x(i, j);
        }
        DenseVectorSample *sample = new DenseVectorSample(vector);
        samples.push_back(sample);
      }

      HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
      DenseVectorEuclideanMetric metric;
      distances = genericProcessor.computeDistances(samples, metric);

      result = genericProcessor.processOnMetric(
                                 distances /* distance matrix */,
                                 y         /* qoi */,
                                 15        /* knn */,
                                 5         /* samples */,
                                 20        /* persistence */,
                                 true      /* random */,
                                 0.25      /* sigma */,
                                 0         /* smooth */);
      data = new SimpleHDVizDataImpl(result);
      topoData = new LegacyTopologyDataImpl(data);

      MetricMDS<Precision> mds;
      layout = mds.embed(distances, 2);

      for (i = 0; i < data->getNearestNeighbors().N(); i++) {
        for (j = 0; j < data->getNearestNeighbors().M(); j++) {
          int neighbor = data->getNearestNeighbors()(j, i);
            edgeIndices.push_back(i);
            edgeIndices.push_back(neighbor);
        }
      }

    } catch (const char *err) {
      std::cerr << err << std::endl;
    }
    // End HDProcess Block

    stat = dsx_Initialize(&dsxcntxt, &nCases, &nParams, &cases, &pNames,
                          &nQoIs, &QoIs, &qNames);
    printf(" dsx_Initialize = %d   nCases = %d, nParams = %d, nQoIs = %d\n",
           stat, nCases, nParams, nQoIs);
    if ((stat != 0) || (cases == NULL)) {
      wv_sendText(wsi, "Error: dSpaceX BackEnd Initialization failure!");
      wv_killInterface(0, wsi);
      return;
    }

    /* send case */
    sendCase(wsi, icase, nParams, cases, pNames, nQoIs, QoIs, qNames);

    /* compute the Morse-Smale Complex */
    maxP = data->getMaxPersistenceLevel();
    printf(" getMinPersistenceLevel = %d\n", data->getMinPersistenceLevel());
    printf(" getMaxPersistenceLevel = %d\n", maxP);
    persistence = 0;
    parts       = data->getCrystalPartitions(persistence);
    nCrystal    = 0;
    for (int i = 0; i < parts.N(); i++)
      if (parts(i) > nCrystal) nCrystal = parts(i);
    nCrystal++;
    iCrystal = 0;
    printf(" nCrystals is %d\n", nCrystal);

    /* put up key & render */
    color_only = 0;
    word       = "next";
  }

  /* slider change */
  if (strcmp(word,"slide") == 0) {
    word  = strtok_r(NULL, sep, &lasts);
    if (word == NULL) {
      printf(" No slider data!\n");
      return;
    }
    float slide = (maxP+1)/100.0;
    width       = (atoi(word)-1)*slide;
    width       = maxP - width;
    if (width != persistence) {
      persistence = width;
      printf(" new persistence = %d\n", persistence);

      /* recompute the Morse-Smale Complex using new persistence */
      parts    = data->getCrystalPartitions(persistence);
      nCrystal = 0;
      for (int i = 0; i < parts.N(); i++)
        if (parts(i) > nCrystal) nCrystal = parts(i);
      nCrystal++;
      iCrystal = 0;
      printf(" nCrystals is %d\n", nCrystal);

      if (key == -1) {
        lims[0] = 0.0;
        lims[1] = nCrystal-1;
        dsx_drawKey(cntxt, lims, "Crystal");
      }

      /* set 3D rendering of the result */
      dsx_draw3D(cntxt, lims, nCrystal, key, 0);
      /* set 2D rendering of the result */
      dsx_draw2D(cntxt, y, layout, edgeIndices, parts, lims, key, 0);
    }

    return;
  }

  /* locate2D request */
  if (strcmp(word,"locate2D") == 0) {
    word  = strtok_r(NULL, sep, &lasts);
    word1 = strtok_r(NULL, sep, &lasts);
    float xloc  = atof(word);
    float yloc  = atof(word1);
    float dist2 = 10000.0;
    int   ipart = iCrystal;
    int   icasx = 1;
    /* note: does not take into account the application of "focus" */
    if (iCrystal == 0) {
      for (i = 0; i < layout.N(); i++) {
        float d = (layout(0,i)-xloc)*(layout(0,i)-xloc) +
                  (layout(1,i)-yloc)*(layout(1,i)-yloc);
        if (d >= dist2) continue;
        dist2 = d;
        icasx = i+1;
        ipart = parts(i);
      }
    } else {
      for (i = 0; i < lCrystal.N(); i++) {
        float d = (lCrystal(0,i)-xloc)*(lCrystal(0,i)-xloc) +
                  (lCrystal(1,i)-yloc)*(lCrystal(1,i)-yloc);
        if (d >= dist2) continue;
        dist2 = d;
        icasx = jCrystal(i);
      }
    }
    dist2 = sqrtf(dist2);
/*
    printf(" locate2D = (%f, %f),  case# = %d,  crystal = %d,  dist = %f\n",
           xloc, yloc, icasx, ipart, dist2);
 */
    if (dist2 > 0.05) return;
    icase = icasx;
    sendCase(wsi, icase, nParams, cases, pNames, nQoIs, QoIs, qNames);
    dsx_drawKey(cntxt, lims, NULL);    /* turn off key */
    stat = dsx_ThumbNail(&dsxcntxt, icase, &width, &height, &image);
    if (stat != 0) {
      printf(" dsx_ThumbNail = %d\n", stat);
      if (key == -1) {
        dsx_drawKey(cntxt, lims, "Crystal");
      } else {
        dsx_drawKey(cntxt, lims, qNames[key]);
      }
    } else {
      stat = wv_thumbNail(cntxt, width, height, image);
      if (stat != 0) printf(" wv_thumbnail  = %d\n", stat);
    }
    return;
  }

  /* thumbnail request */
  if (strcmp(word,"thumbnail") == 0) {
    dsx_drawKey(cntxt, lims, NULL);    /* turn off key */
    stat = dsx_ThumbNail(&dsxcntxt, icase, &width, &height, &image);
    printf(" dsx_ThumbNail = %d  %d %d\n", stat, width, height);
    if (stat == 0) {
      stat = wv_thumbNail(cntxt, width, height, image);
      if (stat != 0) printf(" wv_thumbnail  = %d\n", stat);
    }
    return;
  }

  /* hover request */
  if (strcmp(word,"hover") == 0) {
    if (key == -1) {
      dsx_drawKey(cntxt, lims, "Crystal");
    } else {
      dsx_drawKey(cntxt, lims, qNames[key]);
    }
    return;
  }

  /* 3D picked request */
  if (strcmp(word,"Picked") == 0) {
    word = strtok_r(NULL, sep, &lasts);
    if (word != NULL) {
      word = strtok_r(NULL, sep, &lasts);
      if (word != NULL) {
        i = atoi(word);
        if (i == iCrystal) return;
        iCrystal = i;
        if (iCrystal < 0) iCrystal = 0;
        printf(" current Crystal = %d\n", iCrystal);
        if (iCrystal == 0) {
          dsx_draw2D(cntxt, y, layout, edgeIndices, parts, lims, key, 0);
        } else {
          MorseSmaleComplex *complex = topoData->getComplex(persistence);
          Crystal *crystal = complex->getCrystals()[iCrystal-1];
          std::vector<unsigned int> samples = crystal->getAllSamples();
          FortranLinalg::DenseMatrix<Precision> dSubset(samples.size(),
                                                        samples.size());
          FortranLinalg::DenseVector<int>       pCry(samples.size());
          FortranLinalg::DenseVector<int>       jCry(samples.size());
          FortranLinalg::DenseVector<Precision> yCry(samples.size());
          pCrystal = pCry;
          jCrystal = jCry;
          yCrystal = yCry;
          for (i = 0; i < samples.size(); i++) {
            pCrystal(i) = iCrystal;
            jCrystal(i) = samples[i] + 1;
            yCrystal(i) = y(samples[i]);
            for (j = 0; j < samples.size(); j++)
              dSubset(j, i) = distances(samples[j], samples[i]);
          }
          MetricMDS<Precision> mds;
          lCrystal = mds.embed(dSubset, 2);
          std::vector<unsigned int> eIndices;
          for (i = 0; i < data->getNearestNeighbors().N(); i++) {
            for (j = 0; j < data->getNearestNeighbors().M(); j++) {
              int neighbor = data->getNearestNeighbors()(j, i);

              std::vector<unsigned int>::iterator iter1 =
                              find(samples.begin(), samples.end(), neighbor);
              std::vector<unsigned int>::iterator iter2 =
                              find(samples.begin(), samples.end(), i);
              if (iter1 != samples.end() && iter2 != samples.end()) {
                eIndices.push_back((int) std::distance(samples.begin(),
                                                          iter1));
                eIndices.push_back((int) std::distance(samples.begin(),
                                                          iter2));
              }
            }
          }
          eCrystal = eIndices;

          dsx_draw2D(cntxt, yCrystal, lCrystal, eCrystal, pCrystal, lims, key,
                     0);
        }
      }
    }
    return;
  }

  /* 3D locating request */
  if (strcmp(word,"Located") == 0) {
    printf(" %s\n", text);
    return;
  }

  /* case request */
  if (strcmp(word,"case") == 0) {
    word = strtok_r(NULL, sep, &lasts);
    if (word == NULL) {
      icase++;
      if (icase > nCases) icase = 1;
    } else {
      icase = atoi(word);
    }
    sendCase(wsi, icase, nParams, cases, pNames, nQoIs, QoIs, qNames);
    return;
  }

  /* just change the color mapping */
  if ((strcmp(word,"next") == 0) || (strcmp(word,"limits") == 0)) {

    if (strcmp(word,"next") == 0) {
      key++;
      if (key >= nQoIs) {
        key = -1;
        lims[0] = 0.0;
        lims[1] = nCrystal-1;
      } else {
        lims[0] = lims[1] = cases[key];
        for (i = 1; i < nCases; i++) {
          if (QoIs[nQoIs*i+key] < lims[0]) lims[0] = QoIs[nQoIs*i+key];
          if (QoIs[nQoIs*i+key] > lims[1]) lims[1] = QoIs[nQoIs*i+key];
        }
      }
    } else {
      word  = strtok_r(NULL, sep, &lasts);
      word1 = strtok_r(NULL, sep, &lasts);
      if ((word == NULL) || (word1 == NULL)) {
        printf(" Enter new imits [old = %e, %e]:", lims[0], lims[1]);
        scanf("%f %f", &lims[0], &lims[1]);
      } else {
        lims[0] = atof(word);
        lims[1] = atof(word1);
      }
      printf(" new limits = %e %e\n", lims[0], lims[1]);
    }

    /* recolor data */
    if (key == -1) {
      dsx_drawKey(cntxt, lims, "Crystal");
    } else {
      dsx_drawKey(cntxt, lims, qNames[key]);
    }

    /* set 3D rendering of the result */
    dsx_draw3D(cntxt, lims, nCrystal, key, color_only);
    /* set 2D rendering of the result */
    if (iCrystal == 0) {
      dsx_draw2D(cntxt, y, layout, edgeIndices, parts, lims, key, color_only);
    } else {
      dsx_draw2D(cntxt, yCrystal, lCrystal, eCrystal, pCrystal, lims, key,
                 color_only);
    }

    return;
  }

  printf(" Unknown Token: %s (%d)\n", text, lena);
}
