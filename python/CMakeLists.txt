find_package(pybind11 REQUIRED)

set(module_sources
  dspacexModule.cpp
  )

pybind11_add_module(dspacex_py MODULE ${module_sources})

target_link_libraries(dspacex_py PUBLIC
  pmodels
  )

set_target_properties(dspacex_py PROPERTIES
  OUTPUT_NAME dspacex
  )

install(TARGETS dspacex_py
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  )
