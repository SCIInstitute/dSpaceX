//
// Created by Kyli McKay-Bishop on 5/28/20.
//
#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;
PYBIND11_MODULE(my_example, m) {
    m.def("add", &add, "adds two number");
}
