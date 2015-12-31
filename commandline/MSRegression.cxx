#include "Precision.h"
#include "EnableFloatingPointExceptions.h"

#include "LinalgIO.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "KernelRegression.h"
#include "KernelDensity.h"
#include "GaussianKernel.h"

#include "CmdLine.h"

#include <list>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>


Precision MAX = std::numeric_limits<Precision>::max();

DenseVector<unsigned int> extremaIndex;
DenseVector<unsigned int> cells;
DenseMatrix<unsigned int> edges;
DenseVector<unsigned int> extrema;
DenseVector<Precision> persistance;
DenseVector<int> merge;
DenseVector<bool> isMax;

DenseMatrix<Precision> Xall;
DenseVector<Precision> yall;




void parseMS(std::string &filename){  
  std::ifstream file;
  file.open(filename.c_str());

  std::list<unsigned int> c;
  std::list<unsigned int> ext;
  std::list<Precision> pers;
  std::list<int> mer;
  std::list<bool> eType;

  std::list< DenseVector<unsigned int> > e;


  int type = -1;
  std::string token;
  while(!file.eof()){
    getline(file, token, ' ');
    if(file.eof()) break;

    if(token.compare("#") == 0){
      getline(file, token, ' ');
      if(token.compare("extrema") == 0){
        type = 0;
      }
      else if(token.compare("cells") == 0){
        type = 1;
      }
      else if(token.compare("vertices") == 0){
        type = 2;
      }
      getline(file, token);
    } 
    else if(type == 0){
      eType.push_back(token.compare("max") == 0);

      getline(file, token, ' ');
      ext.push_back(atoi(token.c_str()));     

      getline(file, token, ' ');
      pers.push_back(atof(token.c_str()));

      getline(file, token);
      mer.push_back(atoi(token.c_str()));
    }
    else if(type == 1){
      getline(file, token, ' ');
      int minIndex = atoi(token.c_str());
      getline(file, token);
      int maxIndex = atoi(token.c_str());
      DenseVector<unsigned int> edge(2);
      edge(0) = minIndex;
      edge(1) = maxIndex;
      e.push_back(edge);
    }
    else if(type == 2){
      getline(file, token, ' ');
      c.push_back(atoi(token.c_str()));
      getline(file, token);
    }
    else{
      getline(file, token);
    }
  }


  cells = DenseVector<unsigned int>(c.size());
  int index = 0;
  for(std::list<unsigned int>::iterator it = c.begin(); it != c.end(); ++it, ++index){
    cells(index) = *it;
  }

  edges = DenseMatrix<unsigned int>(2, e.size());  
  index = 0;
  for(std::list<DenseVector<unsigned int> >::iterator it = e.begin(); it != e.end(); ++it, ++index){
    Linalg<unsigned int>::SetColumn(edges, index, *it);
  } 

  extrema = DenseVector<unsigned int>(ext.size());  
  index = 0;
  for(std::list< unsigned int >::iterator it = ext.begin(); it != ext.end(); ++it, ++index){
    extrema(index) = *it;
  }  

  isMax = DenseVector<bool>(eType.size());  
  index = 0;
  for(std::list< bool >::iterator it = eType.begin(); it != eType.end(); ++it, ++index){
    isMax(index) = *it;
  }

  merge = DenseVector<int>(ext.size());  
  index = 0;
  for(std::list< int >::iterator it = mer.begin(); it != mer.end(); ++it, ++index){
    merge(index) = *it;
  }  


  persistance = DenseVector<Precision>(ext.size());  
  index = 0;
  for(std::list< Precision >::iterator it = pers.begin(); it != pers.end(); ++it, ++index){
    persistance(index) = *it;
  }  




  //check edges
  for(unsigned int i=0; i<edges.N(); i++){
    if( isMax(edges(0, i)) == isMax(edges(1, i)) ){
      std::cout << "Crystal " << i << " connects 2 equivavlent extrema types" << std::endl;
    }
    Precision e1 = yall(extrema(edges(0, i)));
    Precision e2 = yall(extrema(edges(1, i)));
    if(e1 < e2 && isMax(edges(0, i)) ){
      std::cout << "Crystal " << i << " min/max assignment inconcistent with data" << std::endl;    
      std::cout << " " << "Extrema " << edges(0, i) << " is labeled as ";
      std::cout << (isMax(edges(0, i))?"Max":"Min") << " Value: " << e1<< std::endl;    
      std::cout << " " << "Extrema " << edges(1, i) << " is labeled as ";
      std::cout << (isMax(edges(1, i))?"Max":"Min") << " Value: " << e2<< std::endl;    
    }
  }

};







