#include "Precision.h"

#include "LinalgIO.h"
#include "KernelDensity.h"
#include "AdaptiveKernelDensity.h"
#include "GaussianKernel.h"
#include "DenseMatrix.h"

#include "CmdLine.h"

int main(int argc, char **argv){

  //Command line parsing
  TCLAP::CmdLine cmd("Kernel Density Estimation", ' ', "1");

  TCLAP::ValueArg<std::string> xArg("x","X","Training Data", true, "",
      "matrix header file");
  cmd.add(xArg);
  
  TCLAP::ValueArg<std::string> eArg("e","E","Estimation Points Data", true, "",
      "matrix header file");
  cmd.add(eArg);

  TCLAP::SwitchArg aArg("a","A","Adaptive", false);
  cmd.add(aArg);

  TCLAP::ValueArg<std::string> oArg("o","O","Output file name", 
      true, "", "vector data file");
  cmd.add(oArg);

  
  TCLAP::ValueArg<Precision> bArg("b","bw", "Bandwidth ", true,  0, "double");
  cmd.add(bArg);  
  
  
  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }
 

  //data
  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(xArg.getValue());
  DenseMatrix<Precision> E = LinalgIO<Precision>::readMatrix(eArg.getValue());
  Precision bw = bArg.getValue();
  
  std::string outfile = oArg.getValue();

  DenseVector<Precision> p;
  if(aArg.getValue()){
    AdaptiveKernelDensity<Precision> ade(X, bw);
    p = ade.p(E);
  }
  else{
    GaussianKernel<Precision> k(bw, X.M());
    KernelDensity<Precision> kd(X, k);
    p = kd.p(E);
  }


  LinalgIO<Precision>::writeVector(outfile, p);


 
  return 0;
}
