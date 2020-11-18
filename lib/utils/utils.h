#pragma once

#include "flinalg/Linalg.h"
#include "DenseVectorSample.h"
#include "flinalg/DenseMatrix.h"
#include "hdprocess/HDGenericProcessor.h"

#include <Eigen/Dense>
#include <limits>

namespace HDProcess {

template<typename T>
FortranLinalg::DenseMatrix<T> computeDistanceMatrix(FortranLinalg::DenseMatrix<T> &x) {
  std::vector<DenseVectorSample*> samples;
  for (int j = 0; j < x.N(); j++) {
    FortranLinalg::DenseVector<T> vector(x.M());
    for (int i = 0; i < x.M(); i++) {
      vector(i) = x(i, j);
    }
    DenseVectorSample *sample = new DenseVectorSample(vector);
    samples.push_back(sample);
  }

  HDGenericProcessor<DenseVectorSample, DenseVectorEuclideanMetric> genericProcessor;
  DenseVectorEuclideanMetric metric;
  return genericProcessor.computeDistances(samples, metric);
}

} // HDProcess


namespace dspacex {

/*
 * Converts mat to a FortranLinalg::DenseMatrix, taking data from mat (thus non-const and empty of data at the end).
 */
template<typename T>
static FortranLinalg::DenseMatrix<T> toDenseMatrix(Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& mat)
{
  FortranLinalg::DenseMatrix<T> ret(mat.rows(), mat.cols(), mat.data());

  // use placement new to ensure data doesn't get reclaimed when mat goes out of scope by caller
  new (&mat) Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>(nullptr, -1, -1);

  return std::move(ret);
}

/*
 * Return best sigma for this set of field values.
 * - compute pairwise distances between all given field values (i.e. fieldvalues) -- this will result in a symmetric matrix with zeros on the diagonal. all entries don't have to be computed as abs(x1 - x2) = abs(x2 - x1)
 * - set diagonal entries to a very large value
 * - compute min distance (entry) for each row
 * - compute average distance across all rows
 * - sigma = a factor times this average distance
 */
Precision computeSigma(const std::vector<Precision>& vals);

} // dspacex
