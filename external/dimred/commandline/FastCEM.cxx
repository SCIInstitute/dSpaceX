#include "Precision.h"

#include "LinalgIO.h"
#include "FastCEM.h"
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

  TCLAP::ValueArg<std::string> lzArg("","lz","Z parameters for cooordinate mapping", 
      true, "", "matrix header file");
  cmd.add(lzArg);

  TCLAP::ValueArg<std::string> lyArg("","ly","Y locations for cooordinate mapping", 
      true, "", "matrix header file");
  cmd.add(lyArg);

  TCLAP::ValueArg<int> nArg("n","nSamples","Number of samples to estimate objective within iterations", 
      false, 200, "int");
  cmd.add(nArg);

  TCLAP::ValueArg<int> iterArg("i","iterations","Maximum number of iterations, default 200", 
      false, 200, "int");
  cmd.add(iterArg);
  
  TCLAP::ValueArg<float> scalingArg("s","step",
      "Gradient descent step size", false, 0.8,"float");
  cmd.add(scalingArg);
    
  TCLAP::ValueArg<int> knnYArg("","knnY",
      "Number of nearest neighbors to use for lambda", 
      false, 10,  "int");
  cmd.add(knnYArg); 

  TCLAP::ValueArg<int> knnXArg("","knnX",
      "Number of nearest neighbors to use for g", 
      false, 10,  "int");
  cmd.add(knnXArg);  
   
  TCLAP::ValueArg<int> rArg("r","risk",
      "Optimization objective", 
      false, 2,  "int");
  cmd.add(rArg);    
  
  TCLAP::ValueArg<int> pArg("p","penalty",
      "Optimization objective penalty", 
      false, 0,  "int");
  cmd.add(pArg);    
    
    
  TCLAP::ValueArg<float> sigmaXArg("","sigmaX",
      "Multipication factor of low dimensional kernel sigma", false, 1.0/3.0, 
      "float");
  cmd.add(sigmaXArg);

  TCLAP::ValueArg<float> sigmaYArg("","sigmaY",
      "Multipication factor of high dimensional kernel sigma", false, 1.0/3.0, 
      "float");
  cmd.add(sigmaYArg);

  TCLAP::ValueArg<float> sigmaZArg("","sigmaZ",
      "Multipication factor of high dimensional kernel sigma", false, 1.0/3.0, 
      "float");
  cmd.add(sigmaZArg);

  
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
  DenseMatrix<Precision> lZ = LinalgIO<Precision>::readMatrix(lzArg.getValue());
  DenseMatrix<Precision> lY = LinalgIO<Precision>::readMatrix(lyArg.getValue());

  
  std::string outprefix = outArg.getValue();
  std::stringstream ss1;
  ss1 << outprefix << "_Xp.data";
  std::stringstream ss2;
  ss2 << outprefix << "_Z.data";
  std::stringstream ss3;
  ss3 << outprefix << "_Yp.data";





  Precision sigmaZ = sigmaZArg.getValue();
  Precision sigmaY = sigmaYArg.getValue();
  Precision sigmaX = sigmaXArg.getValue();
  int knnX = knnXArg.getValue();
  int knnY = knnYArg.getValue();

  Penalty penalty = FastCEM<Precision>::toPenalty(pArg.getValue());
  Risk risk = FastCEM<Precision>::toRisk(rArg.getValue());

    FastCEM<Precision> cem(Y, lY, knnY, lZ, knnX, sigmaZ, sigmaY, sigmaX, true);
  
    cem.gradDescent( iterArg.getValue(), nArg.getValue(), scalingArg.getValue(),
        scalingArg.getValue()/2.0, 2, risk, penalty, false);

    DenseMatrix<Precision> Xp = cem.parametrize();
    LinalgIO<Precision>::writeMatrix(ss1.str(), Xp);
 
    DenseMatrix<Precision> Zend = cem.getZ();
    LinalgIO<Precision>::writeMatrix(ss2.str(), Zend);

    DenseMatrix<Precision> Yp = cem.reconstruct();
    LinalgIO<Precision>::writeMatrix(ss3.str(), Yp);
   
  DenseMatrix<double> curvature(lZ.M(), Xp.N());
  DenseVector<double> gauss(Xp.N());
  DenseVector<double> mean(Xp.N());
  DenseVector<double> detg(Xp.N());
  DenseVector<double> detB(Xp.N());
  DenseVector<double> frob(Xp.N());

  DenseVector<double> xp(lZ.M());
  for(unsigned int i=0; i<Xp.N(); i++){
	  Linalg<double>::ExtractColumn(Xp, i, xp);
	  DenseVector<double> c = cem.curvature(xp, mean(i), gauss(i), detg(i), detB(i), frob(i));
	  Linalg<double>::SetColumn(curvature, i, c);
    c.deallocate();
  }
  xp.deallocate();




    sigmaX = cem.getSigmaX();
    sigmaY = cem.getSigmaY();
  
    std::stringstream ss4;
    ss4 << outprefix << ".kmminfo";

    std::ofstream info;
    info.open(ss4.str().c_str());
    info << "KMM" << std::endl;
    info << "Ydata: " << yArg.getValue() << std::endl;
    info << "Z_optimized: " << ss2.str() << ".hdr" << std::endl;
    info << "knnX: " << knnX << std::endl;
    info << "sigmaX: " << sigmaX << std::endl; 
    info << "sigmaY: " << sigmaY << std::endl; 
    info.close();

    Yp.deallocate();

    cem.cleanup();
 
    return 0;
}
