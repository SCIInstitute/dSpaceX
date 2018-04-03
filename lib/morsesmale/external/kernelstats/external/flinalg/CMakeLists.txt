CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(FLinalg)

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

INCLUDE_DIRECTORIES(lib external/utils/lib)

ADD_SUBDIRECTORY(commandline)
ADD_SUBDIRECTORY(Rpackage)

ADD_CUSTOM_TARGET(MATLAB_flinalgio 
    COMMAND mkdir -p ./matlab-flinalgio/ 
    COMMAND cp 
lib/DenseMatrix.h
lib/Matrix.h 
lib/Vector.h 
lib/DenseVector.h
lib/LinalgIO.h
matlab/compile.m
matlab/ReadLinalgMatrix.cpp
matlab/ReadLinalgVector.cpp
matlab/WriteLinalgMatrix.cpp
matlab/WriteLinalgVector.cpp
./matlab-flinalgio/
    COMMAND tar -c matlab-flinalgio > matlab-flinalgio.tar
)
