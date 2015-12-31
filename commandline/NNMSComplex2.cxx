#include "Precision.h"

#include <tclap/CmdLine.h>

#include "NNMSComplex2old.h"


#include "LinalgIO.h"


int main(int argc, char **argv){

  //Command line parsing
  TCLAP::CmdLine cmd("Nearest neighbor Morse-Smale approximation tool", ' ', "1");

  TCLAP::ValueArg<std::string> xArg("x","X","Domain samples", true, "",
      "matrix header file");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> yArg("y","Y","Function values", true, "",
      "vector header file");
  cmd.add(yArg);
 
  TCLAP::ValueArg<int> knnArg("k","knn","Number of nearest neighbors", false, 15,
      "integer");
  cmd.add(knnArg);

  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }


  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(xArg.getValue());
  DenseVector<Precision> y = LinalgIO<Precision>::readVector(yArg.getValue());
  int knn = knnArg.getValue();
  

  NNMSComplexR2<Precision> msc(X, y, knn);

  DenseVector<int> crystals = msc.getPartitions();
  DenseMatrix<int> extrema = msc.getExtrema();
 
  std::cout << msc.getNAllExtrema() << std::endl;



  LinalgIO<int>::writeVector("crystals.data", crystals);
  LinalgIO<int>::writeMatrix("extrema.data",    extrema);
   
  crystals.deallocate();
  extrema.deallocate();
  msc.cleanup();
  X.deallocate();
  y.deallocate();
  return 0;
}
