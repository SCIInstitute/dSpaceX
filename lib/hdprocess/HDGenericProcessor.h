#pragma once

#include "DenseMatrix.h"
#include "DenseVector.h"
#include "graph/KNNNeighborhood.h"
#include "HDProcessor.h"
#include "HDProcessResult.h"
#include "Isomap.h"
#include "kernelstats/FirstOrderKernelRegression.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "morsesmale/NNMSComplex.h"
#include "Precision.h"
#include "PCA.h"
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
