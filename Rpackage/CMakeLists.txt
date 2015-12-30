PROJECT(R)



############## cem package ##############

ADD_CUSTOM_TARGET(R_cems 
    COMMAND rsync -aC ${R_SOURCE_DIR}/cems . 
    COMMAND rsync -aC ${CMAKE_SOURCE_DIR}/external/annmod/lib/ann/ ./cems/src 
    COMMAND cp 
    ${CMAKE_SOURCE_DIR}/external/utils/lib/Random.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/Matrix.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/DenseMatrix.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/Vector.h 
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/DenseVector.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/metrics/lib/Metric.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/metrics/lib/EuclideanMetric.h 
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/metrics/lib/SquaredEuclideanMetric.h 
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/metrics/lib/Distance.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/Linalg.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/LapackDefs.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/SymmetricEigensystem.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/external/flinalg/lib/SymmetricDenseMatrix.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/lib/Kernel.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/lib/GaussianKernel.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/lib/MahalanobisKernel.h
    ${CMAKE_SOURCE_DIR}/external/kernelstats/lib/KernelDensity.h
    ${CMAKE_SOURCE_DIR}/lib/CEM.h
    ${CMAKE_SOURCE_DIR}/lib/FastCEM.h
    ${CMAKE_SOURCE_DIR}/lib/BlockCEM.h
    ${CMAKE_SOURCE_DIR}/lib/CEMRegression.h
    ${CMAKE_SOURCE_DIR}/external/utils/lib/Heap.h
    ${CMAKE_SOURCE_DIR}/external/utils/lib/MaxHeap.h
    ${CMAKE_SOURCE_DIR}/external/utils/lib/MinHeap.h
    ./cems/src
    COMMAND rm ./cems/src/CMakeLists.txt
    COMMAND rm ./cems/src/perf.cpp
    COMMAND rm ./cems/src/kd_dump.cpp
    COMMAND rm ./cems/R/cemr* 
    COMMAND rm ./cems/R/cem.old* 
    COMMAND rm ./cems/src/cems-* 
    COMMAND mv ./cems/src/ReadMe_ANN.txt .
)

ADD_CUSTOM_TARGET(R_cems_build  
    COMMAND R CMD build --resave-data cems)
ADD_DEPENDENCIES(R_cems_build R_cems)

ADD_CUSTOM_TARGET(R_cems_install
    COMMAND R CMD INSTALL  --no-multiarch  cems)
ADD_DEPENDENCIES(R_cems_install R_cems)

ADD_CUSTOM_TARGET(R_cems_check 
    COMMAND R CMD check --no-multiarch cems)
ADD_DEPENDENCIES(R_cems_check R_cems)






