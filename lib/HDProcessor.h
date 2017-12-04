#pragma once

#include "HDProcessResult.h"
#include "Precision.h"
#include "PCA.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "KNNNeighborhood.h"
#include "Isomap.h"
#include "FirstOrderKernelRegression.h"
#include "NNMSComplex.h"
#include "Random.h"

#include <map>
#include <string>
#include <vector>

/**
 * Processes high dimensional data and generate low dimensional embeddings.
 */
class HDProcessor {
 public:
  HDProcessor();
  HDProcessResult* process(FortranLinalg::DenseMatrix<Precision> x,
      FortranLinalg::DenseVector<Precision> y,  
      int knn, int nSamples, int persistenceArg, bool randArg, 
      Precision sigmaArg, Precision sigmaSmooth);

 private:  
  void computeInverseRegressionForLevel(NNMSComplex<Precision> &msComplex, 
    unsigned int persistenceLevel, int nSamples, Precision sigma);
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
  HDProcessResult *m_result = nullptr;

  // List of extrema and extrema of previous persistence level used to align low
  // dimensional mappings to previous level
  typedef std::map<int, int> map_i_i;
  typedef map_i_i::iterator map_i_i_it; 
  map_i_i exts;
  map_i_i extsOrig;
};
