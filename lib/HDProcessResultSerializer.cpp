#include "HDProcessResultSerializer.h"
#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include <string>

using namespace FortranLinalg;

const std::string k_defaultPersistenceDataHeaderFilename = "Persistence.data.hdr";
const std::string k_defaultPersistenceStartHeaderFilename = "PersistenceStart.data.hdr";
const std::string k_defaultGeomDataHeaderFilename = "Geom.data.hdr";
const std::string k_defaultFunctionDataHeaderFilename = "Function.data.hdr";
const std::string k_defaultParameterNamesFilename = "names.txt";

HDProcessResult* HDProcessResultSerializer::read(std::string path) {
  if (!path.empty() && *path.rbegin() != '/') {
     path += '/';
  } 

  HDProcessResult *result = new HDProcessResult();
  
  result->X = LinalgIO<Precision>::readMatrix(path + "Geom.data.hdr");   
  result->Y = LinalgIO<Precision>::readVector(path + "Function.data.hdr");   
  result->scaledPersistence = 
      LinalgIO<Precision>::readVector(path + "Persistence.data.hdr");
  result->minLevel = LinalgIO<Precision>::readVector(path + "PersistenceStart.data.hdr");

  // Resize Stores for Persistence Level information
  result->crystals.resize(result->scaledPersistence.N());
  result->extremaValues.resize(result->scaledPersistence.N());  
  result->extremaWidths.resize(result->scaledPersistence.N());
  result->R.resize(result->scaledPersistence.N());
  result->gradR.resize(result->scaledPersistence.N());
  result->Rvar.resize(result->scaledPersistence.N());
  result->mdists.resize(result->scaledPersistence.N());
  result->fmean.resize(result->scaledPersistence.N());
  result->spdf.resize(result->scaledPersistence.N());
  result->PCAExtremaLayout.resize(result->scaledPersistence.N());  
  result->PCALayout.resize(result->scaledPersistence.N());
  result->PCA2ExtremaLayout.resize(result->scaledPersistence.N());
  result->PCA2Layout.resize(result->scaledPersistence.N());
  result->IsoExtremaLayout.resize(result->scaledPersistence.N());
  result->IsoLayout.resize(result->scaledPersistence.N());

  for (int level = result->minLevel(0); level < result->scaledPersistence.N(); level++) {
    std::string crystalsFilename = "Crystals_" + std::to_string(level) + ".data.hdr";
    result->crystals[level] = LinalgIO<int>::readMatrix(path + crystalsFilename);

    std::string ExtremaValuesFilename = "ExtremaValues_" + std::to_string(level) + ".data.hdr";
    result->extremaValues[level] = LinalgIO<Precision>::readVector(path + ExtremaValuesFilename);

    std::string extremaWidthsFilename = "ExtremaWidths_" + std::to_string(level) + ".data.hdr";
    result->extremaWidths[level] = LinalgIO<Precision>::readVector(path + extremaWidthsFilename);

    // Resize Stores for Regression Information
    result->R[level].resize(result->crystals[level].N());
    result->gradR[level].resize(result->crystals[level].N());
    result->Rvar[level].resize(result->crystals[level].N());
    result->mdists[level].resize(result->crystals[level].N());  
    result->fmean[level].resize(result->crystals[level].N());  
    result->spdf[level].resize(result->crystals[level].N());  

    for (int crystalIndex = 0; crystalIndex < result->crystals[level].N(); crystalIndex++) {
      std::string crystalFilePrefix =
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex);
      std::string crystalIdFilename = crystalFilePrefix + "_Rs.data.hdr";
      result->R[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + crystalIdFilename);

      std::string gradFilename = crystalFilePrefix + "_gradRs.data.hdr";
      result->gradR[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + gradFilename);

      std::string rvarFilename = crystalFilePrefix + "_Svar.data.hdr";
      result->Rvar[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + rvarFilename);

      std::string mdistsFilename = crystalFilePrefix + "_mdists.data.hdr";
      result->mdists[level][crystalIndex] = LinalgIO<Precision>::readVector(path + mdistsFilename);

      std::string fmeanFilename = crystalFilePrefix + "_fmean.data.hdr";
      result->fmean[level][crystalIndex] = LinalgIO<Precision>::readVector(path + fmeanFilename);

      std::string spdfFilename = crystalFilePrefix + "_spdf.data.hdr";
      result->spdf[level][crystalIndex] = LinalgIO<Precision>::readVector(path + spdfFilename);
    }
  }

  // Layout Data
  result->LminPCA = LinalgIO<Precision>::readVector(path + "PCAMin.data.hdr");
  result->LmaxPCA = LinalgIO<Precision>::readVector(path + "PCAMax.data.hdr");
  result->LminPCA2 = LinalgIO<Precision>::readVector(path + "PCA2Min.data.hdr");
  result->LmaxPCA2 = LinalgIO<Precision>::readVector(path + "PCA2Max.data.hdr");
  result->LminIso = LinalgIO<Precision>::readVector(path + "IsoMin.data.hdr");
  result->LmaxIso = LinalgIO<Precision>::readVector(path + "IsoMax.data.hdr");

  for (int level = result->minLevel(0); level < result->scaledPersistence.N(); level++) {    
    std::string PCAExtremaLayoutFilename = "ExtremaLayout_" + std::to_string(level) + ".data.hdr";
    result->PCAExtremaLayout[level] = LinalgIO<Precision>::readMatrix(path + PCAExtremaLayoutFilename);

    std::string PCA2ExtremaLayoutFilename = "PCA2ExtremaLayout_" + std::to_string(level) + ".data.hdr";
    result->PCA2ExtremaLayout[level] = LinalgIO<Precision>::readMatrix(path + PCA2ExtremaLayoutFilename);

    std::string IsoExtremaFilename = "IsoExtremaLayout_" + std::to_string(level) + ".data.hdr";
    result->IsoExtremaLayout[level] = LinalgIO<Precision>::readMatrix(path + IsoExtremaFilename);


    // Resize Layouts
    result->PCALayout[level].resize(result->crystals[level].N());
    result->PCA2Layout[level].resize(result->crystals[level].N());
    result->IsoLayout[level].resize(result->crystals[level].N());

    for (int crystalIndex = 0; crystalIndex < result->crystals[level].N(); crystalIndex++) {
      std::string PCALayoutFilename = 
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_layout.data.hdr";
      result->PCALayout[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + PCALayoutFilename);

      std::string PCA2LayoutFilename = 
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_pca2layout.data.hdr";
      result->PCA2Layout[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + PCA2LayoutFilename);

      std::string IsoLayoutFilename = 
          "ps_" + std::to_string(level) + "_crystal_" + std::to_string(crystalIndex) + "_isolayout.data.hdr";
      result->IsoLayout[level][crystalIndex] = LinalgIO<Precision>::readMatrix(path + IsoLayoutFilename);
    }
  }

  std::cout << "I'm here now" << std::endl;

  return result;
} 

void HDProcessResultSerializer::write(HDProcessResult *result, std::string path) {
  if (!path.empty() && *path.rbegin() != '/') {
     path += '/';
  } 

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