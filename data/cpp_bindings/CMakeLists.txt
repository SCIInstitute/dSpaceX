project(example)

find_package(pybind11 REQUIRED)
pybind11_add_module(example MODULE example.cpp)