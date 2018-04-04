#pragma once

#include "DenseVector.h"
#include "metrics/EuclideanMetric.h"
#include "Precision.h"


class DenseVectorSample {
public:
  DenseVectorSample(FortranLinalg::DenseVector<Precision> &data) : m_vector(data) {}

  FortranLinalg::DenseVector<Precision>& getVector() {
    return m_vector;
  }

private:
   FortranLinalg::DenseVector<Precision> m_vector;
};


class DenseVectorEuclideanMetric {
 public:
  Precision distance(DenseVectorSample &a, DenseVectorSample &b) {
    return metric.distance(a.getVector(), b.getVector());
  }
 private:
  EuclideanMetric<Precision> metric;
};