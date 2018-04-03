#include "Precision.h"

#include "LinalgIO.h"
#include "Linalg.h"
#include "DenseMatrix.h"

#include <tclap/CmdLine.h>


int main(int argc, char **argv){
  using namespace FortranLinalg;

  //Command line parsing
  TCLAP::CmdLine cmd("", ' ', "1");

  TCLAP::ValueArg<std::string> aArg("a","A","", true, "",
      "matrix header file");
  cmd.add(aArg);

  TCLAP::ValueArg<std::string> bArg("b","B","", true, "",
      "matrix header file");
  cmd.add(bArg);
  
  TCLAP::ValueArg<std::string> outArg("o","out", "output matrix", 
      true, "Z.data", "filename");
  cmd.add(outArg);

  TCLAP::SwitchArg rArg("r", "rows", "join rows");
  cmd.add(rArg);

  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }
 

  //data
  DenseMatrix<Precision> A = LinalgIO<Precision>::readMatrix(aArg.getValue());
  DenseMatrix<Precision> B = LinalgIO<Precision>::readMatrix(bArg.getValue());

  
  DenseMatrix<Precision> C;

  if( rArg.getValue() ){
    if(A.N() != B.N()){
      std::cout << "Matrices don't have same number of columns" << std::endl;
    }
    C = DenseMatrix<Precision>(A.M() + B.M(), A.N() );
    for(int i=0; i<A.M(); i++){
      for(int j=0; j<A.N(); j++){
        C(i, j) = A(i, j);
      }
    }
    for(int i=0; i<B.M(); i++){
      for(int j=0; j<B.N(); j++){
        C(A.N()+i, j) = B(i, j);
      }
    }
  }
  else{
    if(A.M() != B.M()){
      std::cout << "Matrices don't have same number of rows" << std::endl;
    }
    C = DenseMatrix<Precision>(A.M(), A.N() + B.N());
    for(int i=0; i<A.M(); i++){
      for(int j=0; j<A.N(); j++){
        C(i, j) = A(i, j);
      }
    }
    for(int i=0; i<B.M(); i++){
      for(int j=0; j<B.N(); j++){
        C(i, A.N()+j) = B(i, j);
      }
    }

  }

  LinalgIO<Precision>::writeMatrix(outArg.getValue(), C);
 
  return 0;
}
