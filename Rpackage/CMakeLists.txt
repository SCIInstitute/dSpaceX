PROJECT(R)



############## cem package ##############

ADD_CUSTOM_TARGET(R_cems 
    COMMAND rsync -aC ${R_SOURCE_DIR}/cems . 
    COMMAND rsync -aC ${DIMRED_SORCE_DIR}/external/annmod/lib ./cems/src 
    COMMAND cp 
    ${DIMRED_SOURCE_DIR}/external/utils/lib/Random.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/Matrix.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/DenseMatrix.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/Vector.h 
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/DenseVector.h
    ${DIMRED_SOURCE_DIR}/external/metrics/lib/Metric.h
    ${DIMRED_SOURCE_DIR}/external/metrics/lib/EuclideanMetric.h 
    ${DIMRED_SOURCE_DIR}/external/metrics/lib/SquaredEuclideanMetric.h 
    ${DIMRED_SOURCE_DIR}/external/utils/lib/Geometry.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/Linalg.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/LapackDefs.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/SymmetricEigensystem.h
    ${DIMRED_SOURCE_DIR}/external/flinalg/lib/SymmetricDenseMatrix.h
    ${DIMRED_SOURCE_DIR}/external/kernelstats/lib/Kernel.h
    ${DIMRED_SOURCE_DIR}/external/kernelstats/lib/GaussianKernel.h
    ${DIMRED_SOURCE_DIR}/external/kernelstats/lib/MahalanobisKernel.h
    ${DIMRED_SOURCE_DIR}/external/kernelstats/lib/KernelDensity.h
    ${DIMRED_SOURCE_DIR}/lib/CEM.h
    ${DIMRED_SOURCE_DIR}/lib/FastCEM.h
    ${DIMRED_SOURCE_DIR}/BlockCEM.h
    ${DIMRED_SOURCE_DIR}/lib/CEMRegression.h
    ${DIMRED_SOURCE_DIR}/external/utils/lib/Heap.h
    ${DIMRED_SOURCE_DIR}/external/utils/lib/MaxHeap.h
    ${DIMRED_SOURCE_DIR}/external/utils/lib/MinHeap.h
    ./cems/src
    COMMAND rm ./cems/src/CMakeLists.txt
    COMMAND rm ./cems/src/perf.cpp
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






