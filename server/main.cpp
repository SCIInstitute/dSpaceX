#include "Controller.h"
#include "serverlib/wst.h"

#include <chrono>
#include <exception>
#include <thread>


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
    controller = new Controller();     //TODO: add parameter to specific dataset location(s) (defaults to ../../examples)
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
