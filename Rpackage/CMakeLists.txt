PROJECT(R)



############## cem package ##############

ADD_CUSTOM_TARGET(R_cems 
    COMMAND rsync -aC ${Rcode_SOURCE_DIR}/cems . 
    COMMAND rsync -aC ${DIAT_LIB_DIR}/ann/ ./cems/src 
    COMMAND cp 
    ${DIAT_LIB_DIR}/statistics/Random.h
    ${DIAT_LIB_DIR}/linalg/Matrix.h
    ${DIAT_LIB_DIR}/linalg/DenseMatrix.h
    ${DIAT_LIB_DIR}/linalg/Vector.h 
    ${DIAT_LIB_DIR}/linalg/DenseVector.h
    ${DIAT_LIB_DIR}/metrics/Metric.h
    ${DIAT_LIB_DIR}/metrics/EuclideanMetric.h 
    ${DIAT_LIB_DIR}/metrics/SquaredEuclideanMetric.h 
    ${DIAT_LIB_DIR}/utils/Geometry.h
    ${DIAT_LIB_DIR}/linalg/Linalg.h
    ${DIAT_LIB_DIR}/linalg/LapackDefs.h
    ${DIAT_LIB_DIR}/linalg/SymmetricEigensystem.h
    ${DIAT_LIB_DIR}/linalg/SymmetricDenseMatrix.h
    ${DIAT_LIB_DIR}/statistics/Kernel.h
    ${DIAT_LIB_DIR}/statistics/GaussianKernel.h
    ${DIAT_LIB_DIR}/statistics/MahalanobisKernel.h
    ${DIAT_LIB_DIR}/statistics/KernelDensity.h
    ${DIAT_LIB_DIR}/statistics/CEM.h
    ${DIAT_LIB_DIR}/statistics/FastCEM.h
    ${DIAT_LIB_DIR}/statistics/BlockCEM.h
    ${DIAT_LIB_DIR}/statistics/CEMRegression.h
    ${DIAT_LIB_DIR}/datastructures/heap/Heap.h
    ${DIAT_LIB_DIR}/datastructures/heap/MaxHeap.h
    ${DIAT_LIB_DIR}/datastructures/heap/MinHeap.h
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






