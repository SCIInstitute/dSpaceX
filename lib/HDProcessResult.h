#pragma once

#include "Precision.h"
#include "Linalg.h"


 /**
  * All output data generated from the HDProcessor. 
  */
struct HDProcessResult {
  
  FortranLinalg::DenseVector<Precision> scaledPersistence; // Persistence.data.hdr
  FortranLinalg::DenseVector<Precision> minLevel;          // PersistenceStart.data.hdr
  FortranLinalg::DenseMatrix<Precision> X;                 // Geom.data.hdr
  FortranLinalg::DenseVector<Precision> Y;                 // Function.data.hdr

  // loadData
  //          Crystals_[level].data.hdr 
  //          ExtremaValues_[level].data.hdr
  //          ExtremaWidths_[level].data.hdr
    
  // loadLayouts
  // Lmin     IsoMin.data.hdr
  // Lmax     IsoMax.data.hdr
  // L[i] =   ps_[level]_crystal_[i]_isolayout.data.hdr
  //          IsoExtremaLayout_[level].data.hdr
  // Lmin     PCAMin.data.hdr
  // Lmax     PCAMax.data.hdr
  // L[i] =   ps_[level]_crystal_[i]_layout.data.hdr
  //          ExtremaLayout_[level].data.hdr
  // Lmin     PCA2Min.data.hdr
  // Lmin     PCA2Max.data.hdr
  // L[i] =   ps_[level]_crystal_[i]_pca2layout.data.hdr
  //          PCA2ExtremaLayout_[level].data.hdr
    
        
  // loadColorValues    // ps_[level]_crystal_[i]_fmean.data.hdr
  // loadWidthValues    // ps_[level]_crystal_[i]_mdists.data.hdr
  // loadDensityValues  // ps_[level]_crystal_[i]_spdf.data.hdr

  // Reconstructions 
  // R =                // ps_[level]_crystal_[i]_Rs.data.hdr";
  // gradR =            // ps_[level]_crystal_[i]_gradRs.data.hdr";
  // Rvar =             // ps_[level]_crystal_[i]_Svar.data.hdr"; 
};