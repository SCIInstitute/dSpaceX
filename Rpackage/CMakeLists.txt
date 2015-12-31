CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(R)


######## msr package #########

ADD_CUSTOM_TARGET(R_msr 
    COMMAND rsync -aC ${R_SOURCE_DIR}/msr . 
    COMMAND rsync -aC ${CMAKE_LIB_DIR}/ann/ ./msr/src 
    COMMAND cp 
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/DenseMatrix.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/EuclideanMetric.h 
    ${CMAKE_LIB_DIR}/external/kernelstats/external/metrics/lib/Distance.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/metrics/lib/KernelDensity.h 
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/Linalg.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/LapackDefs.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/Matrix.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/metrics/lib/Metric.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/metrics/lib/SquaredEuclideanMetric.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/Vector.h 
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/lib/DenseVector.h
    ${CMAKE_LIB_DIR}/lib/GaussianKernel.h
    ${CMAKE_LIB_DIR}/external/utils/lib/Heap.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/metrics/lib/Kernel.h
    ${CMAKE_LIB_DIR}/external/utils/lib/MaxHeap.h
    ${CMAKE_LIB_DIR}/external/utils/lib/MinHeap.h
    ${CMAKE_LIB_DIR}/lib/NNMSComplex.h
    ${CMAKE_LIB_DIR}/lib/NNMSComplex2old.h
    ${CMAKE_LIB_DIR}/external/kernelstats/external/flinalg/LinalgIO.h
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






