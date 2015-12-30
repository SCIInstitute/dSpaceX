LINK_DIRECTORIES( ${ANN_LINK_DIR} )

ADD_EXECUTABLE(Isomap Isomap.cxx)
TARGET_LINK_LIBRARIES (Isomap gfortran lapack blas)

ADD_EXECUTABLE(MetricMDS MetricMDS.cxx)
TARGET_LINK_LIBRARIES (MetricMDS gfortran lapack blas)

ADD_EXECUTABLE(PCA PCA.cxx)
TARGET_LINK_LIBRARIES (PCA gfortran lapack blas)

ADD_EXECUTABLE(FastCEM FastCEM.cxx)
TARGET_LINK_LIBRARIES (FastCEM gfortran lapack blas ANN)
  
ADD_EXECUTABLE(BlockCEM BlockCEM.cxx)
TARGET_LINK_LIBRARIES (BlockCEM gfortran lapack blas ANN)


