#include "Controller.h"
#include "serverlib/wst.h"
#include "optparse/OptionParser.h"
#include <chrono>
#include <exception>
#include <thread>
#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

const int kDefaultPort = 7681;
const std::string kDefaultDatapath("../../examples");
 
dspacex::Controller *controller = nullptr;

extern "C" void browserData(void *wsi, void *data) {
  controller->handleData(wsi, data);
}

extern "C" void browserText(void *wsi, char *text, int lena) {
  controller->handleText(wsi, std::string(text));
}

int main(int argc, char *argv[])
{
  py::scoped_interpreter guard{}; // start the interpreter and keep it alive
  py::print("Hello, World!"); // use the Python API

  py::module sys = py::module::import("sys");
  py::print(sys.attr("path"));
  sys.attr("path").attr("insert")(1, "/Users/cam/code/dSpaceX/data/test");
  py::print(sys.attr("path"));
  py::object DataProc = py::module::import("call_from_server"); // run server from <src>/<build>
  auto foo = DataProc.attr("add")(4, 3).cast<int>();
  std::cout << "result: " << foo << std::endl;

  using optparse::OptionParser;
  OptionParser parser = OptionParser().description("dSpaceX Server");
  parser.add_option("-p", "--port").dest("port").type("int").set_default(kDefaultPort).help("server port");
  parser.add_option("-d", "--datapath").dest("datapath").set_default(kDefaultDatapath).help("path to datasets");

  const optparse::Values &options = parser.parse_args(argc, argv);
  const std::vector<std::string> args = parser.args();

  int port = options.get("port");
  std::string datapath = options["datapath"];
  
  try {
    controller = new dspacex::Controller(datapath);
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