void parseGeometry(std::string &filename){
  std::ifstream file;
  file.open(filename.c_str());

  std::list< DenseVector<Precision> > geom;
  std::list< Precision > out;

  std::string token;
  int dimension = -1;
  while(!file.eof()){
    getline(file, token, ' ');
    if(token.compare("d") == 0){
      getline(file, token);
    }
    else if(token.compare("v") == 0){
      getline(file, token);
      std::vector<Precision> subs;
      std::istringstream iss(token);
      std::copy(std::istream_iterator<Precision>(iss),
          std::istream_iterator<Precision>(),
          std::back_inserter< std::vector<Precision> >(subs));
      dimension = subs.size()-1;
      DenseVector<Precision> v(dimension);
      for(int i = 0; i<dimension; i++){
        v(i) = subs[i];
      }
      geom.push_back(v);
      out.push_back(subs[dimension]);

    }
    else{
      getline(file, token);
    }
  }

  int index = 0;
  Xall = DenseMatrix<Precision>(dimension, geom.size());
  for(std::list<DenseVector<Precision> >::iterator it = geom.begin(); it !=
      geom.end(); ++it, ++index){
    Linalg<Precision>::SetColumn(Xall, index, *it);
  }

  index = 0;
  yall = DenseVector<Precision>(geom.size());
  for(std::list< Precision >::iterator it = out.begin(); it !=
      out.end(); ++it, ++index){
    yall(index) = *it;
  }


};




