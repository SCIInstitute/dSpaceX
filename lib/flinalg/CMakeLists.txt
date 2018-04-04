CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(FLinalg)

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

OPTION(BUILD_FLINALG_CLI "Build flinagl-cli" OFF)

if(BUILD_FLINALG_CLI)
  ADD_SUBDIRECTORY(commandline)
endif()

ADD_CUSTOM_TARGET(MATLAB_flinalgio 
    COMMAND mkdir -p ./matlab-flinalgio/ 
    COMMAND cp 
DenseMatrix.h
Matrix.h 
Vector.h 
DenseVector.h
LinalgIO.h
matlab/compile.m
matlab/ReadLinalgMatrix.cpp
matlab/ReadLinalgVector.cpp
matlab/WriteLinalgMatrix.cpp
matlab/WriteLinalgVector.cpp
./matlab-flinalgio/
    COMMAND tar -c matlab-flinalgio > matlab-flinalgio.tar
)
