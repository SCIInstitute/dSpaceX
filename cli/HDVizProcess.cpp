#include "HDProcessor.h"
#include "HDProcessResultSerializer.h"
#include <tclap/CmdLine.h>
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"

using namespace FortranLinalg;

void writeData(HDProcessResult *result, std::string path);

/**
 * HDVisProcess application entry point.
 */
int main(int argc, char **argv){
  TCLAP::CmdLine cmd("Compute MS-Complex and summary representation", ' ', "1");
  TCLAP::ValueArg<Precision> sigmaArg("s" /* flag */, "sigma" /* name */,
      "Kernel regression bandwith (sigma for Gaussian)" /* description */, 
      true /* required */, 1 /* default */, "float" /* type */);
  cmd.add(sigmaArg);  
    
  TCLAP::ValueArg<std::string> xArg("x" /* flag */, "domain" /* name */, 
      "Filename for Data points in domain" /* description */, 
      true /* required */, "", "string");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> fArg("f" /* flag */, "function" /* name */, 
      "f(x), Filename for function value for each data point in X" /* description */, 
      true /* required */, "", "string");
  cmd.add(fArg);

  TCLAP::ValueArg<std::string> outArg("o" /* flag */, "output_dir" /* name */,
      "The directory to output analysis files to." /* description */,
      false /* required */, "./", "string");
  cmd.add(outArg);

  TCLAP::ValueArg<int> pArg("p" /* flag */, "persistence",
      "Number of persistence levels to compute; all = -1 , default = 20" /* description */, 
      true /* required */, 20 /* default */, "integer");
  cmd.add(pArg);  
  
  TCLAP::ValueArg<int> samplesArg("n" /* flag */, "samples" /* name */,
      "Number of samples for each regression curve, default = 50" /* description */, 
      true /* required */, 50 /* default */,  "integer" /* type */);
  cmd.add(samplesArg);  

  TCLAP::ValueArg<int> knnArg("k" /* flag */, "knn" /* name */,
      "Number of nearest neighbors for Morse-Smale approximation, default = 50" /* description */, 
      true /* required */, 50,  "integer" /* type */);
  cmd.add(knnArg); 

  TCLAP::SwitchArg randArg("r" /* flag */, "random" /* name */, 
      "Adds 0.0001 * range(f) uniform random noise to f, " 
      "in case of 0 gradients due to equivivalent values" /* description */, 
      false /* required */); 
  cmd.add(randArg);

  TCLAP::ValueArg<double> smoothArg("" /* flag */, "smooth" /* name */, 
      "Smooth function values to nearest nieghbor averages" /* description */, 
      false /* required */, 0 /* default */, "double" /* type */); 
  cmd.add(smoothArg);

    
  try {
    cmd.parse( argc, argv );    
  } catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }

  // Load Input Data
  // TODO: Move into a data loading library
  DenseMatrix<Precision> x = LinalgIO<Precision>::readMatrix(xArg.getValue());
  DenseVector<Precision> y = LinalgIO<Precision>::readVector(fArg.getValue());
      
  HDProcessResult *result = nullptr;
  try {
    HDProcessor processor;
    result = processor.process(
        x /* domain */,
        y /* function */,
        knnArg.getValue() /* knn */,        
        samplesArg.getValue() /* samples */,
        pArg.getValue() /* persistence */,        
        randArg.getValue() /* random */,
        sigmaArg.getValue() /* sigma */,
        smoothArg.getValue() /* smooth */);
  } catch (const char *err) {
    std::cerr << err << std::endl;
  }

  // Check result and maybe write to disk.
  if (result != nullptr) {
    std::cout << "Successfully returned process result. Saving..." << std::endl;
    HDProcessResultSerializer::write(result, outArg.getValue());    
    std::cout << "Done." << std::endl;
  } else {
    std::cout << "Something went wrong. Process returned null result." << std::endl;
  }
  
  return 0;
}
