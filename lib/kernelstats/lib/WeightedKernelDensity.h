#ifndef WEIGHTEDKERNELDENSITY_H
#define WEIGHTEDKERNELDENSITY_H


#include "DenseVector.h"
#include "DenseMatrix.h"
#include "Kernel.h"
#include "Linalg.h"


template<typename TPrecision>
class WeightedKernelDensity{
      
  public:
    WeightedKernelDensity(DenseMatrix<TPrecision> &data, Kernel<TPrecision, TPrecision> &k,
        DenseVector<TPrecision> &w)
                    :X(data), kernel(k), weights(w){
    };

    //retunrs unnormalized density
    double p(int j, bool leaveout = false){
      TPrecision wsum = 0;
      for(int i=0; i < X.N(); i++){
        if(leaveout && j== i) continue;
        wsum += kernel.f(X, j, X, i) * weights(i);
      }
      return wsum;
    };

    //retunrs unnormalized density
    double p(DenseMatrix<TPrecision> &T, int index, bool leaveout = false){
      TPrecision wsum = 0;
      for(unsigned int i=0; i < X.N(); i++){
        bool use = true;
        if(leaveout){
          use = ! Linalg<Precision>::IsColumnEqual(T, index, X, i);
        }
        if(use){
          wsum += kernel.f(T, index, X, i) * weights(i);
        }
      }
      return wsum;
    };

    //retunrs unnormalized density
    double p(DenseVector<TPrecision> &x, int leaveout = -1){
      TPrecision wsum = 0;
      for(int i=0; i < X.N(); i++){
        if(leaveout != i){
          wsum += kernel.f(x, X, i) * weights(i);
        }
      }
      return wsum;
    };

    void setData(DenseMatrix<TPrecision> &data){
      X = data;
    };

  private:
    DenseMatrix<TPrecision> X;
    Kernel<TPrecision, TPrecision> &kernel;
    DenseVector<TPrecision> &weights;
};



#endif
