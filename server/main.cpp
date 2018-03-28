#include "controller.h"
#include "wst.h"

#include <chrono>
#include <exception>
#include <thread>

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#define strtok_r strtok_s
#endif

const int kDefaultPort = 7681;
Controller *controller = nullptr;

extern "C" void browserData(void *wsi, void *data) {
  controller->handleData(wsi, data);
}

extern "C" void browserText(void *wsi, char *text, int lena) {
  controller->handleText(wsi, std::string(text));
}

int main(int argc, char *argv[])
{
  if (argc > 2) {
    printf("\n Usage: dSpaceX [port]\n\n");
    return 1;
  }

  int port = kDefaultPort;
  if (argc == 2) {
    port = atoi(argv[1]);
  }

  try {
    controller = new Controller();
    controller->configureAvailableDatasets();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  // Create the Web Socket Transport context.
  wstContext *cntxt = wst_createContext();
  if (!cntxt) {
    std::cout << "Failed to create wstContext." << std::endl;
    return -1;
  }

  // Start listening for connections.
  int status = wst_startServer(port, nullptr, nullptr, nullptr, 0, cntxt);
  if (status != 0) {
    std::cout << "FATAL: wst_startServer returned failure." << std::endl;
    return -1;
  }

  /**
   * If browser command supplied, open the url. For example on a Mac:
   * setenv DSX_START "open -a /Applications/Firefox.app ../client/dSpaceX.html"
   */
  char *startapp = getenv("DSX_START");
  if (startapp) {
    system(startapp);
  }

  // Keep server alive.
  while (wst_statusServer(0)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  wst_cleanupServers();
  return 0;
}
