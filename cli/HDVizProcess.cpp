#include "HDProcessor.h"
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
    writeData(result, outArg.getValue());    
    std::cout << "Done." << std::endl;
  } else {
    std::cout << "Something went wrong. Process returned null result." << std::endl;
  }
  
  return 0;
}

/**
 * Save contents of HDProcessResult to files used by HDViz
 */
void writeData(HDProcessResult *result, std::string path) {
  // Fix output path if necessary.
  if (!path.empty() && *path.rbegin() != '/') {
     path += '/';
  }
  std::cout << "Saving all output files to: " << path << std::endl;

  // Save result data to disk.
  LinalgIO<Precision>::writeMatrix(path + "Geom.data", result->X);   
  LinalgIO<Precision>::writeVector(path + "Function.data", result->Y);   
  LinalgIO<Precision>::writeVector(path + "Persistence.data", result->scaledPersistence);
  LinalgIO<Precision>::writeVector(path + "PersistenceStart.data", result->minLevel);
  for (int level = result->minLevel(0); level < result->scaledPersistence.N(); level++) {
    std::string crystalsFilename = "Crystals_" + std::to_string(level) + ".data";
    LinalgIO<int>::writeMatrix(path + crystalsFilename, result->crystals[level]);

    std::string ExtremaValuesFilename = "ExtremaValues_" + std::to_string(level) + ".data";
    LinalgIO<Precision>::writeVector(path + ExtremaValuesFilename, result->extremaValues[level]);      

    std::string extremaWidthsFilename = "ExtremaWidths_" + std::to_string(level) + ".data";
    LinalgIO<Precision>::writeVector(path + extremaWidthsFilename, result->extremaWidths[level]);

    for (int crystalIndex = 0; crystalIndex < result->crystals[level].N(); crystalIndex++) {
      std::string crystalFilePrefix =
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex);
      std::string crystalIdFilename = crystalFilePrefix + "_Rs.data";
      LinalgIO<Precision>::writeMatrix(path + crystalIdFilename, result->R[level][crystalIndex]);

      std::string gradFilename = crystalFilePrefix + "_gradRs.data";
      LinalgIO<Precision>::writeMatrix(path + gradFilename, result->gradR[level][crystalIndex]);

      std::string rvarFilename = crystalFilePrefix + "_Svar.data";
      LinalgIO<Precision>::writeMatrix(path + rvarFilename, result->Rvar[level][crystalIndex]);

      std::string mdistsFilename = crystalFilePrefix + "_mdists.data";
      LinalgIO<Precision>::writeVector(path + mdistsFilename, result->mdists[level][crystalIndex]);

      std::string fmeanFilename = crystalFilePrefix + "_fmean.data";
      LinalgIO<Precision>::writeVector(path + fmeanFilename, result->fmean[level][crystalIndex]);

      std::string spdfFilename = crystalFilePrefix + "_spdf.data";
      LinalgIO<Precision>::writeVector(path + spdfFilename, result->spdf[level][crystalIndex]);
    }
  }

  // Output Layout Data
  LinalgIO<Precision>::writeVector(path + "PCAMin.data", result->LminPCA);
  LinalgIO<Precision>::writeVector(path + "PCAMax.data", result->LmaxPCA);
  LinalgIO<Precision>::writeVector(path + "PCA2Min.data", result->LminPCA2);
  LinalgIO<Precision>::writeVector(path + "PCA2Max.data", result->LmaxPCA2);
  LinalgIO<Precision>::writeVector(path + "IsoMin.data", result->LminIso);
  LinalgIO<Precision>::writeVector(path + "IsoMax.data", result->LmaxIso);

  for (int level = result->minLevel(0); level < result->scaledPersistence.N(); level++) {
    std::string PCAExtremaLayoutFilename = "ExtremaLayout_" + std::to_string(level) + ".data";
    LinalgIO<Precision>::writeMatrix(path + PCAExtremaLayoutFilename, result->PCAExtremaLayout[level]);

    std::string PCA2ExtremaLayoutFilename = "PCA2ExtremaLayout_" + std::to_string(level) + ".data";
    LinalgIO<Precision>::writeMatrix(path + PCA2ExtremaLayoutFilename, result->PCA2ExtremaLayout[level]);

    std::string IsoExtremaFilename = "IsoExtremaLayout_" + std::to_string(level) + ".data";
    LinalgIO<Precision>::writeMatrix(path + IsoExtremaFilename, result->IsoExtremaLayout[level]);

    for (int crystalIndex = 0; crystalIndex < result->crystals[level].N(); crystalIndex++) {
      std::string PCALayoutFilename = 
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_layout.data";
      LinalgIO<Precision>::writeMatrix(path + PCALayoutFilename, result->PCALayout[level][crystalIndex]);

      std::string PCA2LayoutFilename = 
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_pca2layout.data";
      LinalgIO<Precision>::writeMatrix(path + PCA2LayoutFilename, result->PCA2Layout[level][crystalIndex]);            

      std::string IsoLayoutFilename = 
        "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_isolayout.data";
      LinalgIO<Precision>::writeMatrix(path + IsoLayoutFilename, result->IsoLayout[level][crystalIndex]);            
    }
  }
}
