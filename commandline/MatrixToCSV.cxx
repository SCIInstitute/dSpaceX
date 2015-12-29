#include "Precision.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <math.h>

#include "DenseMatrix.h"
#include <tclap/CmdLine.h>
#include "LinalgIO.h"


int main(int argc, char **argv){
  
  //Command line parsing
  TCLAP::CmdLine cmd("Create Object file from Data matrix", ' ', "1");

  TCLAP::ValueArg<std::string> mArg("m","matrix","data matrix", true,
      "", "file");
  cmd.add(mArg);
  
  TCLAP::ValueArg<std::string> oArg("o","out",
      "outputfile name", 
      true, "", "file name");
  cmd.add(oArg);


  try{
	  cmd.parse( argc, argv );
	} 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }



  FortranLinalg::DenseMatrix<Precision> X = FortranLinalg::LinalgIO<Precision>::readMatrix(mArg.getValue());

  std::string out = oArg.getValue();





  std::ofstream obj;
  obj.open(out.c_str());
  for(unsigned int i=0; i< X.N(); i++){
    for(unsigned int j=0; j<X.M(); j++){
      obj << X(j, i) << " ";
    }
    obj << std::endl;
  }   
  
  obj.close();

  
  return 0;

}
