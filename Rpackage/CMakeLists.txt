CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(Rcode)


######## msr package #########

ADD_CUSTOM_TARGET(R_msr 
    COMMAND rsync -aC ${Rcode_SOURCE_DIR}/msr . 
    COMMAND rsync -aC ${DIAT_LIB_DIR}/ann/ ./msr/src 
    COMMAND cp 
    ${DIAT_LIB_DIR}/linalg/DenseMatrix.h
    ${DIAT_LIB_DIR}/metrics/EuclideanMetric.h 
    ${DIAT_LIB_DIR}/utils/Geometry.h
    ${DIAT_LIB_DIR}/statistics/KernelDensity.h 
    ${DIAT_LIB_DIR}/linalg/Linalg.h
    ${DIAT_LIB_DIR}/linalg/LapackDefs.h
    ${DIAT_LIB_DIR}/linalg/Matrix.h
    ${DIAT_LIB_DIR}/metrics/Metric.h
    ${DIAT_LIB_DIR}/metrics/SquaredEuclideanMetric.h
    ${DIAT_LIB_DIR}/linalg/Vector.h 
    ${DIAT_LIB_DIR}/linalg/DenseVector.h
    ${DIAT_LIB_DIR}/statistics/GaussianKernel.h
    ${DIAT_LIB_DIR}/datastructures/heap/Heap.h
    ${DIAT_LIB_DIR}/statistics/Kernel.h
    ${DIAT_LIB_DIR}/datastructures/heap/MaxHeap.h
    ${DIAT_LIB_DIR}/datastructures/heap/MinHeap.h
    ${DIAT_LIB_DIR}/topology/NNMSComplex.h
    ${DIAT_LIB_DIR}/topology/NNMSComplex2old.h
    ${DIAT_LIB_DIR}/io/LinalgIO.h
    ./msr/src
    COMMAND rm ./msr/src/CMakeLists.txt
    COMMAND rm ./msr/src/perf.cpp
    COMMAND rm ./msr/src/kd_dump.cpp
    COMMAND mv ./msr/src/ReadMe_ANN.txt .
)

ADD_CUSTOM_TARGET(R_msr_build  
    COMMAND R CMD build --resave-data msr)
ADD_DEPENDENCIES(R_msr_build R_msr)

ADD_CUSTOM_TARGET(R_msr_install
    COMMAND R CMD INSTALL msr)
ADD_DEPENDENCIES(R_msr_install R_msr)

ADD_CUSTOM_TARGET(R_msr_check 
    COMMAND R CMD check --as-cran msr)
ADD_DEPENDENCIES(R_msr_check R_msr)






