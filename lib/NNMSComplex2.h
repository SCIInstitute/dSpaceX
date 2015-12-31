//Compute persistence based on crytsal size (number of points) instaed of
//function value differences
#ifndef NNMSCOMPLEXR2_H
#define NNMSCOMPLEXR2_H

#include "Geometry.h"
#include "EuclideanMetric.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "Linalg.h"

#include <map>
#include <vector>
#include <utility>
#include <limits>
#include <list>
#include <set>

template<typename TPrecision>
class NNMSComplexR2{

  private:
    typedef std::pair<int, int> pair_i;
    
    typedef std::set<int> set_i;
    typedef set_i::iterator set_i_it;
    
    typedef std::set<pair_i> set_pi;
    typedef set_pi::iterator set_pi_it;

    typedef std::map< pair_i, int> map_pi_i;
    typedef map_pi_i::iterator map_pi_i_it;

    typedef std::map< int, set_i> map_i_si;
    typedef map_i_si::iterator map_i_si_it;

    typedef std::map< int, double> map_i_d;
    typedef map_i_d::iterator map_i_d_it;

    typedef std::map< pair_i, set_i > map_pi_si;
    typedef map_pi_si::iterator map_pi_si_it;
    

    //Steepest ascending KNNG(0,) and descending KNNG(1, ) neighbros for each point    
    FortranLinalg::DenseMatrix<int> KNNG;

    //Data points
    FortranLinalg::DenseMatrix<TPrecision> X;
    FortranLinalg::DenseVector<TPrecision> y;
    //extrema ID for ach point --- max extrema(0, ) and min extrema(1, ) 
    FortranLinalg::DenseMatrix<int> extrema;

    //map of crystals as <max, min> -> set of points in crystal 
    map_i_si crystals;
    //map of crystals to R2 based on linear fit or -1/n if number of points n
    //too small for R2 statistic
    map_i_d r2;
    
    //connections of maxima (to minima) and minima (to maxima)
    map_i_si connections;

    
    
    //extrema ID to index into X
    FortranLinalg::DenseVector<int> extremaIndex;
    
    //number of maxima, first nMax entries in extremaIndex are maxima
    int nMax;


    EuclideanMetric<TPrecision> l2;


    int ascending(int index){
      return KNNG(0, index);
    };

    int descending(int index){
      return KNNG(1, index);
    };




   double fitLM(set_i &points){
     using namespace FortranLinalg;
      //TODO decide based on power of R2 statistics
      int n = points.size();
      if(n < X.M()+10 ){
        return  - 100000.0 / n;
      }
      else{
        DenseMatrix<TPrecision> A(n, X.M() + 1);
        DenseMatrix<TPrecision> b(n, 1);
        int index = 0;
        double mean = 0;
        for(set_i_it it = points.begin(); it != points.end(); ++it, ++index){
          A(index, 0 ) = 1;
          int xindex = *it;
          for(int i=0; i<X.M(); i++){
            A(index, i+1) = X(i, xindex);
          }
          b(index, 0) = y( xindex );
          mean += y(xindex);
        }
        mean/=n;
        double sse = 0; 
        DenseMatrix<TPrecision> coef = Linalg<TPrecision>::LeastSquares(A, b, &sse);
        double sst = 0;
	for(set_i_it it = points.begin(); it != points.end(); ++it, ++index){
          int xindex = *it;
          double tmp = y(xindex) - mean;
          sst += tmp*tmp;
        }
        A.deallocate();
        b.deallocate();
        coef.deallocate();
        return (1-sse/sst);
      } 
   };



   void doMerge(pair_i m){  
      int c1 = m.first;
      int c2 = m.second;
      set_i &s1 = crystals[c1];
      set_i &s2 = crystals[c2];
      s2.insert(s1.begin(), s1.end());
      double r = fitLM(s2);
      r2[c2] = r;
      
      //merge connections
      set_i &cons1 = connections[c1];
      set_i &cons2 = connections[c2];
      cons2.insert(cons1.begin(), cons1.end());

      //remove references to c1
      r2.erase(c1);
      crystals.erase(c1);
      connections.erase(c1);
      for(map_i_si_it it = connections.begin(); it != connections.end(); ++it){
        set_i &c = it->second;
        if( c.erase(c1) == 1){
          c.insert(c2);
        }
      } 
   };


