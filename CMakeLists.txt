CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(HDVis)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/")
find_package(Eigen3 REQUIRED)



INCLUDE_DIRECTORIES(
                     external/morsesmale/external/annmod/lib 
                     external/morsesmale/external/annmod/lib/ann 
                     external/morsesmale/external/utils/lib 
                     external/morsesmale/external/kernelstats/lib 
                     external/morsesmale/external/kernelstats/external/metrics/lib 
                     external/morsesmale/external/kernelstats/external/flinalg/lib 
                     ${EIGEN3_INCLUDE_DIR} 
                     )
ADD_SUBDIRECTORY(commandline)

