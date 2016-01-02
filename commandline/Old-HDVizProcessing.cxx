#include "Precision.h"

#include "LinalgIO.h"
#include "UKR.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "PCA.h"
//#include "EnableFloatingPointExceptions.h"
#include "CmdLine.h"
#include "KNNNeighborhood.h"
#include "Isomap.h"
#include "KernelRegression.h"

#include <list>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>


Precision MAX = std::numeric_limits<Precision>::max();


DenseMatrix<Precision> extremaPosPCA;
DenseMatrix<Precision> extremaPosPCA2;
DenseMatrix<Precision> extremaPosIso;
DenseVector<unsigned int> extremaIndex;
int translationIndex = -1;

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





void fit(DenseMatrix<Precision> &E, DenseMatrix<Precision> &Efit){
  DenseMatrix<Precision> Eold(extremaIndex.N(), 2);
  DenseMatrix<Precision> Enew(extremaIndex.N(), 2);

  //find translationIndex minima
  int t2 = -1;
  for(unsigned int i=0; i<extrema.N(); i++){
    if((int) extremaIndex(i) == translationIndex){
      t2 = i;
      break;
    }
  }
  std::cout << t2 << std::endl;
  for(unsigned int i=0; i< extremaIndex.N(); i++){
    for(unsigned int j=0; j<2; j++){
      Eold(i, j) = Efit(j, extremaIndex(i)) - Efit(j, translationIndex);
      Enew(i, j) = E(j, i) - E(j, t2);
    }   
    //Enew(i, 2) = 1;
    //Eold(i, 2) = 1;
  }
  DenseMatrix<Precision> Etmp = Linalg<Precision>::Copy(Enew);
  DenseMatrix<Precision> T = Linalg<Precision>::LeastSquares(Etmp, Eold);
  DenseMatrix<Precision> ET = Linalg<Precision>::Multiply(Enew, T);
  for(unsigned int i=0; i< extremaIndex.N(); i++){
    for(unsigned int j=0; j<2; j++){
      E(j, i) = ET(i, j) + Efit(j, translationIndex);
    }   
  }
  T.deallocate();
  Eold.deallocate();
  Enew.deallocate();
  Etmp.deallocate();
  ET.deallocate();
}



