CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(R)

ADD_CUSTOM_TARGET(R_mio 
    COMMAND rsync -aC ${R_SOURCE_DIR}/mio . 
    COMMAND cp 
    ${FLINALG_SOURCE_DIR}/lib/LinalgIO.h
    ${FLINALG_SOURCE_DIR}/lib/DenseMatrix.h
    ${FLINALG_SOURCE_DIR}/lib/Matrix.h
    ${FLINALG_SOURCE_DIR}/lib/Vector.h 
    ${FLINALG_SOURCE_DIR}/lib/DenseVector.h
    ./mio/src
)

ADD_CUSTOM_TARGET(R_mio_build  
    COMMAND R CMD build --resave-data mio)
ADD_DEPENDENCIES(R_mio_build R_mio)

ADD_CUSTOM_TARGET(R_mio_install
    COMMAND R CMD INSTALL mio)
ADD_DEPENDENCIES(R_mio_install R_mio)

ADD_CUSTOM_TARGET(R_mio_check 
    COMMAND R CMD check mio)
ADD_DEPENDENCIES(R_mio_check R_mio)



