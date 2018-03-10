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

#include <jsoncpp/json.h>

#include <exception>
#include <string>
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


extern "C" void browserData(void *wsi, void *data)
{
  // TODO:  Implement
}

extern "C" void browserText(void *wsi, char *text, int lena)
{  
  std::string message(text);
  Json::Reader reader; 
  Json::Value object;

  try {
    reader.parse(message, object);

    int messageId = object["id"].asInt();
    std::string commandName = object["name"].asString();

    std::cout << "messageId:" << messageId << std::endl;

    
    if(commandName == "fetchDatasetList") {
      std::cout << "Received request for avaliable datasets." << std::endl;
    } else {
      std::cout << "Error: Unrecognized Command: " << commandName << std::endl;
    }
  } catch(const std::exception &e) {
    std::cerr << "Command Parsing Error." << e.what() << std::endl;     
    // TODO: Send back an error message.
  }

  

  // if (strcmp(word,"fetchDatasetList") == 0) {
  //   std::cout << "Received request for avaliable datasets." << std::endl;
  //   return;
  // }

  // printf(" Unknown Token: %s (%d)\n", text, lena);
}