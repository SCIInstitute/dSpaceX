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

py::object thumbnail_renderer;
bool nanosnap{false};

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
  py::object thumbnail_utils = py::module::import("data.thumbnails.thumbnail_utils");
  //Eigen::MatrixXf I = Eigen::MatrixXf::Random(1, 299568);
  // auto vtkimg = thumbnail_utils.attr("vtkRenderMesh_func")(I);
  // auto npimg = thumbnail_utils.attr("vtkToNumpy")(vtkimg).cast<py::array_t<unsigned char>>();

  // create a vtkRenderMesh instance for use by Controller
  // - renderer must be in main thread
  // - more efficient to instantiate it once then reuse it when a new thumbnail is needed
  thumbnail_renderer = thumbnail_utils.attr("vtkRenderMesh")();

#if 0
  // this works alone! But calling updateMesh ends up with an empty image... maybe cuz of random values
  {
    thumbnail_renderer.attr("updateMesh")();      // this *works* (doesn't change any points)
    thumbnail_renderer.attr("update")();
    auto npvec = thumbnail_renderer.attr("screenshot")().cast<py::array_t<unsigned char>>();
    dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
    image.write("/tmp/test_render_from_server-default.png");
  }
  {
    Eigen::MatrixXf I = Eigen::MatrixXf::Random(1, 299568);
    thumbnail_renderer.attr("updateMesh")(I);     // this works too! (ugly bcuz random points)
    thumbnail_renderer.attr("update")();
    auto npvec = thumbnail_renderer.attr("screenshot")().cast<py::array_t<unsigned char>>();
    dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
    image.write("/tmp/test_render_from_server-updated.png");
  }
  {
    thumbnail_renderer.attr("updateMesh")();      // this *works* (doesn't change any points)
    thumbnail_renderer.attr("update")();
    auto npvec = thumbnail_renderer.attr("screenshot")().cast<py::array_t<unsigned char>>();
    dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
    image.write("/tmp/test_render_from_server-default-again.png");
  }
  {
    Eigen::MatrixXf I = Eigen::MatrixXf::Random(1, 299568);
    thumbnail_renderer.attr("updateMesh")(I);     // this works too! (ugly bcuz random points)
    thumbnail_renderer.attr("update")();
    auto npvec = thumbnail_renderer.attr("screenshot")().cast<py::array_t<unsigned char>>();
    dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
    image.write("/tmp/test_render_from_server-updated-again.png");
  }
#endif

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
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (nanosnap) { // make this a vector if it works, first saving snaps to disk then figuring out how to return the array
      thumbnail_renderer.attr("update")();
      auto npvec = thumbnail_renderer.attr("screenshot")().cast<py::array_t<unsigned char>>();
      dspacex::Image image(dspacex::toStdVec(npvec), 300, 300, 3);
      std::ostringstream os;
      os << "/tmp/nanosnap-" << std::setfill('0') << std::setw(3) << snapshotidx++ << ".png";
      image.write(os.str());
      nanosnap = false; // done!
    }
  }
  wst_cleanupServers();
  return 0;
}
