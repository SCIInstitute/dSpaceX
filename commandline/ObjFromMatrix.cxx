#include "Precision.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <math.h>

#include "DenseMatrix.h"
#include "Geometry.h"
#include <tclap/CmdLine.h>
#include "SquaredEuclideanMetric.h"
#include "LinalgIO.h"


int main(int argc, char **argv){
  
  //Command line parsing
  TCLAP::CmdLine cmd("Create Object file from Data matrix", ' ', "1");

  TCLAP::ValueArg<std::string> mArg("m","matrix","data matrix", true,
      "", "file");
  cmd.add(mArg);
  
  TCLAP::ValueArg<std::string> fArg("f","function","function value", true,
      "", "file");
  cmd.add(fArg);
  
  TCLAP::ValueArg<std::string> oArg("o","out",
      "outputfile name", 
      true, "", "file name");
  cmd.add(oArg);

  TCLAP::ValueArg<unsigned int> kArg("k","knn","number of nearest neighbors for graph structure", 
      false, 10, "integer");
  cmd.add(kArg);

  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }



  DenseMatrix<Precision> X = LinalgIO<Precision>::readMatrix(mArg.getValue());
  DenseVector<Precision> y = LinalgIO<Precision>::readVector(fArg.getValue());

  unsigned int K = kArg.getValue();
  std::string out = oArg.getValue();



  //Compute nearest neighbors
  DenseMatrix<Precision> knnDist(K+1, X.N());
  DenseMatrix<int> knn(K+1, X.N());
  SquaredEuclideanMetric<Precision> sl2;
  Geometry<Precision>::computeKNN(X, knn, knnDist, sl2);


  std::ofstream obj;
  obj.open(out.c_str());
  for(unsigned int i=0; i< X.N(); i++){
    obj << "v ";
    for(unsigned int j=0; j<X.M(); j++){
      obj << X(j, i) << " ";
    }
    obj  << y(i) << std::endl;
  }   
  for(unsigned int i=0; i< X.N(); i++){
    for(unsigned int k=1; k<knn.M(); k++){
      obj << "e " << i + 1 << " " << knn(k, i) + 1 << std::endl;
    }

  }   
  obj.close();

  
  return 0;

}
