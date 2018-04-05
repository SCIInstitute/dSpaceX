
FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

ADD_EXECUTABLE(MSRegression MSRegression.cxx)
TARGET_LINK_LIBRARIES( MSRegression gfortran lapack blas )

ADD_EXECUTABLE(NNMSComplex NNMSComplex.cxx)
TARGET_LINK_LIBRARIES( NNMSComplex gfortran lapack blas )

ADD_EXECUTABLE(NNMSComplex2 NNMSComplex2.cxx)
TARGET_LINK_LIBRARIES( NNMSComplex2 gfortran lapack blas ANN)