   double testMerge(pair_i m){
      int c1 = m.first;
      int c2 = m.second;
      set_i &s2 = crystals[c2];
      set_i &s1 = crystals[c1];
      set_i sa = s1;
      sa.insert(s2.begin(), s2.end());
      double r2a = fitLM(sa);
      double r21 = r2[c1];  
      double r22 = r2[c2];
      return r2a - (r21 + r22)/2.0;  
   };

   

  public:

    NNMSComplexR2(FortranLinalg::DenseMatrix<TPrecision> &Xin, FortranLinalg::DenseVector<TPrecision> &yin, int
        knn, bool smooth = false, double eps=0.1) : X(Xin), y(yin){
     using namespace FortranLinalg;
      if(knn > X.N()){
        knn = X.N();
      }
      DenseMatrix<int> KNN(knn, X.N());
      DenseMatrix<TPrecision> KNND(knn, X.N());

      //Compute nearest neighbors
      Geometry<TPrecision>::computeANN(X, KNN, KNND, eps);


      DenseVector<TPrecision> ys;
      if(smooth){
        ys = DenseVector<TPrecision>(y.N());
        for(int i=0; i< ys.N(); i++){
          ys(i) = 0;
          for(int k=0; k<knn; k++){
            ys(i) += y(KNN(k, i));
          }
          ys(i) /= knn;//*(knn+1)/2;
        }
      }
      else{
        ys = y;
      }

      KNNG = DenseMatrix<int>(2, X.N());
      Linalg<int>::Set(KNNG, -1);
      DenseMatrix<TPrecision> G = DenseMatrix<TPrecision>(2, X.N());
      Linalg<TPrecision>::Zero(G);

      //compute steepest asc/descending neighbors
      for(int i=0; i<X.N(); i++){
        for(int k=1; k<KNN.M(); k++){
          int j = KNN(k, i);
          TPrecision g = (ys(j) - ys(i)) / KNND(k, i);
          if(G(0, i) < g){
            G(0, i) = g;
            KNNG(0, i) = j;
          }
          else if(G(1, i) > g){
            G(1, i) = g;
            KNNG(1, i) = j;
          }          
          if(G(0, j) < -g){
            G(0, j) = -g;
            KNNG(0, j) = i;
          }
          else if(G(1, j) > -g){
            G(1, j) = -g;
            KNNG(1, j) = i;
          }
        }
      }
      G.deallocate();
      KNND.deallocate();
      if(smooth){
       ys.deallocate();
      }


      //compute for each point its minimum and maximum based on
      //steepest ascent/descent
      extrema = DenseMatrix<int>(2, X.N()); 
      Linalg<int>::Set(extrema, -1);

      std::list<int> extremaL;
      std::list<int> path;
      int nExt = 0;
      nMax = 0;
      std::vector<int> nPnts;
      for(int e=0; e<2; e++){
        for(int i=0; i<extrema.N(); i++){
          if(extrema(e, i) == -1){
            path.clear();
            int prev = i;
            while(prev != -1 && extrema(e, prev) == -1){
              path.push_back(prev);
              if(e==0){
                prev = ascending(prev);
              }
              else{
                prev = descending(prev);
              }
            }
            int ext = -1;
            if(prev == -1){
              int extIndex = path.back();
              extremaL.push_back(extIndex);
              ext = nExt;
              nExt++;
              if(e==0){
                nMax++;
              }
            }
            else{
              ext = extrema(e, prev);
            }
            for(std::list<int>::iterator it = path.begin(); it!=path.end(); ++it){
              extrema(e, *it) = ext;
            }   
          }
        }
      }

      extremaIndex = DenseVector<int>(nExt);
      int index = 0;
      for(std::list<int>::iterator it = extremaL.begin(); it != extremaL.end(); ++it, ++index){
        extremaIndex(index) = *it;
      }

      //Persistence based on linear fit
      //for each crystal fit linear model - if not enough points for r2
      //statistic set to 1/n with n the number of crystals in the peak. This
      //merges smallest crystals first and then based on linear fit.
      
      //create lowest persistence point assignments
      //store extrema connections min->max and max -> min
      map_pi_i crystalIndex;
      int cIndex = 0;
      for(int i=0; i<extrema.N(); i++){
        int e1 = extrema(0, i);
        int e2 = extrema(1, i);
        pair_i p(e1, e2);
        map_pi_i_it it = crystalIndex.find(p);
        int cid = 0;
        if(it == crystalIndex.end()){
	  crystalIndex[p] = cIndex;
          cid = cIndex;
          ++cIndex;
        }
        else{
          cid = it->second;
        }
        crystals[cid].insert(i);
      }

      for(map_pi_i_it it = crystalIndex.begin(); it != crystalIndex.end(); ++it){
        pair_i p = it->first;
        int cid = it->second;
        set_i &s = crystals[cid];
        s.insert(extremaIndex(p.first));
        s.insert(extremaIndex(p.second));
      }
      
      for(int i=0; i < extrema.N(); i++){
        pair_i c1( extrema(0, i), extrema(1, i) );
        for(int k=1; k < KNN.M(); k++){
          pair_i c2( extrema(0, KNN(k, i)), extrema(1, KNN(k, i)) );
          if(c1 != c2) {
            int id1 = crystalIndex[c1];
            int id2 = crystalIndex[c2];
            connections[id1].insert(id2);
            connections[id2].insert(id1);
          }
        }
      }
      
      


      //compute inital persistencies
      for(map_i_si_it it = crystals.begin(); it != crystals.end(); ++it){
        set_i points = it->second;
        double r = fitLM(points);
        r2[it->first] = r;
      }
      

     
      //check possible merges (adjacent mins and maxs) for reduction in r2
      for(;;){
        double curR2 = 0;
        for(map_i_d_it it = r2.begin(); it != r2.end(); ++it){
          curR2 += it->second;
        }
        curR2 /= r2.size();

        //find best possible merge in terms of R2
        double improve = 0;
        set_pi merged;
        pair_i bestMerge;
        for(map_i_si_it it = connections.begin(); it != connections.end(); ++it){
          set_i &adj = it->second;
	  for( set_i_it i1 = adj.begin(); i1 != adj.end(); ++i1){
            set_i_it i2 = i1;
            ++i2;
            for(; i2 != adj.end(); i2++){
              pair_i m( *i1, *i2);
              std::pair<set_pi_it, bool> exists = merged.insert(m);
              if( !exists.second ){
                double tmp = testMerge(m);
                if(tmp > improve){
                  improve = tmp;
             	  bestMerge = m;
                }
              } 
            } 
          }
        }

        //Is there an R2 reducing merge?
        if(improve > 0 ){
          doMerge(bestMerge);
        }
        else{
          break;
        }
      }
  
      KNN.deallocate();
    };




