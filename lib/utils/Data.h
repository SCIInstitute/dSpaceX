#pragma once

#include <vector>

#include <pybind11/embed.h>
#include <pybind11/eigen.h>
namespace py = pybind11;

namespace dspacex {

/*
 * copy contents of pyvec into a standard vector
 */
template<typename T>
std::vector<T> toStdVec(const py::array_t<T>& pyvec) {
  py::buffer_info info = pyvec.request();
  int n = 1;
  for (auto r: info.shape) {
    n *= r;
  }
  auto ptr = static_cast<T*>(info.ptr);
  return std::vector<T>(ptr, ptr + n);
}



} // dspacex
