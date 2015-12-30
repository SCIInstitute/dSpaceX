#ifndef NULL
#define NULL 0
#endif

#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <stdio.h>
//#include <Rdefines.h>

#include "LinalgIO.h"

extern "C" {


SEXP write_matrix_double(SEXP Rm, SEXP Rn, SEXP Rx, SEXP Rfile) {
  using namespace FortranLinalg;
  try{ 
  
  double *x = REAL(Rx);
  int m = *INTEGER(Rm);
  int n = *INTEGER(Rn);
  const char *file = CHAR(STRING_ELT(Rfile,0));
 
  DenseMatrix<double> X(m, n, x);


  LinalgIO<double>::writeMatrix(std::string(file), X);
  }
  catch(...){
  }

  return R_NilValue;  
}


SEXP read_matrix_double(SEXP Rfile) {
  using namespace FortranLinalg;
  const char *file = CHAR(STRING_ELT(Rfile,0));
 
  try{ 
    DenseMatrix<double> X = LinalgIO<double>::readMatrix(std::string(file));


   SEXP Xret;
   PROTECT(Xret = Rf_allocMatrix(REALSXP, X.M(), X.N()));
    memcpy( REAL(Xret), X.data(), X.M()*X.N()*sizeof(double) );
   UNPROTECT(1);


   X.deallocate();
 
   return Xret;  
  }
  catch(...){
    return R_NilValue;
  }
  
}




SEXP write_matrix_int(SEXP Rm, SEXP Rn, SEXP Rx, SEXP Rfile) {
  using namespace FortranLinalg;
    try{ 

  int *x = INTEGER(Rx);
  int m = *INTEGER(Rm);
  int n = *INTEGER(Rn);
  const char *file = CHAR(STRING_ELT(Rfile,0));
 
  DenseMatrix<int> X(m, n, x);


  LinalgIO<int>::writeMatrix(std::string(file), X);
  }
  catch(...){
  }

  return R_NilValue;
}



SEXP read_matrix_int(SEXP Rfile) { 
  using namespace FortranLinalg;
 try{ 

  const char *file = CHAR(STRING_ELT(Rfile,0));
  
  DenseMatrix<int> X = LinalgIO<int>::readMatrix(std::string(file));


  SEXP Xret;
  PROTECT(Xret = Rf_allocMatrix(INTSXP, X.M(), X.N()));
  memcpy( INTEGER(Xret), X.data(), X.M()*X.N()*sizeof(int) );
  UNPROTECT(1);


  X.deallocate();

  return Xret;    
}
  catch(...){
    return R_NilValue;
  }
  

}






SEXP write_vector_double(SEXP Rn, SEXP Rx, SEXP Rfile) {
  using namespace FortranLinalg;
  try{
  double *x = REAL(Rx);
  int n = *INTEGER(Rn);
  const char *file = CHAR(STRING_ELT(Rfile,0));
 
  DenseVector<double> X(n, x);

  LinalgIO<double>::writeVector(std::string(file), X);
}
catch(...){}

  return R_NilValue;
}



SEXP read_vector_double(SEXP Rfile) {
  using namespace FortranLinalg;
try{
  const char *file = CHAR(STRING_ELT(Rfile,0));
  
  DenseVector<double> X = LinalgIO<double>::readVector(std::string(file));


  SEXP Xret;
  PROTECT(Xret = Rf_allocVector(REALSXP, X.N()));
  memcpy( REAL(Xret), X.data(), X.N()*sizeof(double) );
  UNPROTECT(1);


  X.deallocate();

  return Xret;   
}
  catch(...){
    return R_NilValue;
  }
  
 
}



SEXP write_vector_int(SEXP Rn, SEXP Rx, SEXP Rfile) {
  using namespace FortranLinalg;
  try{
  int *x = INTEGER(Rx);
  int n = *INTEGER(Rn);
  const char *file = CHAR(STRING_ELT(Rfile,0));
 
  DenseVector<int> X(n, x);


  LinalgIO<int>::writeVector(std::string(file), X);
  }
  catch(...){
  }

  return R_NilValue;
}



SEXP read_vector_int(SEXP Rfile) {
  using namespace FortranLinalg;
try{
  const char *file = CHAR(STRING_ELT(Rfile,0));
  
  DenseVector<int> X = LinalgIO<int>::readVector(std::string(file));


  SEXP Xret;
  PROTECT(Xret = Rf_allocVector(INTSXP, X.N()));
  memcpy( INTEGER(Xret), X.data(), X.N()*sizeof(int) );
  UNPROTECT(1);


  X.deallocate();

  return Xret;  
}
  catch(...){
    return R_NilValue;
  }

}


}//end extern C
