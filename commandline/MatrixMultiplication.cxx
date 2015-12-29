#include "Precision.h"

#include "LinalgIO.h"
#include "Linalg.h"
#include "DenseMatrix.h"

#include <tclap/CmdLine.h>


int main(int argc, char **argv){

  using namespace FortranLinalg;

  //Command line parsing
  TCLAP::CmdLine cmd("Z-scores", ' ', "1");

  TCLAP::ValueArg<std::string> xArg("x","X","", true, "",
      "matrix header file");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> outArg("o","out",
      "output matrix", 
      false, "Z.data", "filename");
  cmd.add(outArg);

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

  DenseMatrix<Precision> C = Linalg<Precision>::Multiply(A, B);

  LinalgIO<Precision>::writeMatrix(outArg.getValue(), C);
 
  return 0;
}
