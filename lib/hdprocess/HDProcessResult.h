#pragma once

#include "flinalg/Linalg.h"
#include "dspacex/Precision.h"
#include <vector>


 /**
  * All output data generated from the HDProcessor. 
  */
struct HDProcessResult {
  FortranLinalg::DenseMatrix<int> knn;
  FortranLinalg::DenseVector<Precision> scaledPersistence; // Persistence.data.hdr
  FortranLinalg::DenseVector<Precision> minLevel;          // PersistenceStart.data.hdr
  FortranLinalg::DenseMatrix<Precision> X;                 // Geom.data.hdr
  FortranLinalg::DenseVector<Precision> Y;                 // Function.data.hdr
  FortranLinalg::DenseVector<int> regressionSampleCount;   

  // loadData
  std::vector<FortranLinalg::DenseMatrix<int>> crystals;            // Crystals_[level].data.hdr
  std::vector<FortranLinalg::DenseVector<int>> crystalPartitions;   // CrystalPartitions_[level].data.hdr
  std::vector<FortranLinalg::DenseVector<Precision>> extremaValues; // ExtremaValues_[level].data.hdr
  std::vector<FortranLinalg::DenseVector<Precision>> extremaWidths; // ExtremaWidths_[level].data.hdr
    
  // Layout Data 
  FortranLinalg::DenseVector<Precision> LminPCA;       // PCAMin.data.hdr
  FortranLinalg::DenseVector<Precision> LmaxPCA;       // PCAMax.data.hdr  
  std::vector<FortranLinalg::DenseMatrix<Precision>> PCAExtremaLayout;        // PCA_ExtremaLayout_[level].data.hdr
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> PCALayout;  // ps_[level]_crystal_[i]_layout.data.hdr
  
  FortranLinalg::DenseVector<Precision> LminPCA2;       // PCA2Min.data.hdr
  FortranLinalg::DenseVector<Precision> LmaxPCA2;       // PCA2Max.data.hdr
  std::vector<FortranLinalg::DenseMatrix<Precision>> PCA2ExtremaLayout;        // PCA2ExtremaLayout_[level].data.hdr
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> PCA2Layout;  // ps_[level]_crystal_[i]_pca2layout.data.hdr
  

  FortranLinalg::DenseVector<Precision> LminIso;        // IsoMin.data.hdr
  FortranLinalg::DenseVector<Precision> LmaxIso;        // IsoMax.data.hdr
  std::vector<FortranLinalg::DenseMatrix<Precision>> IsoExtremaLayout;         // IsoExtremaLayout_[level].data.hdr
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> IsoLayout;   // ps_[level]_crystal_[i]_isolayout.data.hdr
        
  // loadColorValues    
  std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> fmean;  // ps_[level]_crystal_[i]_fmean.data.hdr
  // loadWidthValues    
  std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> mdists; // ps_[level]_crystal_[i]_mdists.data.hdr
  // loadDensityValues  
  std::vector<std::vector<FortranLinalg::DenseVector<Precision>>> spdf;   // ps_[level]_crystal_[i]_spdf.data.hdr

  // Reconstructions - The is the regression curve
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> R;     // ps_[level]_crystal_[i]_Rs.data.hdr"; CrystalIds
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> gradR; // ps_[level]_crystal_[i]_gradRs.data.hdr";
  std::vector<std::vector<FortranLinalg::DenseMatrix<Precision>>> Rvar;  // ps_[level]_crystal_[i]_Svar.data.hdr"; 

  // parameter names
  FortranLinalg::DenseVector<std::string> names;
};
