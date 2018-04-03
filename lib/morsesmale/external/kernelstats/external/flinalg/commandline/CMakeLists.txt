FIND_PACKAGE(LAPACK)
FIND_PACKAGE(BLAS)


ADD_EXECUTABLE(SymmetricEigensystem SymmetricEigensystem.cxx)
TARGET_LINK_LIBRARIES (SymmetricEigensystem blas lapack gfortran)

ADD_EXECUTABLE(SVD SVD.cxx)
TARGET_LINK_LIBRARIES (SVD gfortran lapack blas)

#ADD_EXECUTABLE(Matrix2Obj ObjFromMatrix.cxx)
#TARGET_LINK_LIBRARIES (Matrix2Obj gfortran lapack blas)

ADD_EXECUTABLE(Matrix2CSV MatrixToCSV.cxx)
TARGET_LINK_LIBRARIES (Matrix2CSV gfortran lapack blas)

#ADD_EXECUTABLE(DCD2Matrix  DCD2Matrix.cxx)
#TARGET_LINK_LIBRARIES (DCD2Matrix wordom)

ADD_EXECUTABLE(CSV2Matrix  CSVToMatrix.cxx)
TARGET_LINK_LIBRARIES (CSV2Matrix)

#ADD_EXECUTABLE(MatrixMult MatrixMultiplication.cxx)
#TARGET_LINK_LIBRARIES (MatrixMult gfortran lapack blas)

ADD_EXECUTABLE(JoinMatrices JoinMatrices.cxx)
TARGET_LINK_LIBRARIES (JoinMatrices)

ADD_EXECUTABLE(SortMatrix SortMatrix.cxx)
TARGET_LINK_LIBRARIES (SortMatrix)


