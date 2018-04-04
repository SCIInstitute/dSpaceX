#ifndef KERNELREGRESSION2_H
#define KERNELREGRESSION2_H

#include <math.h>
#include "Geometry.h"
#include "SquaredEuclideanMetric.h"
#include "DenseMatrix.h"
#include "DenseVector.h"

#define KERNEL_CUTOFF 0.000000001
#define PI 3.141592653589793238462643383279502884197169399375

template<typename TPrecision>
class KernelRegression2{

  public:


    KernelRegression2( DenseMatrix<TPrecision> &data, DenseMatrix<TPrecision> &values, 
                       TPrecision sigmaValue): X(data), y(values), sigma(sigmaValue)
    {
      sigmaSqr = 2*sigma*sigma;
    };


    void evaluate(Vector<TPrecision> &yValue, Vector<TPrecision> &out,
        Vector<TPrecision> &distances = empty){
     
      for(int i=0; i < X.M(); i++){
        out[i] = 0;
      } 

      TPrecision wsum = 0;
      TPrecision w = 0;
      TPrecision dist = 0;
      for(int i=0; i < y.N(); i++){
        dist = metric.distance(y, i, yValue);
        w = exp(-dist / sigmaSqr );
        if(distances.N() != 0){
          distances[i] = dist;
        }
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


    DenseVector<TPrecision> project(Vector<TPrecision> &dataPoint, int knn, TPrecision psigma){
      DenseVector<TPrecision> projected(y.M());
      project(dataPoint, knn, projected, psigma);  
      return projected; 
    };

    void project(Vector<TPrecision> &dataPoint, int knn,
        Vector<TPrecision> &projected, TPrecision psigma){
        
        DenseVector<int> nn(knn);
        DenseVector<TPrecision> dists(knn);
        
        Geometry<TPrecision>::computeKNN(X, dataPoint, nn, dists, metric);

        for(int i=0; i<y.M(); i++){
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

  private:
    DenseMatrix<TPrecision> X;
    DenseMatrix<TPrecision> y;
    
    TPrecision sigma;
    TPrecision sigmaSqr;

    SquaredEuclideanMetric<TPrecision> metric;

    static DenseVector<TPrecision> empty;
};

template <typename TPrecision>
DenseVector<TPrecision> KernelRegression2<TPrecision>::empty;

#endif
