#pragma once

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
  void process(const std::string &domainFilename, 
      const std::string &functionFilename, 
      float sigmaArg, int samplesArg, int persistenceArg, 
      int knnArg, bool randArg, double smoothArg);

 private:
  void computePCALayout(FortranLinalg::DenseMatrix<Precision> &S, 
    int nExt, int nSamples, unsigned int nP);
  void computePCAExtremaLayout(FortranLinalg::DenseMatrix<Precision> &S, 
    std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
    int nExt, int nSamples, unsigned int nP);
  void computeIsomapLayout(FortranLinalg::DenseMatrix<Precision> &S, 
    std::vector<FortranLinalg::DenseMatrix<Precision>> &ScrystalIDs, 
    EuclideanMetric<Precision> &l2, int nExt, int nSamples, unsigned int nP);
  void fit(FortranLinalg::DenseMatrix<Precision> &E, FortranLinalg::DenseMatrix<Precision> &Efit);

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

  // List of extrema and extrema of previous persistence level used to align low
  // dimensional mappings to previous level
  typedef std::map<int, int> map_i_i;
  typedef map_i_i::iterator map_i_i_it; 
  map_i_i exts;
  map_i_i extsOrig;
};
