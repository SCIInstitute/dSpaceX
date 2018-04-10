ADD_DEFINITIONS(-DTEST_DATA_DIR="\\"${DSPACEX_SOURCE_DIR}/test/data/\\"")
ADD_DEFINITIONS(-DEXAMPLE_DATA_DIR="\\"${DSPACEX_SOURCE_DIR}/examples/\\"")

include_directories(../../server)

newtest(HDVizData_tests)
newtest(DataLoader_tests ../../server/DatasetLoader.cpp )