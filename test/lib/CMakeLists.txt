ADD_DEFINITIONS(-DTEST_DATA_DIR="\\"${CMAKE_SOURCE_DIR}/test/data\\"")
ADD_DEFINITIONS(-DEXAMPLE_DATA_DIR="\\"${CMAKE_SOURCE_DIR}/examples\\"")  # manually create a symlink to /usr/sci/projects/dspacex?

include_directories(${CMAKE_SOURCE_DIR}/server ${EIGEN3_INCLUDE_DIR})

newtest(HDVizData_tests)
newtest(DataLoader_tests)

TARGET_LINK_LIBRARIES(DataLoader_tests
pmodels
)
