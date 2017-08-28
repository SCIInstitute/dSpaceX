#include "HDProcessor.h"
#include <tclap/CmdLine.h>

/**
 * HDVisProcess application entry point.
 */
int main(int argc, char **argv){
  
  using namespace FortranLinalg;
  //Command line parsing
  TCLAP::CmdLine cmd("Compute MS-Complex and summary representation", ' ', "1");

  TCLAP::ValueArg<Precision> sigmaArg("s","sigma",
      "Kernel regression bandwith (sigma for Gaussian)", 
      true, 1,  "float");
  cmd.add(sigmaArg);  
    
  TCLAP::ValueArg<std::string> xArg("x","domain","Data points in domain", 
      true,  "", "");
  cmd.add(xArg);

  TCLAP::ValueArg<std::string> fArg("f","function","f(x), function value for each data point in X", 
      true,  "", "");
  cmd.add(fArg);

  TCLAP::ValueArg<int> pArg("p","persistence",
      "Number of persistence levels to compute; all = -1 , default = 20", 
      true, 20,  "integer");
  cmd.add(pArg);  
  
  TCLAP::ValueArg<int> samplesArg("n","samples",
    "Number of samples for each regression curve, default = 50", 
    true, 50,  "integer");
  cmd.add(samplesArg);  

  TCLAP::ValueArg<int> knnArg("k","knn",
      "Number of nearest neighbors for Morse-Smale approximation, default = 50", 
      true, 50,  "integer");
  cmd.add(knnArg); 

  TCLAP::SwitchArg randArg("r", "random", 
    "Adds 0.0001 * range(f) uniform random noise to f, in case of 0 gradients due to equivivalent values", false); 
  cmd.add(randArg);

  TCLAP::ValueArg<double> smoothArg("", "smooth", 
    "Smooth function values to nearest nieghbor averages", false, 0, "double"); 
  cmd.add(smoothArg);
    
  try {
    cmd.parse( argc, argv );    
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }

  try {
    HDProcessor processor;
    processor.process(
        xArg.getValue() /* domainFilename */,
        fArg.getValue() /* functionFilename */,
        sigmaArg.getValue() /* sigma */,
        samplesArg.getValue() /* samples */,
        pArg.getValue() /* persistence */,
        knnArg.getValue() /* knn */,
        randArg.getValue() /* random */,
        smoothArg.getValue() /* smooth */);
  } catch (const char *err) {
    std::cerr << err << std::endl;
  }
  
  return 0;
}
