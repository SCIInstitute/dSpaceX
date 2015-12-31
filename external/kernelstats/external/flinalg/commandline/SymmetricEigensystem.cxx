#include "Precision.h"

#include "LinalgIO.h"

#include "SymmetricEigensystem.h"
#include "DenseMatrix.h"

#include <iostream>



void save(FortranLinalg::SymmetricEigensystem<Precision> &es, char *file){
  using namespace FortranLinalg;
  std::cout << "info: " << es.info << std::endl;
  std::cout << "nEigenvectors found: " << es.nEigenvectors << std::endl;
  
  std::stringstream ss1;
  ss1 << file << ".evecs";
  LinalgIO<Precision>::writeMatrix(ss1.str().c_str(), es.ev);
  
  std::stringstream ss2;
  ss2 << file << ".evals";
  LinalgIO<Precision>::writeVector(ss2.str().c_str(), es.ew);
}




int main(int argc, char **argv){
  using namespace FortranLinalg;
  if(argc < 4){
    std::cout << "Usage: " << std::endl;
    std::cout << "matrixFile N  outputFile [evalsOnly il ih]" << std::endl;
    return -1;
  }
 
  
  int argIndex = 1;
  char *file = argv[argIndex++];
  int N = atoi(argv[argIndex++]);
  char *outfile = argv[argIndex++];

  //optional params
  bool evalsOnly = false; 
  if(argIndex < argc){
    evalsOnly = atoi(argv[argIndex++]) != 0;
  }
  int il = -1;
  int ih = -1;
  if(argIndex < argc){
    il = atoi(argv[argIndex++]);
    ih = atoi(argv[argIndex++]);  
  }; 

  
  DenseMatrix<Precision> matrix(N, N); 
  LinalgIO<Precision>::readMatrix(file, matrix);

  if(il == -1){
    SymmetricEigensystem<Precision> se(matrix, evalsOnly);
    save(se, outfile);
  }else{
    SymmetricEigensystem<Precision> se(matrix, il, ih, evalsOnly);
    save(se, outfile);
  }
  return 0;

}
