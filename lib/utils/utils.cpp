#include "utils.h"
#include "DenseVectorSample.h"
#include "flinalg/DenseMatrix.h"
#include "hdprocess/HDGenericProcessor.h"


namespace HDProcess {

FortranLinalg::DenseMatrix<Precision> computeDistanceMatrix(
    FortranLinalg::DenseMatrix<Precision> &x) {
  std::vector<DenseVectorSample*> samples;
  for (int j = 0; j < x.N(); j++) {
    FortranLinalg::DenseVector<Precision> vector(x.M());
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

// reinventing Windows... 
std::string uniqueFilename(const std::string &baseName, const std::string &ext) {
  std::string filename(baseName), basename(filename);

  std::ifstream f(filename + ".csv");
  int suffix(0);
  while (!f.good()) {
    // if output file already exists, increment suffix till it's unique
    f.open(basename + "(" + std::to_string(suffix) + ").csv");
    filename = basename + "(" + std::to_string(suffix) + ")";
  }

  return filename + ".csv";
}

} // dspacex
