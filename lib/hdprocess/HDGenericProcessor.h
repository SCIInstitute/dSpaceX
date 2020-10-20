#pragma once

#include "dimred/Isomap.h"
#include "dimred/PCA.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/DenseVector.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "graph/KNNNeighborhood.h"
#include "HDProcessor.h"
#include "HDProcessResult.h"
#include "kernelstats/FirstOrderKernelRegression.h"
#include "morsesmale/NNMSComplex.h"
#include "dataset/Precision.h"
#include "utils/Random.h"

#include <map>
#include <string>
#include <vector>

/**
 * Processes high dimensional data and generate low dimensional embeddings.
 */
template <typename SampleType, typename MetricType> 
class HDGenericProcessor : public HDProcessor {
 public:
  FortranLinalg::DenseMatrix<Precision> computeDistances(std::vector<SampleType*> &samples, MetricType &metric) {
    FortranLinalg::DenseMatrix<Precision> distances(samples.size(), samples.size());
    for (unsigned int i = 0; i < samples.size(); i++) {
      for (unsigned int j = i; j < samples.size(); j++) {
        distances(i,j) = distances(j,i) = metric.distance(*(samples[i]), *(samples[j]));        
      }      
    }
    return distances;
  }
};
