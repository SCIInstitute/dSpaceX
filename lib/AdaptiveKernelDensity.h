#ifndef ADAPTIVEKERNELDENSITY_H
#define ADAPTIVEKERNELDENSITY_H


#include "DenseVector.h"
#include "DenseMatrix.h"
#include "Kernel.h"
#include "Linalg.h"
#include "Geometry.h"
#include "SquaredEuclideanMetric.h"

template<typename TPrecision>
class AdaptiveKernelDensity{
      
  public:
    AdaptiveKernelDensity(DenseMatrix<TPrecision> &data, int k)
                    :X(data), knn(k){
       if(knn > data.N() ){
         knn=data.N()-1;
       }
       nn = DenseVector<int>(knn+1);
       nnd = DenseVector<TPrecision>(knn+1);
    };


    //returns unnormalized density
    double p(int j, int leaveout = -1 ){

      Geometry<TPrecision>::computeKNN(X, j, nn, nnd, l2metric);
      TPrecision wsum = 0;
      int start = 0;
      TPrecision d = nnd(knn-1+start);
      for(int i=start; i < (knn+start); i++){
        wsum +=  ( 1.0 - nnd(i)/d );
      }
      return wsum * 0.75 / sqrt(d);
    };

    double p(DenseVector<TPrecision> &x){
      Geometry<TPrecision>::computeKNN(X, x, nn, nnd, l2metric);
      TPrecision wsum = 0;
      TPrecision d = nnd(knn-1);
      for(int i=0; i < knn; i++){
        wsum += ( 1.0 - nnd(i)/d );
      }
      return wsum * 0.75 / sqrt(d);
    };


    
    DenseVector<TPrecision> p(DenseMatrix<TPrecision> e){
      DenseVector<TPrecision> res(e.N());
      DenseVector<TPrecision> tmp(e.M());
      for(int i=0; i< e.N(); i++){
        Linalg<TPrecision>::ExtractColumn(e, i, tmp);
        res(i) = p(tmp);
      }
      return res;
    };



    void setData(DenseMatrix<TPrecision> &data){
      X = data;
    };

  private:
    DenseMatrix<TPrecision> X;
    int knn;
    DenseVector<int> nn;
    DenseVector<TPrecision> nnd;
    SquaredEuclideanMetric<TPrecision> l2metric;
};



#endif
