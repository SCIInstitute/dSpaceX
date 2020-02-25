enable_testing()
find_package(GTest REQUIRED)
find_package(yaml-cpp REQUIRED)

# Definitions for Tests
function(NEWTEST name)
  add_executable(${name} ${name}.cpp ${ARGN})
  # link to gtest and all the dspacex libraries (fixme: will this work for testing one of the libraries)
  target_link_libraries(${name} GTest::GTest GTest::Main  
    hdprocess yaml-cpp imageutils dspacex_utils dspacex
    -lpthread)
  add_test(${name} ${name})
endfunction()

# The tests themselves
add_subdirectory(lib)
