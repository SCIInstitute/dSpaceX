#include "Precision.h"

#include "LinalgIO.h"
#include "BlockCEM.h"
#include "DenseMatrix.h"

#include <tclap/CmdLine.h>

//#include "EnableFloatingPointExceptions.h"

int main(int argc, char **argv){
      using namespace FortranLinalg;

  //Command line parsing
  TCLAP::CmdLine cmd("Fast cnditional expectation manifold optimization", ' ', "1");

  TCLAP::ValueArg<std::string> yArg("y","Y","High dimensional data", true, "",
      "matrix header file");
  cmd.add(yArg);

  TCLAP::ValueArg<std::string> xArg("x","X","Z parameters for cooordinate mapping", 
      true, "", "matrix header file");
  cmd.add(xArg);

  TCLAP::ValueArg<int> nArg("n","nSamples","Number of samples to estimate objective within iterations", 
      false, 200, "int");
  cmd.add(nArg);

  TCLAP::ValueArg<int> iterArg("i","iterations","Maximum number of iterations, default 200",
      false, 200, "int");
  cmd.add(iterArg);
  
  TCLAP::ValueArg<float> scalingArg("s","step", "Gradient descent step size",
      false, 0.8,"float");
  cmd.add(scalingArg);

  TCLAP::ValueArg<int> knnXArg("","knnX", "Number of nearest neighbors to use for g", false, 10,  "int");
  cmd.add(knnXArg);  
   
  TCLAP::ValueArg<int> rArg("r","risk", "Optimization objective", 
      false, 2,  "int");
  cmd.add(rArg);    
  
  TCLAP::ValueArg<int> pArg("p","penalty", "Optimization objective penalty",
      false, 0,  "int");
  cmd.add(pArg);    
    
    
  TCLAP::ValueArg<float> sigmaXArg("","sigmaX",
      "Multipication factor of low dimensional kernel sigma", false, 1.0/3.0, 
      "float");
  cmd.add(sigmaXArg);

  
  TCLAP::ValueArg<std::string> outArg("o","out", "output prefix for saving CEM data", true, "", "filename");
  cmd.add(outArg);

  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }
 

  //data
  DenseMatrix<Precision> Y = LinalgIO<Precision>::readMatrix(yArg.getValue());
  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(xArg.getValue());

  
  std::string outprefix = outArg.getValue();
  std::stringstream ss1;
  ss1 << outprefix << "_X.data";
  std::stringstream ss2;
  ss2 << outprefix << "_Yp.data";





  Precision sigmaX = sigmaXArg.getValue();
  int knnX = knnXArg.getValue();

  BlockCEM<Precision>::Penalty penalty = BlockCEM<Precision>::toPenalty(pArg.getValue());
  BlockCEM<Precision>::Risk risk = BlockCEM<Precision>::toRisk(rArg.getValue());

  BlockCEM<Precision> cem(Y, X, knnX, sigmaX, true);

  cem.gradDescent( iterArg.getValue(), nArg.getValue(), scalingArg.getValue(),
      scalingArg.getValue()/2.0, 2, risk, penalty, true);

  DenseMatrix<Precision> Xp = cem.getX();
  LinalgIO<Precision>::writeMatrix(ss1.str(), Xp);

  DenseMatrix<Precision> Yp = cem.reconstruct();
  LinalgIO<Precision>::writeMatrix(ss2.str(), Yp);

  DenseMatrix<double> curvature(X.M(), Xp.N());
  DenseVector<double> gauss(Xp.N());
  DenseVector<double> mean(Xp.N());
  DenseVector<double> detg(Xp.N());
  DenseVector<double> detB(Xp.N());
  DenseVector<double> frob(Xp.N());

  DenseVector<double> xp(X.M());
  for(unsigned int i=0; i<Xp.N(); i++){
    Linalg<double>::ExtractColumn(Xp, i, xp);
    DenseVector<double> c = cem.curvature(xp, mean(i), gauss(i), detg(i), detB(i), frob(i));
    Linalg<double>::SetColumn(curvature, i, c);
    c.deallocate();
  }
  xp.deallocate();




  sigmaX = cem.getSigmaX();

  std::stringstream ss4;
  ss4 << outprefix << ".bcem";

  std::ofstream info;
  info.open(ss4.str().c_str());
  info << "BlockCEM" << std::endl;
  info << "Ydata: " << yArg.getValue() << std::endl;
  info << "X_optimized: " << ss2.str() << ".hdr" << std::endl;
  info << "knnX: " << knnX << std::endl;
  info << "sigmaX: " << sigmaX << std::endl; 
  info.close();

  Yp.deallocate();

  cem.cleanup();

  return 0;
}
