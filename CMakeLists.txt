CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(MorseSmaleComplex)

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)


INCLUDE_DIRECTORIES( lib 
                     external/kernelstats/lib 
                     external/kernelstats/external/flinalg/lib
                     external/kernelstats/external/metrics/lib
                    )

ADD_SUBDIRECTORY(commandline)
ADD_SUBDIRECTORY(Rpackage)