int main(int argc, char **argv){
  
  //Command line parsing
  TCLAP::CmdLine cmd("Process MS-Complex and compute cell manifolds", ' ', "1");

  TCLAP::ValueArg<Precision> sigmaArg("","sigma",
      "Percentage of range to compute kernel sigma", 
      true, 0.2,  "float");
  cmd.add(sigmaArg);  
    
  TCLAP::ValueArg<std::string> msArg("m","ms","Morse-Smale complex information file", 
      true,  "", "");
  cmd.add(msArg);

  TCLAP::ValueArg<std::string> gArg("g","geom","Geometry information file", 
      true,  "", "");
  cmd.add(gArg);


  TCLAP::ValueArg<int> pArg("p","persitance",
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

    //Number of samples for geomtric layout
    int nSamples = 50;
    //Number of nearest neighbohrs for sigma computation
    Precision sigma = sigmaArg.getValue();


    //Save geometries and Morse Smale infomration
    std::string extFile = "Extrema.data";
    LinalgIO<unsigned int>::writeVector(extFile, extrema);


    std::string geomFile = "Geom.data";
    LinalgIO<Precision>::writeMatrix(geomFile, Xall);   

    std::string fFile = "Function.data";
    LinalgIO<Precision>::writeVector(fFile, yall);   

    std::string pFile = "Persistance.data";
    LinalgIO<Precision>::writeVector(pFile, persistance);  

    DenseVector<Precision> pSorted = Linalg<Precision>::Copy(persistance);
    std::sort(pSorted.data(), pSorted.data()+pSorted.N());

    std::string psFile = "PersistanceSorted.data";
    LinalgIO<Precision>::writeVector(psFile, pSorted);  


    int nlevels = pArg.getValue();
    int start = 0;
    if(nlevels > 0){
      start = pSorted.N()-1 - nlevels;
    }
    if(start < 0){
      start = 0;
    }
    DenseVector<Precision> pStart(1);
    pStart(0) = start;
    LinalgIO<Precision>::writeVector("PersistanceStart.data", pStart);  

    std::cout << start << std::endl;
    for(int i=0; i<start; i++){
      simplify(pSorted(i));
    }
    for(unsigned int nP = start; nP < pSorted.N()-1; nP++){

      //Matrix of all samples plus extrema for geomtric low dimensional layout - MDS  
      std::stringstream edgesFile;
      edgesFile << "Edges_" << nP << ".data";
      LinalgIO<unsigned int>::writeMatrix(edgesFile.str(), edges);    

      std::cout << std::endl << "PersistanceLevel: " << nP << std::endl;
      std::cout << "# of Crystals: " << edges.N() << std::endl;
      std::cout << "=================================" << std::endl << std::endl;

      DenseMatrix<Precision> S(Xall.M(), edges.N()*nSamples+extrema.N());
      DenseMatrix<Precision> Scell[edges.N()];
      DenseMatrix<Precision> Xcell[edges.N()];
      DenseMatrix<Precision> Xpcell[edges.N()];
      DenseMatrix<Precision> ycell[edges.N()];
      DenseVector<Precision> eWidths(extrema.N());
      Linalg<Precision>::Zero(eWidths);

      std::vector<unsigned int> Xi[edges.N()];
      std::vector<unsigned int> Xiorig[edges.N()];
      std::vector<Precision> yci[edges.N()];
      EuclideanMetric<Precision> l2;

      //Compute regression for each Morse-Smale cell
      for(unsigned int cellIndex = 0; cellIndex < edges.N(); ++cellIndex){
        for(unsigned int i=0; i< cells.N(); i++){
          if(cells(i) == cellIndex){
            Xiorig[cellIndex].push_back(i);
            Xi[cellIndex].push_back(i);
            yci[cellIndex].push_back(yall(i));
          }
        }
      }


      for(unsigned int a=0; a<edges.N(); a++){
        for(unsigned int b=0; b<edges.N(); b++){
          if(a == b) continue;
          int ea1 = edges(0, a);
          int ea2 = edges(1, a);
          int eb1 = edges(0, b);
          int eb2 = edges(1, b);
          bool touch = false;
          Precision val = 0;
          if(ea1 == eb1 || ea1 == eb2){
            val = yall(extrema(ea1));
            touch = true;
          }
          if(ea2 == eb1 || ea2 == eb2){
            val = yall(extrema(ea2));
            touch = true;
          }
          //add points within sigma of extrema to this points of a
          if(touch){
            for(unsigned int i=0; i< Xiorig[b].size(); i++){
              unsigned int index = Xiorig[b][i];
              if(fabs(val - yall(index)) < 2*sigma){
                Xi[a].push_back(index);
                yci[a].push_back(val + val - yall(index));
              }
              //Xi[a].push_back(index);
              //yci[a].push_back(val + val - yall(index));
            } 
          }
        }
      }

      for(unsigned int cellIndex = 0; cellIndex < edges.N(); cellIndex++){

        //Extract samples and function values from cell
        DenseMatrix<Precision> X(Xall.M(), Xi[cellIndex].size());
        DenseMatrix<Precision> y(1, X.N());
        for(unsigned int i=0; i< X.N(); i++){
          unsigned int index = Xi[cellIndex][i];
          Linalg<Precision>::SetColumn(X, i, Xall, index);
          y(0, i) = yci[cellIndex][i];
        }
        Xcell[cellIndex] = Linalg<Precision>::Copy(X);
        ycell[cellIndex] = Linalg<Precision>::Copy(y);


        //Compute Rgeression curve
        std::cout << "Computing regression curve for cell " << cellIndex << std::endl;
        std::cout << X.N() << " points" << std::endl;
        

       //UKR<Precision> ukr(X, y, 15, sigma);
        GaussianKernel<Precision> gk(sigma);
        KernelRegression<Precision> kr(y, X, gk);
      



        //Get locations
        //DenseMatrix<Precision> Zend = ukr.getZ();
        //DenseMatrix<Precision> Zend=X;
        
        //Xpcell[cellIndex] = ukr.project(X);
        //Xpcell[cellIndex] = kr.evaluate(y);


        //Compute min and max function value
        int e1 = edges(0, cellIndex);
        int e2 = edges(1, cellIndex);
        Precision zmax = yall(extrema(e1));
        Precision zmin = yall(extrema(e2));
        if(zmin > zmax){
          std::swap(zmin, zmax);
          std::swap(e1, e2);
        } 

        //Create samples (regressed in input space) between min and max function values 
        DenseVector<Precision> tmp(Xall.M());
        DenseVector<Precision> z(1);
        DenseMatrix<Precision> Zp(1, nSamples);
        Scell[cellIndex] = DenseMatrix<Precision>(Xall.M(), nSamples);
        DenseMatrix<Precision> gStmp(Xall.M(), 1);
        Linalg<Precision>::Zero(gStmp);
        DenseMatrix<Precision> gradS(Xall.M(), nSamples);
        DenseVector<Precision> pdist(nSamples);
        for(int k=0; k < nSamples; k++){
          z(0) = zmin + (zmax-zmin) * ( k/ (nSamples-1.f) );

          Zp(0, k) = z(0);;
          //ukr.g(z, tmp, gStmp);
          Precision sd = kr.g(z, tmp);
          pdist(k) = sd;
          Linalg<Precision>::SetColumn(S, cellIndex*nSamples + k, tmp);
          Linalg<Precision>::SetColumn(Scell[cellIndex], k, tmp);
          Linalg<Precision>::SetColumn(gradS, k, gStmp, 0);
        }
        z.deallocate();
        tmp.deallocate();


        //Compute mapping back onto curve should be equal to
        /*
        std::stringstream ss31;
        ss31 << "ps_" << nP << "_cell_" << cellIndex << "_Yp.data";
        LinalgIO<Precision>::writeMatrix(ss31.str(), Xpcell[cellIndex]);

        std::stringstream ss1;
        ss1 << "ps_" << nP << "_cell_" << cellIndex << "_Zs.data";
        LinalgIO<Precision>::writeMatrix(ss1.str(), Zp);
*/

        std::stringstream ss2;
        ss2 << "ps_" << nP << "_cell_" << cellIndex << "_Rs.data";
        LinalgIO<Precision>::writeMatrix(ss2.str(), Scell[cellIndex]);


        std::stringstream ss2a;
        ss2a << "ps_" << nP << "_cell_" << cellIndex << "_gradRs.data";
        LinalgIO<Precision>::writeMatrix(ss2a.str(), gradS);
        gradS.deallocate(); 

/*
        //Compute average projection distances at sampled locations
        DenseVector<Precision> pdist(nSamples);
        GaussianKernel<Precision> &kernel = ukr.getKernelX();
        for(unsigned int i = 0; i<pdist.N(); i++){
          Precision sum = 0;
          Precision tmp = 0;
          for(unsigned int j=0; j < Zend.N(); j++){
            Precision k = kernel.f(Zp, i, Zend, j);
            sum += k;
            tmp += l2.distance(X, j, Xpcell[cellIndex], j) * k; 
          }
          pdist(i) = tmp / sum;
        }
*/

        std::stringstream ss6;
        ss6 <<"ps_" << nP << "_cell_" << cellIndex << "_mdists.data";
        LinalgIO<Precision>::writeVector(ss6.str(), pdist);



        //compute maximal extrema widths
        if(eWidths(e2) < pdist(0)){
          eWidths(e2) = pdist(0); 
        }
        if(eWidths(e1) < pdist(nSamples-1)){
          eWidths(e1) = pdist(nSamples-1); 
        }

        pdist.deallocate();


        
        //Compute function value mean at sampled locations
        DenseVector<Precision> fmean(Zp.N());
        for(unsigned int i=0; i < Zp.N(); i++){
          /*Precision sum = 0;
            Precision tmp = 0;
            for(unsigned int j=0; j < Zend.N(); j++){
            Precision k = kernel.f(Zp, i, Zend, j);
            sum += k;
            tmp += y(0, j) * k; 
            }*/ 
          fmean(i) = Zp(0, i);//tmp / sum;
        }
        std::stringstream ss7;
        ss7 <<"ps_" << nP << "_cell_" << cellIndex << "_fmean.data";
        LinalgIO<Precision>::writeVector(ss7.str(), fmean);
        


        //Compute sample density 
        DenseVector<Precision> spdf(Zp.N());
        for(unsigned int i=0; i < Zp.N(); i++){
          Precision sum = 0;
          for(unsigned int j=0; j < Zend.N(); j++){
            Precision k = kernel.f(Zp, i, Zend, j);
            sum += k;
          } 
          spdf(i) = sum/Xall.N();
        }
        std::stringstream ss8;
        ss8 <<"ps_" << nP << "_cell_" << cellIndex << "_spdf.data";
        LinalgIO<Precision>::writeVector(ss8.str(), spdf);
        spdf.deallocate(); 


        
        //Compute sample variance
        DenseMatrix<Precision> Svar(Xall.M(), nSamples);
        for(unsigned int i=0; i < Zp.N(); i++){
          for(unsigned int m=0; m<Svar.M(); m++){

            Precision sum = 0;
            Precision tmp = 0;
            for(unsigned int j=0; j < Zend.N(); j++){
              Precision k = kernel.f(Zp, i, Zend, j);
              sum += k;
              tmp += (X(m, j) - Xpcell[cellIndex](m, j))*(X(m,
                    j)-Xpcell[cellIndex](m, j)) * k; 
            } 
            Svar(m, i) = sqrt(tmp/sum);
          }
        }
        std::stringstream ss812;
        ss812 <<"ps_" << nP << "_cell_" << cellIndex << "_Svar.data";
        LinalgIO<Precision>::writeMatrix(ss812.str(), Svar);
        Svar.deallocate();
       



        ukr.cleanup();
        Zp.deallocate();

      }


      //Add extremal poitns for computing layout
      for(unsigned int i=0; i<extrema.N(); i++){
        //compute average extremal point
        /*
        std::list< int > adjPnts;
        int n = 0;
        for(unsigned int k=0; k < edges.N(); k++){
          if(edges(0, k) == i || edges(1, k) == i){
            adjPnts.push_back(k);
            n += Xcell[k].N();
          }
        }
        DenseMatrix<Precision> Xtmp(Xall.M(), n);
        DenseMatrix<Precision> ytmp(1, n);
        int index = 0;
        std::list< int >::iterator it = adjPnts.begin();
        while(it != adjPnts.end()){
          int cIndex = *it;
          for(unsigned int k=0; k<Xcell[cIndex].N(); k++){
            Linalg<Precision>::SetColumn(Xtmp, index, Xcell[cIndex], k);
            ytmp(0, index) = ycell[cIndex](0, k);
            ++index;
          }
          ++it;
        }
        GaussianKernel<Precision> gk(sigma, 1);
        KernelRegression<Precision> kd(Xtmp, ytmp, gk);
        DenseVector<Precision> eval(1);
        eval(0) = yall(extrema(i));
        DenseVector<Precision> out(Xall.M());
        kd.evaluate(eval, out);


        Linalg<Precision>::SetColumn(S, nSamples*edges.N()+i, out);



        out.deallocate();
        eval.deallocate();
        ytmp.deallocate();
        Xtmp.deallocate();
        */        
        
        DenseVector<Precision> out(Xall.M());
        Linalg<Precision>::Zero(out);
        int n = 0;
        for(unsigned int k=0; k < edges.N(); k++){
          if(edges(0, k) == i || edges(1, k) == i){
            if(isMax(i)){
              Linalg<Precision>::Add(out, Scell[k], nSamples-1, out); 
            }
            else{
              Linalg<Precision>::Add(out, Scell[k], 0, out);
            }
            n++;
          }
        }
        Linalg<Precision>::Scale(out, 1.f/n, out);
        Linalg<Precision>::SetColumn(S, nSamples*edges.N()+i, out);
        out.deallocate();

      }
      
      
      std::stringstream extw;
      extw << "ExtremaWidths_" << nP << ".data";
      LinalgIO<Precision>::writeVector(extw.str(), eWidths);
      eWidths.deallocate();



      ///----- Complete PCA layout 

      //Compute low-d layout of geomtry using PCA  
      unsigned int dim = 2; 
      PCA<Precision> pca(S, dim);        
      DenseMatrix<Precision> fL = pca.project(S);
      if(fL.M() < dim){
        DenseMatrix<Precision> fLtmp(dim, fL.N());
        Linalg<Precision>::Zero(fLtmp);
        for(unsigned int i=0; i<fL.M(); i++){
          Linalg<Precision>::SetRow(fLtmp, i, fL, i);
        }
        fL.deallocate();
        fL = fLtmp;
      }

      //Save exteral points location and function value
      DenseMatrix<Precision> E(fL.M(), extrema.N());
      DenseVector<Precision> Ef(extrema.N());
      for(unsigned int i=0; i<extrema.N(); i++){
        Linalg<Precision>::SetColumn(E, i, fL, edges.N()*nSamples+i);
        Ef(i) = yall(extrema(i));
      } 



      //ALign extrema to previous etxrema
      if(extremaIndex.N() != 0){
        fit(E, extremaPosPCA);
      }
      else{
        extremaPosPCA = Linalg<Precision>::Copy(E);        
        
        DenseVector<Precision> Lmin = Linalg<Precision>::Min(E);
        DenseVector<Precision> Lmax = Linalg<Precision>::Max(E);
        LinalgIO<Precision>::writeVector("PCAMin.data", Lmin);
        LinalgIO<Precision>::writeVector("PCAMax.data", Lmax);
        Lmin.deallocate();
        Lmax.deallocate();

      }





      //Save layout for each cell - stretch to extremal points
      for(unsigned int i =0; i<edges.N(); i++){
        DenseMatrix<Precision> tmp(fL.M(), nSamples);
        DenseVector<Precision> a(fL.M());
        DenseVector<Precision> b(fL.M());
        Precision ymin = yall(extrema(edges(0, i)));
        Precision ymax = yall(extrema(edges(1, i))); 
        if(ymin < ymax){
          Linalg<Precision>::ExtractColumn(E, edges(0, i), a );
          Linalg<Precision>::ExtractColumn(E, edges(1, i), b );
        }
        else{
          std::swap(ymin, ymax);
          Linalg<Precision>::ExtractColumn(E, edges(0, i), b );
          Linalg<Precision>::ExtractColumn(E, edges(1, i), a );
        }

        for(unsigned int j=0; j<tmp.N(); j++){
          Linalg<Precision>::SetColumn(tmp, j, fL, nSamples*i+j);
        }

        Linalg<Precision>::Subtract(a, tmp, 0, a);
        Linalg<Precision>::AddColumnwise(tmp, a, tmp);
        Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

        DenseVector<Precision> stretch(fL.M());
        for(unsigned int j=0; j<tmp.N(); j++){
          Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
          Linalg<Precision>::Add(tmp, j, stretch);
        }

        std::stringstream ss;
        ss <<"ps_" << nP << "_cell_" << i << "_layout.data";
        LinalgIO<Precision>::writeMatrix(ss.str(), tmp);

        a.deallocate();
        b.deallocate();
        tmp.deallocate();
        stretch.deallocate();
      }



      std::stringstream extL;
      extL << "ExtremaLayout_" << nP << ".data";
      LinalgIO<Precision>::writeMatrix(extL.str(), E);
      E.deallocate();

      std::stringstream extf;
      extf << "ExtremaValues_" << nP <<".data";
      LinalgIO<Precision>::writeVector(extf.str(), Ef);
      Ef.deallocate();

      pca.cleanup();
      fL.deallocate();










     //----- PCA extrema / PCA curves layout 


      DenseMatrix<Precision> Xext(Xall.M(), extrema.N());
      for(unsigned int i=0;i<extrema.N(); i++){
        Linalg<Precision>::SetColumn(Xext, i, S, nSamples*edges.N()+i);
      }
      int ndim = 2;
      if(Xext.N() == 2){
        ndim = 1;
      }
      PCA<Precision> pca2(Xext, ndim);
      DenseMatrix<Precision> pca2L = pca2.project(Xext);
      if(ndim == 1){
        DenseMatrix<Precision> tmp(2, 2);
        tmp(0, 0) = pca2L(0, 0);
        tmp(0, 1) = pca2L(0, 1);
        tmp(1, 0) = 0;
        tmp(1, 1) = 0;
        pca2L.deallocate();
        pca2L = tmp;
      }

      //Align extrema to previous etxrema
      if(extremaIndex.N() != 0){
        fit(pca2L, extremaPosPCA2);
      }
      else{
        extremaPosPCA2 = Linalg<Precision>::Copy(pca2L);       
       
        DenseVector<Precision> Lmin = Linalg<Precision>::Min(pca2L);
        DenseVector<Precision> Lmax = Linalg<Precision>::Max(pca2L);
        LinalgIO<Precision>::writeVector("PCA2Min.data", Lmin);
        LinalgIO<Precision>::writeVector("PCA2Max.data", Lmax);
        Lmin.deallocate();
        Lmax.deallocate();
      }


      //Save layout for each crystal - stretch to extremal points
      for(unsigned int i =0; i<edges.N(); i++){ 
        //do pca for each crystal to preserve strcuture of curve in crystal
        PCA<Precision> pca(Scell[i], dim);        
        DenseMatrix<Precision> tmp = pca.project(Scell[i]);
        DenseVector<Precision> a(pca2L.M());
        DenseVector<Precision> b(pca2L.M());
        int index1 = edges(0, i);
        int index2 = edges(1, i);

        Precision ymin = yall(extrema(index1));
        Precision ymax = yall(extrema(index2)); 
        if(ymin < ymax){
          Linalg<Precision>::ExtractColumn(pca2L, index1, a );
          Linalg<Precision>::ExtractColumn(pca2L, index2, b );
        }
        else{
          std::swap(ymin, ymax);
          Linalg<Precision>::ExtractColumn(pca2L, index1, b );
          Linalg<Precision>::ExtractColumn(pca2L, index2, a );
        }

        Linalg<Precision>::Subtract(a, tmp, 0, a);
        Linalg<Precision>::AddColumnwise(tmp, a, tmp);
        Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

        DenseVector<Precision> stretch(pca2L.M());
        for(unsigned int j=0; j<tmp.N(); j++){
          Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
          Linalg<Precision>::Add(tmp, j, stretch);
        }


        std::stringstream ss;
        ss <<"ps_" << nP << "_cell_" << i << "_pca2layout.data";
        LinalgIO<Precision>::writeMatrix(ss.str(), tmp);
        
        a.deallocate();
        b.deallocate();
        tmp.deallocate();
        stretch.deallocate();
        pca.cleanup();
      }


      //Save extremal points location and function value
      std::stringstream pca2extL;
      pca2extL << "PCA2ExtremaLayout_" << nP <<".data";
      LinalgIO<Precision>::writeMatrix(pca2extL.str(), pca2L);


      pca2L.deallocate();
      pca2.cleanup();



      

      
     //----- Isomap extrema / PCA curves layout 


      //Do an isomap layout
      SparseMatrix<Precision> adj(extrema.N(), extrema.N(), std::numeric_limits<Precision>::max());
      for(unsigned int i=0; i<edges.N(); i++){
        Precision dist = 0; 
        for(int j=1; j< nSamples; j++){
          int index1 = nSamples*i+j;
          int index2 = index1 - 1;
          dist += l2.distance(S, index1, S, index2);
        }       

        int index1 = edges(0, i);
        int index2 = edges(1, i);
        adj.set(index1, index2, dist);
        adj.set(index2, index1, dist);
      }





      KNNNeighborhood<Precision> nh(10);
      Isomap<Precision> isomap(&nh, dim);
      DenseMatrix<Precision> isoL = isomap.embedAdj(adj);



      //Align extrema to previous etxrema
      if(extremaIndex.N() != 0){
        fit(isoL, extremaPosIso);
      }
      else{
        extremaPosIso = Linalg<Precision>::Copy(isoL);        
        
        DenseVector<Precision> Lmin = Linalg<Precision>::Min(isoL);
        DenseVector<Precision> Lmax = Linalg<Precision>::Max(isoL);
        LinalgIO<Precision>::writeVector("IsoMin.data", Lmin);
        LinalgIO<Precision>::writeVector("IsoMax.data", Lmax);
        Lmin.deallocate();
        Lmax.deallocate();

      }


      //Save layout for each crystal - stretch to extremal points
      for(unsigned int i =0; i<edges.N(); i++){ 
        //do pca for each crystal to preserve strcuture of curve in crystal
        PCA<Precision> pca(Scell[i], dim);        
        DenseMatrix<Precision> tmp = pca.project(Scell[i]);
        DenseVector<Precision> a(isoL.M());
        DenseVector<Precision> b(isoL.M());
        int index1 = edges(0, i);
        int index2 = edges(1, i);

        Precision ymin = yall(extrema(index1));
        Precision ymax = yall(extrema(index2)); 
        if(ymin < ymax){
          Linalg<Precision>::ExtractColumn(isoL, index1, a );
          Linalg<Precision>::ExtractColumn(isoL, index2, b );
        }
        else{
          std::swap(ymin, ymax);
          Linalg<Precision>::ExtractColumn(isoL, index1, b );
          Linalg<Precision>::ExtractColumn(isoL, index2, a );
        }

        Linalg<Precision>::Subtract(a, tmp, 0, a);
        Linalg<Precision>::AddColumnwise(tmp, a, tmp);
        Linalg<Precision>::Subtract(b, tmp, tmp.N()-1, b);

        DenseVector<Precision> stretch(isoL.M());
        for(unsigned int j=0; j<tmp.N(); j++){
          Linalg<Precision>::Scale(b, j/(tmp.N()-1.f), stretch);
          Linalg<Precision>::Add(tmp, j, stretch);
        }


        std::stringstream ss;
        ss <<"ps_" << nP << "_cell_" << i << "_isolayout.data";
        LinalgIO<Precision>::writeMatrix(ss.str(), tmp);
        
        a.deallocate();
        b.deallocate();
        tmp.deallocate();
        stretch.deallocate();
        pca.cleanup();
      }


      //Save extremal points location and function value
      std::stringstream isoextL;
      isoextL << "IsoExtremaLayout_" << nP <<".data";
      LinalgIO<Precision>::writeMatrix(isoextL.str(), isoL);

      //save mins and maxs 
      if(extremaIndex.N() == 0){
        extremaIndex = DenseVector<unsigned int>(extrema.N());
        for(unsigned int i=0; i<extrema.N(); i++){
          extremaIndex(i) = i;
        }
        translationIndex = 0;
        for(unsigned int i=0; i<extrema.N(); i++){
          if(persistance(translationIndex) < persistance(i)){
            translationIndex = i;
          }
        };
      }

      isoL.deallocate();













      S.deallocate();
      for(unsigned int i=0; i<edges.N(); i++){ 
        Scell[i].deallocate();
        Xcell[i].deallocate();
        Xpcell[i].deallocate();
        ycell[i].deallocate();
      }










      //next persistsance level
      simplify(pSorted(nP));
    }


  }
  catch(const char *err){
    std::cerr << err << std::endl;
  }
  
  return 0;

}
