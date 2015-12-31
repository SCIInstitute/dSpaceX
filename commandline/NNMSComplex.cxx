#include "Precision.h"

#include <tclap/CmdLine.h>

#include "NNMSComplex.h"


#include "LinalgIO.h"


int main(int argc, char **argv){
  using namespace FortranLinalg;

  //Command line parsing
  TCLAP::CmdLine cmd("Nearest neighbor Morse-Smale approximation tool", ' ', "1");

  TCLAP::ValueArg<std::string> xArg("x","X","Domain samples", true, "",
      "matrix header file");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> yArg("y","Y","Function values", true, "",
      "vector header file");
  cmd.add(yArg);
 
  TCLAP::ValueArg<Precision> pArg("p","persistence","Periststence level", true, 0,
      "float value");
  cmd.add(pArg);

  TCLAP::ValueArg<int> knnArg("k","knn","Number of nearest neighbors", false, 15,
      "integer");
  cmd.add(knnArg);
 
  TCLAP::ValueArg<Precision> epsArg("e","epsilon","ANN epsilon", false, 0.01,
      "float value");
  cmd.add(epsArg);

  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }


  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(xArg.getValue());
  DenseVector<Precision> y = LinalgIO<Precision>::readVector(yArg.getValue());
  Precision plevel = pArg.getValue();
  Precision eps = epsArg.getValue();
  int knn = knnArg.getValue();
  

  NNMSComplex<Precision> msc(X, y, knn, false, eps);
  msc.mergePersistence(plevel);

  DenseVector<int> crystals = msc.getPartitions();
  DenseMatrix<int> extrema = msc.getCrystals();
  DenseVector<double> p = msc.getPersistence();
 
  std::cout << p.N() << std::endl;
  std::cout << msc.getNAllExtrema() << std::endl;

  for(int i=0; i< p.N(); i++){
    std::cout << p(i) << ", ";
  }
  std::cout << std::endl;


  LinalgIO<int>::writeVector("crystals.data", crystals);
  LinalgIO<int>::writeMatrix("extrema.data",    extrema);
   
  crystals.deallocate();
  extrema.deallocate();
  p.deallocate();
  msc.cleanup();
  X.deallocate();
  y.deallocate();
  return 0;
}
