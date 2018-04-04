#include "Precision.h"

#include "LinalgIO.h"
#include "MetricMDS.h"
#include "EuclideanMetric.h"


int main(int argc, char **argv){
  using namespace FortranLinalg;
  if(argc < 4){
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " dataFile ndims outputFile";
    return 0;
  }
  
  char *dataFile = argv[1];
  int ndims = atoi(argv[2]);
  char *outputFile = argv[3];

  DenseMatrix<Precision> distances = LinalgIO<Precision>::readMatrix(dataFile);
  if(ndims <= 0){
    ndims = distances.N();
  }

  EuclideanMetric<Precision> metric;

  MetricMDS<Precision> mds;

  DenseMatrix<Precision> Y = mds.embed(distances, ndims);

  LinalgIO<Precision>::writeMatrix(outputFile, Y);

  
  return 0;

}
