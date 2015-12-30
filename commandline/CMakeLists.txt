PROJECT(DIAT_Data_Tools)

FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)

LINK_DIRECTORIES( ${ANN_LINK_DIR} )

ADD_EXECUTABLE(SymmetricEigensystem SymmetricEigensystem.cxx)
TARGET_LINK_LIBRARIES (SymmetricEigensystem blas lapack gfortran)

ADD_EXECUTABLE(Isomap Isomap.cxx)
TARGET_LINK_LIBRARIES (Isomap gfortran lapack blas)

ADD_EXECUTABLE(MetricMDS MetricMDS.cxx)
TARGET_LINK_LIBRARIES (MetricMDS gfortran lapack blas)

ADD_EXECUTABLE(PCA PCA.cxx)
TARGET_LINK_LIBRARIES (PCA gfortran lapack blas)

ADD_EXECUTABLE(SVD SVD.cxx)
TARGET_LINK_LIBRARIES (SVD gfortran lapack blas)

ADD_EXECUTABLE(MultiscaleEntropy MultiscaleEntropy.cxx)
TARGET_LINK_LIBRARIES (MultiscaleEntropy)

ADD_EXECUTABLE(EigenRandomSVD EigenRandomSVD.cxx)
TARGET_LINK_LIBRARIES (EigenRandomSVD)

ADD_EXECUTABLE(EigenStreamingRandomSVD EigenStreamingRandomSVD.cxx)
TARGET_LINK_LIBRARIES (EigenStreamingRandomSVD)

ADD_EXECUTABLE(EigenRowRandomSVD EigenRowRandomSVD.cxx)
TARGET_LINK_LIBRARIES (EigenRowRandomSVD)

ADD_EXECUTABLE(SANN SANN.cxx)
TARGET_LINK_LIBRARIES (SANN gfortran lapack blas)

ADD_EXECUTABLE(KernelMetric KernelMetric.cxx)
TARGET_LINK_LIBRARIES (KernelMetric gfortran lapack blas)

ADD_EXECUTABLE(GMRA GMRA.cxx)
TARGET_LINK_LIBRARIES (GMRA gfortran lapack blas)

ADD_EXECUTABLE(MultiscaleEMD MultiscaleEMD.cxx)
TARGET_LINK_LIBRARIES (MultiscaleEMD gfortran lapack blas glpk)

ADD_EXECUTABLE(Rips Rips.cxx)
TARGET_LINK_LIBRARIES (Rips gfortran lapack blas )

ADD_EXECUTABLE(IPCARips IPCARips.cxx)
TARGET_LINK_LIBRARIES (IPCARips)

ADD_EXECUTABLE(MultiscaleRips MultiscaleRips.cxx)
TARGET_LINK_LIBRARIES (MultiscaleRips)

ADD_EXECUTABLE(IPCARipsHarer IPCARipsHarer.cxx)
TARGET_LINK_LIBRARIES (IPCARipsHarer gfortran lapack blas )

ADD_EXECUTABLE(BayesGMRA BayesGMRA.cxx)
TARGET_LINK_LIBRARIES (BayesGMRA gfortran lapack blas)

ADD_EXECUTABLE(KernelDensity KernelDensity.cxx)
TARGET_LINK_LIBRARIES (KernelDensity gfortran lapack blas)


#ADD_EXECUTABLE(my_cxx my_cxx.cxx)
#TARGET_LINK_LIBRARIES (my_cxx)

#ADD_EXECUTABLE(my_class my_class.cxx)
#TARGET_LINK_LIBRARIES (my_class)

#ADD_EXECUTABLE(test_G test_G.cxx)
#TARGET_LINK_LIBRARIES (test_G)

#ADD_EXECUTABLE(KMMA_optimize KMMA_optimize.cxx)
#TARGET_LINK_LIBRARIES (KMMA_optimize gfortran lapack blas)

  
ADD_EXECUTABLE(FastCEM FastCEM.cxx)
TARGET_LINK_LIBRARIES (FastCEM gfortran lapack blas myann)

  
ADD_EXECUTABLE(BlockCEM BlockCEM.cxx)
TARGET_LINK_LIBRARIES (BlockCEM gfortran lapack blas myann)




ADD_EXECUTABLE(KMM_optimize KMM_optimize.cxx)
TARGET_LINK_LIBRARIES (KMM_optimize gfortran lapack blas)

ADD_EXECUTABLE(UKR_optimize UKR_optimize.cxx)
TARGET_LINK_LIBRARIES (UKR_optimize gfortran lapack blas)

ADD_EXECUTABLE(KMMRegression_optimize KMMRegression_optimize.cxx)
TARGET_LINK_LIBRARIES (KMMRegression_optimize gfortran lapack blas)

ADD_EXECUTABLE(CEMRegression_optimize CEMRegression_optimize.cxx)
TARGET_LINK_LIBRARIES (CEMRegression_optimize gfortran lapack blas)

ADD_EXECUTABLE(MRegression MRegression.cxx)
TARGET_LINK_LIBRARIES (MRegression gfortran lapack blas)