void simplify(Precision pLevel){



  unsigned int mergeIndex = edges.N()+1;
  for(unsigned int i=0; i<extrema.N(); i++){
    if(persistance(i) <= pLevel){
      mergeIndex = i;
      break;
    }
  }
  if(mergeIndex > edges.N()){ return; }

  DenseVector<unsigned int> nextrema(extrema.N()-1);
  DenseVector<unsigned int> nextremaIndex;
  if(extremaIndex.N()!=0){
    nextremaIndex = DenseVector<unsigned int>(extrema.N()-1);
  }
  DenseVector<Precision> npersistance(extrema.N()-1);
  DenseVector<int> nmerge(extrema.N()-1);
  DenseVector<bool> nisMax(extrema.N()-1);
  int index = 0;
  for(unsigned int i=0; i<extrema.N(); i++){
    if(i==mergeIndex){ continue; }
    if(extremaIndex.N() != 0 ){
      nextremaIndex(index) = extremaIndex(i);
    }
    nextrema(index) = extrema(i);
    npersistance(index) = persistance(i);
    nmerge(index) = merge(i);
    if(nmerge(index) > (int)mergeIndex){
      nmerge(index) = nmerge(index) -1;
    }
    nisMax(index) = isMax(i);

    index++;
  }

  DenseVector<int> mergeEdges(edges.N());
  int nMerges = 0;
  for(unsigned int i=0 ; i<edges.N(); i++){
    mergeEdges(i) = i;
    if(edges(0, i) == mergeIndex){
      for(unsigned int j=0; j<edges.N(); j++){
        if(i==j) continue;
        if(edges(1, i) == edges(1, j) && (int) edges(0, j) == merge(mergeIndex)){
          mergeEdges(i) = j;
          nMerges++;  
          break;
        }
      }
      //edges(0, i) = merge(mergeIndex);
    }
    else if(edges(1, i) == mergeIndex){
      for(unsigned int j=0; j<edges.N(); j++){
        if(i==j) continue;
        if(edges(0, i) == edges(0, j) && (int) edges(1, j) == merge(mergeIndex)){
          mergeEdges(i) = j;
          nMerges++;
          break;
        }
      }
      //edges(1, i) = merge(mergeIndex);
    }
  }
  for(unsigned int i=0 ; i<edges.N(); i++){
    if(edges(0, i) == mergeIndex){
      edges(0, i) = merge(mergeIndex);
    }
    else if(edges(1, i) == mergeIndex){
      edges(1, i) = merge(mergeIndex);
    }
  }

  /*for(unsigned int i=0; i<mergeEdges.N(); i++){
    while(mergeEdges(i) != mergeEdges(mergeEdges(i))){
    mergeEdges(i) = mergeEdges(mergeEdges(i));
    }
    }*/



  DenseMatrix<unsigned int> nedges(2, edges.N() - nMerges);
  DenseVector<int> cellMerge(edges.N());
  index = 0;
  for(unsigned int i=0; i< edges.N(); i++){
    int mIndex = mergeEdges(i);
    if(mIndex != -1){
      nedges(0, index) = edges(0, mIndex);
      nedges(1, index) = edges(1, mIndex);
      for(unsigned int j=0; j<edges.N(); j++){
        if(mergeEdges(j) == mIndex){
          mergeEdges(j) = -1;
          cellMerge(j) = index;
        }
      }
      index++;
    }
  }

  for(unsigned int i=0; i<nedges.N(); i++){
    if(nedges(0, i) > mergeIndex){
      nedges(0, i) = nedges(0, i) - 1;
    }
    if(nedges(1, i) > mergeIndex){
      nedges(1, i) = nedges(1, i) - 1;
    }
  }

  DenseVector<unsigned int> ncells(cells.N());
  for(unsigned int i=0; i<cells.N(); i++){
    ncells(i) = cellMerge(cells(i));
  }    

  cellMerge.deallocate();
  mergeEdges.deallocate(); 

  cells.deallocate();
  cells = ncells;

  edges.deallocate();
  edges = nedges;

  extrema.deallocate();
  extrema = nextrema;


  extremaIndex.deallocate();
  extremaIndex = nextremaIndex;

  isMax.deallocate();
  isMax = nisMax;

  persistance.deallocate();
  persistance = npersistance;

  merge.deallocate();
  merge = nmerge;

}








