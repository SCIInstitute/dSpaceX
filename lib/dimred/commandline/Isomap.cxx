#include "Precision.h"

#include "LinalgIO.h"
#include "Isomap.h"
#include "EuclideanMetric.h"
#include "EpsilonNeighborhood.h"
#include "TangentEpsilonNeighborhood.h"
#include "TangentKNNNeighborhood.h"
#include "KNNNeighborhood.h"

#include "MetricTensorKNNNeighborhood.h"

#include <limits>

Precision MAX = std::numeric_limits<Precision>::max();

int main(int argc, char **argv){
  using namespace FortranLinalg;
  if(argc < 6){
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " dataMatrixHeader outputFile ndims neighborhoodtype [params]" << std::endl;;
    std::cout << "neighborhoodtypes:" <<std::endl;
    std::cout << "  0 = KNNNeighborhood, param = knnm" << std::endl;
    std::cout << "  1 = EpsilonNeighborhood, param = epsilon" << std::endl;
    std::cout << "  2 = TangentEpsilonNeighborhood, param = epsilon" << std::endl;
    std::cout << "  3 = TangentKNNNeighborhood, param = knn" << std::endl;
    std::cout << "  4 = MetricTensorKNNNeighborhood, params = knn alpha" << std::endl;
    std::cout << std::endl;

    return 0;
  }
  
  char *dataFile = argv[1];
  char *outputFile = argv[2];
  int ndims = atoi(argv[3]);
  int ntype = atoi(argv[4]);

  DenseMatrix<Precision> data = LinalgIO<Precision>::readMatrix(dataFile);
  

  Neighborhood<Precision> *nh;
  if(ntype == 0){
    nh = new KNNNeighborhood<Precision>(atoi(argv[5]), MAX);
  }
  else if(ntype == 1){
    EuclideanMetric<Precision> metric;
    nh = new EpsilonNeighborhood<Precision>(metric, atof(argv[5]), MAX);
  }
  else if(ntype == 2){
    nh = new TangentEpsilonNeighborhood<Precision>(atof(argv[5]), MAX);
  }
  else if(ntype == 3){
    nh = new TangentKNNNeighborhood<Precision>(atoi(argv[5]), MAX);
  }
  else if(ntype == 4){
    nh = new MetricTensorKNNNeighborhood<Precision>(atoi(argv[5]), atof(argv[6]),MAX);
  }
  else{
    std::cout << "not a valid neighborhoodtype" << std::endl;
    return 1;
  }


  Isomap<Precision> isomap(nh, ndims);

  DenseMatrix<Precision> Y = isomap.embed(data);

  std::cout << "Saving embedding" << std::endl;
  LinalgIO<Precision>::writeMatrix(outputFile, Y);
  
  return 0;
}
