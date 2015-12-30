#include "Precision.h"

#include "LinalgIO.h"
#include "Linalg.h"
#include "DenseMatrix.h"
#include "Linalg.h"

#include <tclap/CmdLine.h>


int main(int argc, char **argv){

  using namespace FortranLinalg;

  //Command line parsing
  TCLAP::CmdLine cmd("", ' ', "1");

  TCLAP::ValueArg<std::string> aArg("a","A","", true, "",
      "matrix header file");
  cmd.add(aArg);

  TCLAP::ValueArg<std::string> outArg("o","out", "output matrix", 
      true, "Z.data", "filename");
  cmd.add(outArg);

  TCLAP::SwitchArg rArg("r", "rows", "sort rows");
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
  

  if( rArg.getValue() ){
    for(int i=0; i < A.M(); i++){
      DenseVector<Precision> x = Linalg<Precision>::ExtractRow(A, i);
      std::sort( x.data(), x.data()+x.N() );
      Linalg<Precision>::SetRow(A, i, x);
      x.deallocate();
    }
  }
  else{
    for(int i=0; i < A.N(); i++){
      DenseVector<Precision> x = Linalg<Precision>::ExtractColumn(A, i);
      std::sort( x.data(), x.data()+x.N() );
      Linalg<Precision>::SetColumn(A, i, x);
      x.deallocate();
    }
  }

  LinalgIO<Precision>::writeMatrix(outArg.getValue(), A);
 
  return 0;
}
