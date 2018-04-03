CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(DimRed)

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

ADD_SUBDIRECTORY(external/annmod/lib/ann)

INCLUDE_DIRECTORIES( lib 
                     external/kernelstats/lib 
                     external/kernelstats/external/flinalg/lib
                     external/kernelstats/external/metrics/lib
                     external/utils/lib 
                     external/graph/lib                      
                     ${ANN_INCLUDE_FILES} 
                    )

ADD_SUBDIRECTORY(commandline)
