#include "Precision.h"

#include "LinalgIO.h"
#include "PCA.h"
#include "EuclideanMetric.h"


int main(int argc, char **argv){
  if(argc < 4){
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " dataFile ndims outputFile";
    return 0;
  }
  
  char *dataFile = argv[1];
  int ndims = atoi(argv[2]);
  char *outputFile = argv[3];

  FortranLinalg::DenseMatrix<Precision> data = FortranLinalg::LinalgIO<Precision>::readMatrix(dataFile);
  

  PCA<Precision> pca(data, ndims, true);

  FortranLinalg::DenseMatrix<Precision> proj = pca.project(data, false);
  
  FortranLinalg::LinalgIO<Precision>::writeMatrix(outputFile, proj);

  
  return 0;

}
