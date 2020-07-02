#pragma once

#include "dimred/Isomap.h"
#include "dimred/PCA.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "graph/KNNNeighborhood.h"
#include "HDProcessResult.h"
#include "kernelstats/FirstOrderKernelRegression.h"
#include "morsesmale/NNMSComplex.h"
#include "dspacex/Precision.h"
#include "utils/Random.h"

#include <list>
#include <iostream>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

/**
 * Processes high dimensional data and generate low dimensional embeddings.
 */
class HDProcessor {
 public:
  std::unique_ptr<HDProcessResult>  process(FortranLinalg::DenseMatrix<Precision> x,
      FortranLinalg::DenseVector<Precision> y,  
      int knn, int nSamples, int persistenceArg, bool randArg, 
      Precision sigmaArg, Precision sigmaSmooth);
  std::unique_ptr<HDProcessResult>  processOnMetric(FortranLinalg::DenseMatrix<Precision> distances,
    FortranLinalg::DenseVector<Precision> qoi,
    int knn, int nSamples, int persistence, bool random,
    Precision sigmaArg, Precision sigmaSmooth);
 

 private:  
  void computeAnalysisForLevel(NNMSComplex<Precision> &msComplex, 
    unsigned int persistenceLevel, int nSamples, Precision sigma, bool computeRegression = true);
  void computeRegressionForCrystal(unsigned int crystalIndex, unsigned int persistenceLevel, 
    Precision sigma, int nSamples,
    std::vector<std::vector<unsigned int>> &Xi,
    std::vector<std::vector<Precision>> &yci, 
    std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDS,
    FortranLinalg::DenseMatrix<Precision> &S,
    FortranLinalg::DenseVector<Precision> &eWidths);
  void computePCALayout(FortranLinalg::DenseMatrix<Precision> &S, 
    int nExt, int nSamples, unsigned int persistenceLevel);
  void computePCAExtremaLayout(FortranLinalg::DenseMatrix<Precision> &S, 
    std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
    int nExt, int nSamples, unsigned int persistenceLevel);
  void computeIsomapLayout(FortranLinalg::DenseMatrix<Precision> &S, 
    std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
    int nExt, int nSamples, unsigned int persistenceLevel);
  void fit(FortranLinalg::DenseMatrix<Precision> &E, FortranLinalg::DenseMatrix<Precision> &Efit);
  void addNoise(FortranLinalg::DenseVector<Precision> &v);

  FortranLinalg::DenseVector<int> crystalIDs;
  FortranLinalg::DenseMatrix<int> crystals;
  FortranLinalg::DenseVector<Precision> persistence;
  FortranLinalg::DenseMatrix<Precision> Xall;
  FortranLinalg::DenseVector<Precision> yall;


  // Position of extrema for different projection methods
  // PCA of all points inluding regresion curves
  FortranLinalg::DenseMatrix<Precision> extremaPosPCA;

  // PCA of extrema only
  FortranLinalg::DenseMatrix<Precision> extremaPosPCA2;

  // Isomap of regression cruve graphs
  FortranLinalg::DenseMatrix<Precision> extremaPosIso;

  // Output filepath
  std::string m_path;

  // Member variable storing process result as its built;
  std::unique_ptr<HDProcessResult> m_result;

  // List of extrema and extrema of previous persistence level used to align low
  // dimensional mappings to previous level
  typedef std::map<int, int> map_i_i;
  typedef map_i_i::iterator map_i_i_it; 
  map_i_i exts;
  map_i_i extsOrig;
};
