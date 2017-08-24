cmake_minimum_required(VERSION 2.6)

PROJECT(HDVizProcessing)

FIND_PACKAGE(GLUT)
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(FreeType)
FIND_PACKAGE(FTGL)
FIND_PACKAGE(Threads)
#FIND_PACKAGE(GLE)

set(CMAKE_BUILD_TYPE "Debug")

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)
LINK_DIRECTORIES( ${ANN_LINK_DIR} )
ADD_EXECUTABLE(HDVizProcessing HDVizProcess.cxx)
TARGET_LINK_LIBRARIES (HDVizProcessing 
    hdprocess
    blas
    lapack    
    gfortran
    ANN 
)