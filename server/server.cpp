#include "Controller.h"
#include "serverlib/wst.h"
#include "optparse/OptionParser.h"
#include <chrono>
#include <exception>
#include <thread>
#include <sstream>
#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

// <ctc> testing this will work in main thread (crashes from Controller, which is a thread)
#include <pybind11/eigen.h>

#include "utils/Data.h"

const int kDefaultPort = 7681;
const std::string kDefaultDatapath("../../examples");
 
dspacex::Controller *controller = nullptr;

extern "C" void browserData(void *wsi, void *data) {
  controller->handleData(wsi, data);
}

extern "C" void browserText(void *wsi, char *text, int lena) {
  controller->handleText(wsi, std::string(text));
}

// <ctc> get these outta the global namespace
py::object thumbnail_renderer;
py::object thumbnail_utils;

int main(int argc, char *argv[])
{
  py::scoped_interpreter guard{}; // start the interpreter and keep it alive
  py::print("Hello, World!"); // use the Python API

  py::module sys = py::module::import("sys");
  sys.attr("path").attr("insert")(1, "/Users/cam/code/dSpaceX");

  // test that this function can be called from main thread (it crashes opening a GL window from Controller)
  thumbnail_utils = py::module::import("data.thumbnails.thumbnail_utils");
  thumbnail_renderer = thumbnail_utils.attr("vtkRenderMesh")();

  // updated class interface, works from python
  auto& ren = thumbnail_renderer;
  for (auto i=0; i<10; i++) {
    std::ostringstream filename;
    filename << "/Users/cam/data/dSpaceX/latest/nanoparticles_mesh/unprocessed_data/shape_representations/" << i+10 << ".ply";
    ren.attr("loadNewMesh")(filename.str());
    ren.attr("update")();

    auto npvec = thumbnail_utils.attr("vtkToNumpy")(ren.attr("screenshot")()).cast<py::array_t<unsigned char>>();
    dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
    std::ostringstream outname;
    outname << "/tmp/test-nanosnap-" << std::setfill('0') << std::setw(1) << i << ".png";
    image.write(outname.str());
  }
  
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
  static int snapshotidx = 0;
  while (wst_statusServer(0)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // <ctc> maybe even make this shorter
    while (!controller->genthumbs.empty()) {
      auto item(controller->genthumbs.front());
      dspacex::Controller::generateCustomThumbnail(item.I, item.response);
      controller->genthumbs.pop_front();
    }
  }
  wst_cleanupServers();
  return 0;
}
