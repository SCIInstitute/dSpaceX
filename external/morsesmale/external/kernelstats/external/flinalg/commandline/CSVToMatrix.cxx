#include "Precision.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <math.h>

#include <tclap/CmdLine.h>


int main(int argc, char **argv){

  //Command line parsing
  TCLAP::CmdLine cmd("Convert CSV to binary matrix", ' ', "1");

  TCLAP::ValueArg<std::string> cArg("c","csv","csv input filename", true,
      "", "file");
  cmd.add(cArg);

  TCLAP::ValueArg<std::string> oArg("o","out",
      "output matrix file name", 
      true, "", "file name");
  cmd.add(oArg);


  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }


  std::ifstream file;
  file.open(cArg.getValue().c_str());

  std::string filename = oArg.getValue();
  std::ofstream ofile;
  ofile.open(filename.c_str());

  int n=0;
  int m=0;
  int index =0;
  while(!file.eof()){
    std::string line;
    getline(file, line);
    if(line.size() == 0){
      break;
    }
    std::string token;
    std::istringstream iss(line);
    m=0;
    while ( getline(iss, token, ',') ){
      double v = atof(token.c_str());
      ofile.write((char*) &v, sizeof(double) ); 
      m++;
    }
    n++; 
  }
  file.close();
  ofile.close();

  size_t start = filename.find_last_of("/\\");
  std::string localFile;
  if(start ==std::string::npos){
    localFile = filename;
  }
  else{
    localFile = filename.substr(start+1);
  }

  std::stringstream ss;
  ss << filename << ".hdr";

  std::ofstream hdr;
  hdr.open(ss.str().c_str());

  hdr << "DenseMatrix" << std::endl;
  hdr << "Size: " << m << " x " << n << std::endl;
  hdr << "ElementSize: " << sizeof(double) << std::endl;
  hdr << "RowMajor: " << false << std::endl;
  hdr << "DataFile: " << localFile << std::endl;
  hdr.close();


  return 0;

}
