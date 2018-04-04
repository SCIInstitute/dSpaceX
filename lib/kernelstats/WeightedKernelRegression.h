#ifndef WEIGHTEDKERNELREGRESSION_H
#define WEIGHTEDKERNELREGRESSION_H

#include <math.h>

#include "Geometry.h"
#include "Kernel.h"
#include "DenseVector.h"
#include "DenseMatrix.h"

#define KERNEL_CUTOFF 0

template<typename TPrecision>
class WeightedKernelRegression{
      
  public:
    WeightedKernelRegression(DenseMatrix<TPrecision> &data, DenseMatrix<TPrecision> &labels,
                     Kernel<TPrecision, TPrecision> &k, DenseVector<TPrecision> &w)
                    :X(data), y(labels), kernel(k), weights(w){
    };



    void evaluate(DenseVector<TPrecision> &yValue, DenseVector<TPrecision> &out){
     
      for(int i=0; i < X.M(); i++){
        out[i] = 0;
      } 

      TPrecision wsum = 0;
      TPrecision w = 0;
      TPrecision d = 0;
      for(int i=0; i < X.N(); i++){
        w = kernel.f(yValue, y, i);
        w = w * weights(i);
        if(w > KERNEL_CUTOFF){
          wsum +=w;
          for(int j = 0; j < X.M(); j++){
            out[j] += w*X(j, i); 
          }
        }
      }

      for(int j = 0; j < X.M(); j++){
        out[j] /= wsum; 
      }
    };

   
   /* 
    
    DenseVector<TPrecision> project(Vector<TPrecision> &dataPoint, int knn, TPrecision psigma){
      DenseVector<TPrecision> projected(y.M);
      project(dataPoint, knn, projected, psigma);  
      return projected; 
    };

    
    
    void project(DenseVector<TPrecision> &dataPoint, int knn,
        DenseVector<TPrecision> &projected, TPrecision psigma){
        
        DenseVector<int> nn(knn);
        DenseVector<TPrecision> dists(knn);
        
        Geometry<TPrecision>::computeKNN(X, dataPoint, nn, dists, metric);

        for(int i=0; i<y.M; i++){
          projected[i] = 0;
        }
 
        
        TPrecision wsum = 0;
        TPrecision w = 0;

        for(int i=0; i < knn; i++){
          w = exp(-dists[i] / (psigma*psigma) );
          wsum +=w;
          for(int j = 0; j < y.M(); j++){
            projected[j] += w*y(j, nn[i]);
          }
        }
        
        for(int i = 0; i < y.M(); i++){
           projected[i] /= wsum; 
        }

    };

*/

  private:
    Kernel<TPrecision, TPrecision> &kernel;

    DenseMatrix<TPrecision> X;
    DenseMatrix<TPrecision> y;
    
    DenseVector<Precision> weights;

};


#endif