ADD_EXECUTABLE(MSRegression MSRegression.cxx)
TARGET_LINK_LIBRARIES (MSRegression gfortran lapack blas)

ADD_EXECUTABLE(KMM KMM.cxx)
TARGET_LINK_LIBRARIES (KMM gfortran lapack blas)

ADD_EXECUTABLE(UKR UKR.cxx)
TARGET_LINK_LIBRARIES (UKR gfortran lapack blas)

ADD_EXECUTABLE(Matrix2Obj ObjFromMatrix.cxx)
TARGET_LINK_LIBRARIES (Matrix2Obj gfortran lapack blas)

ADD_EXECUTABLE(Matrix2CSV MatrixToCSV.cxx)
TARGET_LINK_LIBRARIES (Matrix2CSV gfortran lapack blas)


ADD_EXECUTABLE(DCD2Matrix  DCD2Matrix.cxx)
TARGET_LINK_LIBRARIES (DCD2Matrix wordom)

ADD_EXECUTABLE(CSV2Matrix  CSVToMatrix.cxx)
TARGET_LINK_LIBRARIES (CSV2Matrix)

ADD_EXECUTABLE(PSDataDenoise PSDenoise.cxx)
TARGET_LINK_LIBRARIES (PSDataDenoise gfortran lapack blas)

ADD_EXECUTABLE(KMMMixture KMMMixture.cxx)
TARGET_LINK_LIBRARIES (KMMMixture gfortran lapack blas)

ADD_EXECUTABLE(TmpExe Tmp2.cxx)
TARGET_LINK_LIBRARIES (TmpExe gfortran lapack blas)

ADD_EXECUTABLE(UKR2 UKR2.cxx)
TARGET_LINK_LIBRARIES (UKR2 gfortran lapack blas)

ADD_EXECUTABLE(Batch Batch.cxx)
TARGET_LINK_LIBRARIES ( Batch )

ADD_EXECUTABLE(Batch2 Batch2.cxx)
TARGET_LINK_LIBRARIES ( Batch2 )

ADD_EXECUTABLE(ManifoldNN ManifoldNN.cxx)
TARGET_LINK_LIBRARIES ( ManifoldNN gfortran lapack blas )
  
ADD_EXECUTABLE(SampleSinusoid SampleSinusoid.cxx)
TARGET_LINK_LIBRARIES ( SampleSinusoid )

ADD_EXECUTABLE(MatrixMult MatrixMultiplication.cxx)
TARGET_LINK_LIBRARIES (MatrixMult gfortran lapack blas)

ADD_EXECUTABLE(NNMSComplex NNMSComplex.cxx)
TARGET_LINK_LIBRARIES (NNMSComplex gfortran lapack blas  myann)

ADD_EXECUTABLE(NNMSComplex2 NNMSComplex2.cxx)
TARGET_LINK_LIBRARIES (NNMSComplex2 gfortran lapack blas myann)

ADD_EXECUTABLE(JoinMatrices JoinMatrices.cxx)
TARGET_LINK_LIBRARIES (JoinMatrices)

ADD_EXECUTABLE(SortMatrix SortMatrix.cxx)
TARGET_LINK_LIBRARIES (SortMatrix)


  option(BUILD_TRANSPORT "Build GMRA optimal transport" ON)
if(BUILD_TRANSPORT)
FIND_PACKAGE(CPLEX)
INCLUDE_DIRECTORIES( ${CPLEX_INCLUDE_DIRS} )


ADD_EXECUTABLE(TransportLP TransportLP.cxx)
TARGET_LINK_LIBRARIES (TransportLP gfortran lapack blas ${CPLEX_LIBRARIES} pthread)

endif(BUILD_TRANSPORT)

ADD_EXECUTABLE(Zscores Zscores.cxx)
TARGET_LINK_LIBRARIES (Zscores)

ADD_EXECUTABLE(SinkhornTransport SinkhornTransport.cxx)
TARGET_LINK_LIBRARIES (SinkhornTransport)

  
ADD_EXECUTABLE(SparseSinkhornTransport SparseSinkhornTransport.cxx)
TARGET_LINK_LIBRARIES (SparseSinkhornTransport)





ADD_CUSTOM_TARGET(MATLAB_linalgio 
    COMMAND mkdir -p ./matlab-io/ 
    COMMAND cp 
${DIAT_LIB_DIR}/linalg/DenseMatrix.h
${DIAT_LIB_DIR}/linalg/Matrix.h 
${DIAT_LIB_DIR}/linalg/Vector.h 
${DIAT_LIB_DIR}/linalg/DenseVector.h
${DIAT_LIB_DIR}/io/LinalgIO.h
${DIAT_LIB_DIR}/io/compile.m
${DIAT_LIB_DIR}/io/ReadLinalgMatrix.cpp
${DIAT_LIB_DIR}/io/ReadLinalgVector.cpp
${DIAT_LIB_DIR}/io/WriteLinalgMatrix.cpp
${DIAT_LIB_DIR}/io/WriteLinalgVector.cpp
./matlab-io/
    COMMAND tar -c matlab-io > matlab-io.tar
)
