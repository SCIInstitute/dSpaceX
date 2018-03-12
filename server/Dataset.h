#include "Precision.h"
#include "Linalg.h"
#include "LinalgIO.h"


class Dataset {
 public:  
  virtual int numberOfSamples() const = 0;
  virtual int numberOfQois() const = 0;
  virtual FortranLinalg::DenseMatrix<Precision> getDistanceMatrix() = 0;
  virtual FortranLinalg::DenseVector<Precision> getQoiVector(int i) = 0;
  virtual std::string getName() const = 0;
};