    //Get partioning accordinng to the crystals of the MS-complex for the
    //currently set persistence level
    FortranLinalg::DenseVector<int> getPartitions(){
     using namespace FortranLinalg;
      DenseVector<int> crys(X.N());
      getPartitions(crys);
      return crys;
    };




    void getPartitions(FortranLinalg::DenseVector<int> &crys){
      int crystalIndex = 0;
      for(map_i_si_it it = crystals.begin(); it != crystals.end(); ++it,  ++crystalIndex){
        set_i &points = it->second;
        for(set_i_it sit = points.begin(); sit != points.end(); ++sit){
          crys(*sit) = crystalIndex;
        }
      }
    };



    int getNCrystals(){
      return crystals.size();
    };


    int getNAllExtrema(){
      return extremaIndex.N();
    }; 

    //return extrema indicies (first row is max, secon is min) for each crystal
    FortranLinalg::DenseMatrix<int> getExtrema(){
       DenseMatrix<int> e(2, crystals.size());
       getExtrema(e);
       return e;
    };


    void getExtrema(FortranLinalg::DenseMatrix<int> ce){
      int crystalIndex = 0;
      for(map_i_si_it it = crystals.begin(); it != crystals.end(); ++it, ++crystalIndex){
        int cid = it->first;
        ce(0, crystalIndex) = cid;//extremaIndex(p.first);
        ce(1, crystalIndex) = cid;//extremaIndex(p.second);
      }
    };



    void getMax(FortranLinalg::DenseVector<int> vmaxs){
      int crystalIndex = 0;
     for(map_i_si_it it = crystals.begin(); it != crystals.end(); ++it, ++crystalIndex){
        int cid = it->first;
        vmaxs(crystalIndex) = cid;//extremaIndex(p.first);
      }
    };


    void getMin(FortranLinalg::DenseVector<int> vmins){
      int crystalIndex = 0;
      for(map_i_si_it it = crystals.begin(); it != crystals.end(); ++it, ++crystalIndex){
        int cid = it->first;
        vmins(crystalIndex) = cid; //extremaIndex(p.second);
      }
    };


    void cleanup(){
      extrema.deallocate();
      extremaIndex.deallocate();
      KNNG.deallocate();
    };


};

#endif 

