#ifndef KERNELREGRESSION_H
#define KERNELREGRESSION_H

#include <math.h>

#include "SquaredEuclideanMetric.h"
#include "Geometry.h"
#include "GaussianKernel.h"
#include "DenseVector.h"
#include "DenseMatrix.h"

#define KERNEL_CUTOFF 0
//0.000000001

template<typename TPrecision>
class KernelRegression{
      
  public:
    KernelRegression(DenseMatrix<TPrecision> &data, DenseMatrix<TPrecision>
        &labels, GaussianKernel<TPrecision> &k):X(data), y(labels), kernel(k){
      dCut = kernel.getKernelParam() * 4;
      dCut = dCut*dCut;
      tmp = DenseVector<TPrecision>(X.M()); 
      w = DenseVector<TPrecision>(X.N()); 
    };





    TPrecision evaluate( DenseVector<TPrecision> &yValue, DenseVector<TPrecision> &out, 
                         DenseVector<TPrecision> &sdev ){
      Linalg<TPrecision>::Zero(out);

      TPrecision sd = 0;
      TPrecision d = 0;
      TPrecision wsum = 0;
      for(unsigned int i=0; i < X.N(); i++){
          d = metric.distance(y,i, yValue);
          if( d>dCut){
            w(i) = 0;
            continue;
          }
          
          w(i) = kernel.f(d);
          wsum += w(i);
          Linalg<TPrecision>::AddScale(out, w(i), X, i, out);
      }

      if(wsum == 0){
        return 0;
      }
      TPrecision s = 1.0/wsum;
      Linalg<TPrecision>::Scale(out, s, out);
    
      //compute coordinatewise mean projection distance
        Linalg<TPrecision>::Zero(sdev);
        for(int i=0; i<X.N(); i++){
          if(w(i) == 0){ continue; }
          Linalg<TPrecision>::Subtract(X,i, out, tmp);
          for(int j=0; j<tmp.N(); j++){
            tmp(j) *= tmp(j);
          }
          sd += w(i) * sqrt(Linalg<TPrecision>::Sum(tmp));
          Linalg<TPrecision>::Sqrt(tmp, tmp);
          Linalg<TPrecision>::AddScale(sdev, w(i), tmp, sdev);
        }
        Linalg<TPrecision>::Scale(sdev, s, sdev);

      return sd*s;
    };

    
    
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
          projected(i) = 0;
        }
 
        
        TPrecision wsum = 0;
        TPrecision w = 0;

        for(int i=0; i < knn; i++){
          w = exp(-dists[i] / (psigma*psigma) );
          wsum +=w;
          for(int j = 0; j < y.M(); j++){
            projected(j) += w*y(j, nn(i));
          }
        }
        
        for(int i = 0; i < y.M(); i++){
           projected(i) /= wsum; 
        }

    };



  private:
    SquaredEuclideanMetric<Precision> metric;
 
    DenseMatrix<TPrecision> X;
    DenseMatrix<TPrecision> y;
    DenseVector<TPrecision> tmp;
    DenseVector<TPrecision> w;

    GaussianKernel<TPrecision> &kernel;

    TPrecision dCut;
};


#endif
