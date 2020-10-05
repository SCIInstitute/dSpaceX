#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
//#include <pybind11/stl_bind.h>  // look at Binding STL containers portion of manual; not sure we even use any in ShapeWorks
//#include <pybind11/operators.h>
//#include <pybind11/eigen.h>
namespace py = pybind11;
using namespace pybind11::literals;  // enables handy "argname"_a rather than py::arg("argname")

//#include <sstream>

#include "Model.h"
using namespace dspacex;

// used for demonstration of submodule creation below
int add(int i, int j=2) {
    return i + j;
}

PYBIND11_MODULE(dspacex, m)
{
  m.doc() = "dSpaceX Python API";

  //m.attr("Pi") = std::atan(1.0) * 4.0;

  // Model::Type
  py::enum_<Model::Type>(m, "ModelType")
  .value("PCA", Model::Type::None)
  .value("ShapeOdds", Model::Type::ShapeOdds)
  .value("InfShapeOdds", Model::Type::InfShapeOdds)
  .value("SharedGP", Model::Type::SharedGP)
  .value("Custom", Model::Type::Custom)
  .value("None", Model::Type::None)
  .export_values();
  ;

  // Model
  py::class_<Model>(m, "Model")
  //.def(py::init<const std::string &>()) // can the argument for init be named (it's filename in this case)
  .def(py::init(&Model::create))
  ;

  // this is simply a demonstration of creating a submodule, which may not be necessary (could add Groom, Optimize, etc)
  py::module sub_module = m.def_submodule("submodule", "dSpaceX submodule classes and functions");
  sub_module.def("add", &add, "adds two numbers", "i"_a=1, "j"_a=2);
}