int main(int argc, char **argv){

  //Command line parsing
  TCLAP::CmdLine cmd("MS-Regression", ' ', "1");

  TCLAP::ValueArg<Precision> sigmaArg("","sigma",
      "kernel density estimation sigma", 
      true, 1,  "float");
  cmd.add(sigmaArg);  

  TCLAP::ValueArg<std::string> msArg("m","ms","Morse-Smale complex information file", 
      true,  "", "");
  cmd.add(msArg);

  TCLAP::ValueArg<std::string> gArg("g","geom","Geometry information file", 
      true,  "", "");
  cmd.add(gArg);


  TCLAP::ValueArg<std::string> evalArg("X","Xeval","Data matrix file", 
      true,  "", "");
  cmd.add(evalArg);

  TCLAP::ValueArg<std::string> outArg("o","out","output vector file", 
      false,  "Regressed.data", "");
  cmd.add(outArg);

  TCLAP::ValueArg<int> pArg("p","persistance",
      "Number of persitsance levels to compute; -1 = all", 
      true, -1,  "integer");
  cmd.add(pArg);  

  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }


  try{

    //Parse geometry 
    parseGeometry(gArg.getValue());
    //Parse Morse-Smale Complex
    parseMS(msArg.getValue());

    //Number of nearest neighbohrs for sigma computation
    Precision sigma = sigmaArg.getValue();

    DenseMatrix<Precision> Xtest =
      LinalgIO<Precision>::readMatrix(evalArg.getValue());

    DenseVector<Precision> pSorted = Linalg<Precision>::Copy(persistance);
    std::sort(pSorted.data(), pSorted.data()+pSorted.N());
    int nlevels = pArg.getValue();
    int start = 0;
    if(nlevels > 0){
      start = pSorted.N() - nlevels-1;
    }
    if(start < 0){
      start = 0;
    }

    std::cout << start << std::endl;
    for(int i=0; i<start; i++){
      simplify(pSorted(i));
    }


    std::vector<unsigned int> Xi[edges.N()];

    for(unsigned int cellIndex = 0; cellIndex < edges.N(); ++cellIndex){
      for(unsigned int i=0; i< cells.N(); i++){
        if(cells(i) == cellIndex){
          Xi[cellIndex].push_back(i);
        }
      }
    }




    GaussianKernel<Precision> gaussian(sigma, Xall.M());
    KernelDensity<Precision> kda(Xall, gaussian);

    KernelDensity<Precision> *kd[edges.N()];
    for(unsigned int cellIndex = 0; cellIndex < edges.N(); cellIndex++){
      //Extract samples and function values from cell
      DenseMatrix<Precision> X(Xall.M(), Xi[cellIndex].size());
      DenseMatrix<Precision> y(1, X.N());
      for(unsigned int i=0; i< X.N(); i++){ 
        unsigned int index = Xi[cellIndex][i];
        Linalg<Precision>::SetColumn(X, i, Xall, index);
        y(0, i) = yall(index);
      }
      kd[cellIndex] = new KernelDensity<Precision>(X, gaussian);
    } 

    std::cout << edges.N() << std::endl;
    DenseVector<Precision> x(Xall.M());
    DenseVector<Precision> p(edges.N());
    Precision ni = 1+Xall.M();
    DenseMatrix<Precision> A(Xall.N(), ni*edges.N());
    DenseMatrix<Precision> b(Xall.N(), 1);
    Precision num = 1.f/edges.N();
    for(int i=0; i<Xall.N(); i++){
      Linalg<Precision>::ExtractColumn(Xall, i, x);
      Precision wsum = 0;
      for(int j=0; j<edges.N(); j++){
        p(j) = kd[j]->p(x);
        wsum +=  p(j);
      }    
      for(int k=0; k<edges.N(); k++){
        Precision w = p(k) / wsum;//cells(i) == k;//tmp(0);
        A(i, k*ni) = w;
        for(int j=0; j<x.N(); j++){
          A(i, k*ni+1+j) = w*x(j);
        }
        b(i, 0) = yall(i);
      }
    }
    std::cout << std::endl;
    LinalgIO<Precision>::writeMatrix("A.data", A);
    LinalgIO<Precision>::writeMatrix("b.data", b);


    DenseMatrix<Precision> params = Linalg<Precision>::LeastSquares(A, b);
    LinalgIO<Precision>::writeMatrix("params.data", params);


    DenseVector<Precision> fy(Xtest.N());

    for(int i=0;i<Xtest.N(); i++){
      Linalg<Precision>::ExtractColumn(Xtest, i, x);
      Precision yr = 0;
      //Precision px = kd.p(x)/Xall.N();
      Precision wsum = 0;
      for(int k=0; k<edges.N(); k++){
        Precision w = kd[k]->p(x);
        wsum+=w;
        yr += w*params(k*ni, 0); 
        for(int j=0; j<x.N(); j++){
          yr += w*x(j)*params(k*ni+1+j, 0);
        }
      }
      fy(i) = yr/wsum;
    }

    LinalgIO<Precision>::writeVector(outArg.getValue(), fy);
  }
  catch(const char *err){
    std::cerr << err << std::endl;
  }

  return 0;

}

