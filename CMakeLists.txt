CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(HDVis)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/")



INCLUDE_DIRECTORIES(
                     external/gle/lib 
                     external/graph/lib 
                     external/imageutils/lib 
                     external/morsesmale/external/annmod/lib 
                     external/morsesmale/external/annmod/lib/ann 
                     external/morsesmale/external/utils/lib 
                     external/morsesmale/external/kernelstats/lib 
                     external/morsesmale/external/kernelstats/external/metrics/lib 
                     external/morsesmale/external/kernelstats/external/flinalg/lib 
                     ${ANN_INCLUDE_FILES} 
                     )
ADD_SUBDIRECTORY(external/gle)
ADD_SUBDIRECTORY(commandline)

