ADD_EXECUTABLE(KernelDensity KernelDensity.cxx)
TARGET_LINK_LIBRARIES (KernelDensity gfortran lapack blas)
