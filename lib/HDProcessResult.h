#pragma once

#include "Precision.h"
#include "Linalg.h"


 /**
  * All output data generated from the HDProcessor. 
  */
struct HDProcessor {
  FortranLinalg::DenseMatrix<TPrecision> X;
  FortranLinalg::DenseMatrix<TPrecision> Y;
};