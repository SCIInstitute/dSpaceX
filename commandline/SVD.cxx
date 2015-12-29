#include "Precision.h"

#include <stdio.h>
#include <time.h>

#include "LinalgIO.h"
#include "SVD.h"
#include "SVD-dgesdd.h"
#include "RandomSVD.h"

#include <tclap/CmdLine.h>

int main(int argc, char **argv){
  
  //Command line parsing
  TCLAP::CmdLine cmd("SVD", ' ', "1");


  TCLAP::ValueArg<int> dArg("d","dimension", "Dimension for randomized SVD", true, 10, "integer");
  cmd.add(dArg);
  
  TCLAP::ValueArg<int> pArg("p","power", "Number of power iterations for randomized SVD", true, 1, "integer");
  cmd.add(pArg);
  
  TCLAP::ValueArg<std::string> dataArg("x","data", "Data file",  true, "", "matrix header file");
  cmd.add(dataArg);

  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }

  using namespace FortranLinalg;

  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(dataArg.getValue());
  int d = dArg.getValue();
  int p = pArg.getValue();

  clock_t t1 = clock();
  SVD<Precision> svd(X);
  clock_t t2 = clock();
  std::cout << "SVD" << std::endl;
  std::cout << (t2-t1)/(double)CLOCKS_PER_SEC << std::endl;
  LinalgIO<Precision>::writeVector("S.data", svd.S);
  LinalgIO<Precision>::writeMatrix("U.data", svd.U);

  t1 = clock();
  RandomSVD<Precision> rsvd(X, d, p);
  t2 = clock();
  std::cout << "Random SVD" << std::endl;
  std::cout << (t2-t1)/(double)CLOCKS_PER_SEC << std::endl;
  LinalgIO<Precision>::writeVector("rS.data", rsvd.S);
  LinalgIO<Precision>::writeMatrix("rU.data", rsvd.U);
 
  t1 = clock();
  SVDdgesdd<Precision> svd2(X);
  t2 = clock();
  std::cout << "SVD" << std::endl;
  std::cout << (t2-t1)/(double)CLOCKS_PER_SEC << std::endl;
  LinalgIO<Precision>::writeVector("S2.data", svd2.S);
  LinalgIO<Precision>::writeMatrix("U2.data", svd2.U);


  
  return 0;

}
