cmake_minimum_required(VERSION 2.6)

PROJECT(HDViz)

FIND_PACKAGE(GLUT)
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(FreeType)
FIND_PACKAGE(FTGL)


option(BUILD_HDVIZ_IMAGE "Build hdviz for image data" OFF)
option(BUILD_HDVIZPORCESSING "Build HDViz preprocessing tool" OFF)
 
# Use itk.  This sets up include/link directories for us.
if(BUILD_HDVIZ_IMAGE)
# Find common/itk
INCLUDE (${CMAKE_ROOT}/Modules/FindITK.cmake)
IF (USE_ITK_FILE)
  INCLUDE(${USE_ITK_FILE})
ENDIF(USE_ITK_FILE) 
INCLUDE_DIRECTORIES(${ITK_INCLUDE_DIR})
endif(BUILD_HDVIZ_IMAGE)

LINK_DIRECTORIES( ${GLE_LINK_DIR} )

INCLUDE_DIRECTORIES(${FTGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIR_ft2build}  ${FREETYPE_INCLUDE_DIR_freetype2} ${GLUT_INCLUDE_DIR} )


ADD_EXECUTABLE(HDViz HDViz.cpp )
TARGET_LINK_LIBRARIES (HDViz 
    mygle 
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}
)
SET_TARGET_PROPERTIES(HDViz PROPERTIES COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf) 


if(BUILD_HDVIZ_IMAGE)
ADD_EXECUTABLE(HDViz_3D HDViz.cpp )
TARGET_LINK_LIBRARIES (HDViz_3D 
    mygle 
    gfortran
    blas
    lapack   
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}    
    ${ITK_LIBRARIES}
)

ADD_EXECUTABLE(HDViz_2D HDViz.cpp )
TARGET_LINK_LIBRARIES (HDViz_2D 
    mygle 
    gfortran
    blas
    lapack   
    ${FTGL_LIBRARIES}  
    ${FREETYPE_LIBRARY}  
    ${OPENGL_LIBRARIES}  
    ${GLUT_LIBRARIES}
    ${ITK_LIBRARIES}
)
SET_TARGET_PROPERTIES( HDViz_2D PROPERTIES COMPILE_DEFINITIONS DIMENSION=2 COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf)
SET_TARGET_PROPERTIES( HDViz_3D PROPERTIES COMPILE_DEFINITIONS DIMENSION=3 COMPILE_FLAGS -DFONTNAME=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf)
endif(BUILD_HDVIZ_IMAGE)



if(BUILD_HDVIZPORCESSING)
FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)
ADD_EXECUTABLE(HDVizProcessing HDVizProcess.cxx)
TARGET_LINK_LIBRARIES (HDVizProcessing 
    blas
    lapack    
    gfortran
    myann 
)
endif(BUILD_HDVIZPORCESSING)

ADD_CUSTOM_TARGET(HDViz_release 
    COMMAND rsync -aC ${HDViz_SOURCE_DIR}/ ./hdviz_src/
    COMMAND rsync -aC ${DIAT_LIB_DIR}/gle/ ./hdviz_src/gle/ 
    COMMAND rsync -aC ${DIAT_LIB_DIR}/ann/ ./hdviz_src/ann/ 
    COMMAND cp 
${DIAT_SOURCE_DIR}/cmake_modules/*
${DIAT_LIB_DIR}/linalg/DenseMatrix.h
${DIAT_LIB_DIR}/metrics/EuclideanMetric.h 
${DIAT_LIB_DIR}/metrics/SquaredEuclideanMetric.h 
${DIAT_LIB_DIR}/utils/Geometry.h
${DIAT_LIB_DIR}/statistics/UKR.h 
${DIAT_LIB_DIR}/statistics/Isomap.h 
${DIAT_LIB_DIR}/statistics/MetricMDS.h 
${DIAT_LIB_DIR}/linalg/Linalg.h
${DIAT_LIB_DIR}/linalg/LapackDefs.h
${DIAT_LIB_DIR}/linalg/Matrix.h   
${DIAT_LIB_DIR}/metrics/Metric.h
${DIAT_LIB_DIR}/linalg/Vector.h 
${DIAT_LIB_DIR}/linalg/DenseVector.h
${DIAT_LIB_DIR}/linalg/SparseMatrix.h
${DIAT_LIB_DIR}/linalg/SymmetricDenseMatrix.h
${DIAT_LIB_DIR}/statistics/GaussianKernel.h
${DIAT_LIB_DIR}/statistics/Random.h
${DIAT_LIB_DIR}/datastructures/heap/Heap.h 
${DIAT_LIB_DIR}/statistics/Kernel.h
${DIAT_LIB_DIR}/statistics/KernelDensity.h
${DIAT_LIB_DIR}/statistics/KernelRegression.h
${DIAT_LIB_DIR}/statistics/KernelInterpolator.h
${DIAT_LIB_DIR}/statistics/FirstOrderKernelRegression.h
${DIAT_LIB_DIR}/datastructures/heap/MaxHeap.h
${DIAT_LIB_DIR}/datastructures/heap/MinHeap.h
${DIAT_LIB_DIR}/datastructures/graph/Neighborhood.h
${DIAT_LIB_DIR}/datastructures/graph/KNNNeighborhood.h
${DIAT_LIB_DIR}/datastructures/graph/GraphAlgorithms.h
${DIAT_LIB_DIR}/topology/NNMSComplex.h 
${DIAT_LIB_DIR}/io/LinalgIO.h
${DIAT_LIB_DIR}/io/ImageIO.h
${DIAT_LIB_DIR}/utils/ImageVectorConverter.h
${DIAT_LIB_DIR}/statistics/PCA.h
${DIAT_LIB_DIR}/linalg/SymmetricEigensystem.h   
./hdviz_src/   
    COMMAND rsync -aC ${DIAT_SOURCE_DIR}/tools/config/ ./hdviz_src/config/
    COMMAND rsync -aC ${DIAT_SOURCE_DIR}/lib/tclap/ ./hdviz_src/tclap/
    COMMAND mv ./hdviz_src/CMakeListsRelease.txt ./hdviz_src/CMakeLists.txt
    COMMAND tar -c hdviz_src > hdviz_src.tar
)

ADD_CUSTOM_TARGET(HDViz_bins
  COMMAND rsync -aC HDVizProcessing ./hdviz_bin/
  COMMAND rsync -aC ${HDViz_SOURCE_DIR}/README ./hdviz_bin/
  COMMAND rsync -aC ${HDViz_SOURCE_DIR}/COMPILING ./hdviz_bin/
  COMMAND rsync -aC HDViz ./hdviz_bin/
  COMMAND rsync -aC ${HDViz_SOURCE_DIR}/examples/ ./hdviz_bin/examples/
  COMMAND tar -c hdviz_bin > hdviz_bin.tar)

ADD_DEPENDENCIES(HDViz_bins HDViz HDVizProcessing)

