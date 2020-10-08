#include "Controller.h"
#include "serverlib/wst.h"
#include "optparse/OptionParser.h"
#include <chrono>
#include <exception>
#include <thread>
#include <pybind11/embed.h> // everything needed for embedding
#include <pybind11/eigen.h> // <ctc> testing this will work in main thread (crashes from Controller, which is a thread)
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
  sys.attr("path").attr("insert")(1, "/Users/cam/code/dSpaceX");
  py::print(sys.attr("path"));

  // import the module and call the add function from this module
  // py::object cfs_module = py::module::import("data.test.call_from_server");
  // auto foo = cfs_module.attr("add")(4, 3).cast<int>();

  // or put them all together and import the function itself (I suspect this still imports the whole module)
  //py::function add_func = py::module::import("data.test.call_from_server").attr("add").cast<py::function>();
  //auto foo = add_func(4, 3).cast<int>();
  //std::cout << "result: " << foo << std::endl;

  // <ctc> NOTE: had problems with imports, but now they seem okay... here is what I was going to add to Controller and why:
  // pybind11 modules callable from c++ (issues importing more than once, so just hanging on to them here for now)
  // note: probably related to this https://github.com/pybind/pybind11/issues/1439, so we just need to initialize the
  //       interpreter every time it's used.
  //py::function imageFromPts;

  // test that this function can be called from main thread (it crashes opening a GL window from Controller)
  py::object thumbnail_utils_module = py::module::import("data.thumbnails.thumbnail_utils");
  Eigen::MatrixXf I = Eigen::MatrixXf::Random(1, 299568);
  auto vtkimg = thumbnail_utils_module.attr("vtkRenderMesh")(I);
  auto npimg = thumbnail_utils_module.attr("vtkToNumpy")(vtkimg).cast<py::array_t<unsigned char>>();

  // import the thumbnail_utils module
  // py::object data_module = py::module::import("data");
  // py::object thumbnails_module = py::module::import("data.thumbnails");
  // py::object eff_vtk = py::module::import("vtk");
  // py::object thumbnail_utils_module = py::module::import("data.thumbnails.thumbnail_utils");
  // py::function img_from_pts = thumbnail_utils_module.attr("generate_image_from_vertices_and_faces").cast<py::function>();

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
