#include "HDProcessor.h"
#include <tclap/CmdLine.h>

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
      "Data points in domain" /* description */, 
      true /* required */, "", "");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> fArg("f" /* flag */, "function" /* name */, 
      "f(x), function value for each data point in X" /* description */, 
      true /* required */, "", "");
  cmd.add(fArg);

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

  try {
    HDProcessor processor;
    processor.process(
        xArg.getValue() /* domainFilename */,
        fArg.getValue() /* functionFilename */,
        knnArg.getValue() /* knn */,        
        samplesArg.getValue() /* samples */,
        pArg.getValue() /* persistence */,        
        randArg.getValue() /* random */,
        sigmaArg.getValue() /* sigma */,
        smoothArg.getValue() /* smooth */);
  } catch (const char *err) {
    std::cerr << err << std::endl;
  }
  
  return 0;
}
