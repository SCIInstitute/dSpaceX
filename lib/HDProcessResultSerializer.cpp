#include "HDProcessResultSerializer.h"
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include <string>

using namespace FortranLinalg;

HDProcessResult* HDProcessResultSerializer::read(std::string path) {

} 

void HDProcessResultSerializer::write(HDProcessResult *result, std::string path) {
  if (!path.empty() && *path.rbegin() != '/') {
     path += '/';
  } 

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