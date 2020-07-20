#ifndef FIRSTORDERKERNELREGRESSION_H
#define FIRSTORDERKERNELREGRESSION_H

#include "flinalg/DenseVector.h"
#include "flinalg/DenseMatrix.h"
#include "flinalg/Linalg.h"
#include "GaussianKernel.h"
#include "metrics/Distance.h"
#include "metrics/SquaredEuclideanMetric.h"

#include <math.h>


template<typename TPrecision>
class FirstOrderKernelRegression {      
  public:
    FirstOrderKernelRegression(FortranLinalg::DenseMatrix<TPrecision> &data, FortranLinalg::DenseMatrix<TPrecision>
        &labels, GaussianKernel<TPrecision> &k, int knn):Y(data), X(labels), kernel(k) {
	    if (knn > X.N()){ 
        knn = X.N(); 
      }
	    A = FortranLinalg::DenseMatrix<TPrecision>(knn, 1+X.M());
     	b = FortranLinalg::DenseMatrix<TPrecision>(knn, Y.M());
    };


    void cleanup(){
      A.deallocate();
      b.deallocate();
    };     


     void project( FortranLinalg::DenseVector<TPrecision> &x, FortranLinalg::DenseVector<TPrecision> &y, 
                   FortranLinalg::DenseVector<TPrecision> &out){
       FortranLinalg::DenseMatrix<TPrecision> sol = ls(x);
       for(unsigned int i=0; i<Y.M(); i++){
           out(i) = sol(0, i);
       }
       for(unsigned int j=0; j< X.M(); j++){
         TPrecision dprod = 0;
         for(unsigned int i=0; i<Y.M(); i++){
           dprod +=  (y(i)-sol(0, i)) * sol(j+1, i);
         }
         TPrecision length = FortranLinalg::Linalg<TPrecision>::LengthRow(sol, j+1); 
         dprod /= (length * length);
         for(unsigned int i=0; i<Y.M(); i++){
           out(i) += dprod * sol(j+1, i);
         }
       }
       sol.deallocate();
      
     };

      void evaluate( FortranLinalg::DenseVector<TPrecision> &x, FortranLinalg::Vector<TPrecision> &out,
FortranLinalg::Matrix<TPrecision> &J, TPrecision *sse=NULL){
        FortranLinalg::DenseMatrix<TPrecision> sol = ls(x, sse);
        for(unsigned int i=0; i<Y.M(); i++){
          out(i) = sol(0, i);
        }     
        for(unsigned int i=0; i< J.N(); i++){
          for(unsigned int j=0; j< J.M(); j++){
            J(j, i) = sol(1+i, j);
          }
        }
        sol.deallocate();
      };


  private:
    SquaredEuclideanMetric<TPrecision> sl2metric;
 
    FortranLinalg::DenseMatrix<TPrecision> Y;
    FortranLinalg::DenseMatrix<TPrecision> X;

    GaussianKernel<TPrecision> &kernel;

    FortranLinalg::DenseMatrix<TPrecision> A;
    FortranLinalg::DenseMatrix<TPrecision> b;

    FortranLinalg::DenseMatrix<TPrecision> ls(FortranLinalg::DenseVector<TPrecision> &x, 
        TPrecision *sse=NULL) {
      FortranLinalg::DenseVector<int> knn(A.M());
      FortranLinalg::DenseVector<TPrecision> knnDist(A.M());
      Distance<TPrecision>::computeKNN(X, x, knn, knnDist, sl2metric);

      TPrecision wsum = 0; 
      for(unsigned int i=0; i < A.M(); i++){
        unsigned int nn = knn(i);
        TPrecision w = kernel.f(knnDist(i));
        A(i, 0) = w;
        for(unsigned int j=0; j< X.M(); j++){
          A(i, j+1) = (X(j, nn)-x(j)) * w; // Essentially get knnDist again and multiplying by w
        }

        for(unsigned int m = 0; m < Y.M(); m++){
          b(i, m) = Y(m, nn) *w;
        }
        wsum += w*w;
      }

      FortranLinalg::DenseMatrix<TPrecision> sol = FortranLinalg::Linalg<TPrecision>::LeastSquares(A, b, sse);
      if(sse != NULL){ 
        for(int i=0; i<sol.N(); i++){
          sse[i] /= wsum;
        }
      }

      knn.deallocate();
      knnDist.deallocate();
      return sol;
    };
};


#endif

