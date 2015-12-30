#include "Precision.h"

#include "LinalgIO.h"
#include "CEMRegression.h"
#include "DenseMatrix.h"

#include "CmdLine.h"

//#include "EnableFloatingPointExceptions.h"

int main(int argc, char **argv){

  //Command line parsing
  TCLAP::CmdLine cmd("Kernel Map Manifolds optimization", ' ', "1");

  TCLAP::ValueArg<std::string> yArg("y","Y","High dimensional data", true, "",
      "matrix header file");
  cmd.add(yArg);

  TCLAP::ValueArg<std::string> zArg("z","Z","Inital low dimensional representation", 
      true, "", "matrix header file");
  cmd.add(zArg);  
  
  
  TCLAP::ValueArg<std::string> lArg("l","Labels","Labels, predictors to regress on", 
      true, "", "matrix header file");
  cmd.add(lArg);

  TCLAP::ValueArg<int> iterArg("i","iterations","Maximum number of iterations, default 200", 
      false, 200, "int");
  cmd.add(iterArg);
  
  TCLAP::ValueArg<float> scalingArg("s","step",
      "Gradietn descent step size", false, 0.8,"float");
  cmd.add(scalingArg);
   
  TCLAP::ValueArg<int> knnSigmaArg("","knnSigma",
      "Number of nearest neighbors to compute kernel sigma from", 
      false, 10,  "int");
  cmd.add(knnSigmaArg);  
  
  
  TCLAP::ValueArg<float> alphaArg("f","fudge",
      "Multipication factor of low dimensional kernel sigma", false, 1, 
      "float");
  cmd.add(alphaArg);  
  
  TCLAP::ValueArg<float> lambdaArg("","lambda",
      "Weight between rgression residual and projection distance in objective", false, 1, 
      "float");
  cmd.add(lambdaArg);
  
  TCLAP::ValueArg<std::string> outArg("o","out",
      "output prefix for saving optimized Z (kernel regression parameters), projected data, reconstruction data and KMM file", 
      true, "", "filename");
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
  DenseMatrix<Precision> Z = LinalgIO<Precision>::readMatrix(zArg.getValue());
  DenseMatrix<Precision> L = LinalgIO<Precision>::readMatrix(lArg.getValue());

  


  Precision alpha = alphaArg.getValue();
  Precision lambda = lambdaArg.getValue();

  int knnSigma = knnSigmaArg.getValue();
  int nIter = iterArg.getValue();
  Precision scaling = scalingArg.getValue();

  //leave-one-out testing
  for(unsigned int i=0; i<Y.N(); i++){

    DenseMatrix<Precision> Yt(Y.M(), Y.N()-1);
    DenseMatrix<Precision> Zt(Z.M(), Z.N()-1);
    DenseMatrix<Precision> Lt(L.M(), L.N()-1);
    int index = 0;
    for(int j=0; j<Y.N(); j++){
      if(i==j) continue;
      Linalg<Precision>::SetColumn(Yt, index, Y, j);
      Linalg<Precision>::SetColumn(Zt, index, Z, j);
      Linalg<Precision>::SetColumn(Lt, index, L, j);
      index++;
    }

    CEMRegression<Precision> cem(Yt, Zt, Lt, alpha, knnSigma, lambda);
  
    cem.gradDescent(nIter, scaling);
  
    std::string outprefix = outArg.getValue();
    std::stringstream ss1;
    ss1 << outprefix << i << "_Xp.data";
    std::stringstream ss2;
    ss2 << outprefix << i << "_Z.data";
    std::stringstream ss3;
    ss3 << outprefix << i << "_Yp.data";

    DenseMatrix<Precision> Xp = cem.parametrize(Y);
    LinalgIO<Precision>::writeMatrix(ss1.str(), Xp);
 
    DenseMatrix<Precision> Zend = cem.getZ();
    LinalgIO<Precision>::writeMatrix(ss2.str(), Zend);

    DenseMatrix<Precision> Yp = cem.reconstruct(Xp);
    LinalgIO<Precision>::writeMatrix(ss3.str(), Yp);
    
    Precision sigmaX = cem.getSigmaX();
  
    std::stringstream ss4;
    ss4 << outprefix << i << ".ceminfo";

    std::ofstream info;
    info.open(ss4.str().c_str());
    info << "CEM-regression" << std::endl;
    info << "Ydata: " << yArg.getValue() << std::endl;
    info << "Z_optimized: " << ss2.str() << ".hdr" << std::endl;
    info << "knnSigma: " << knnSigma << std::endl;
    info << "X_sigma: " << sigmaX << std::endl; 
    info.close();

    Xp.deallocate();
    Yp.deallocate();
    cem.cleanup();
  }
 
  return 0;
}
