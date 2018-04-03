#include "Precision.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <math.h>

#include "DenseMatrix.h"
#include <tclap/CmdLine.h>
#include "LinalgIO.h"
//#include "Linalg.h"

extern "C"{
  #include "wordom/fileio.h"
};


int main(int argc, char **argv){
  
  //Command line parsing
  TCLAP::CmdLine cmd("Create matrix from dcd", ' ', "1");

  TCLAP::ValueArg<std::string> dArg("d","dcd","dcd file", true,
      "", "file");
  cmd.add(dArg);
  
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



  std::string dcdName = dArg.getValue();
  std::string outName = oArg.getValue();

  Traj *trj = InitTrj(dcdName.c_str(), "r");
 
  CoorSet *data = InitCoor(trj->hdr->nato);

  DenseMatrix<double> X(trj->hdr->nato*3, trj->hdr->nframe);
  //Linalg<double>::Set(X, 1);
  for(int i=0; i<trj->hdr->nframe; i++){ 
    ReadTrjCoor(trj, data, i+1);
    int off=0;
    for(int k=0; k<trj->hdr->nato; k++, off+= 3 ){
      X(off, i) = data->xcoor[k];
      X(off+1, i) = data->ycoor[k];
      X(off+2, i) = data->zcoor[k];
    } 
  }
 
  LinalgIO<double>::writeMatrix(outName, X); 
  return 0;

}
