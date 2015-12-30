#ifndef MAHALANOBISMETRIC_H
#define MAHALANOBISMETRIC_H

#include "Metric.h"
#include "Linalg.h"


template<typename TPrecision>
class MahalanobisMetric : public Metric<TPrecision>{
  public:

    // d(x, y) = x' * W^2 * x
    //Expects the square root of the usual weight matrix
    //Caller is responsible for deallocationg W after use.
    MahalanobisMetric(DenseMatrix<TPrecision> &W){
      P = W;
      tmp1 = DenseVector<TPrecision>(P.M());
      tmp2 = DenseVector<TPrecision>(P.N());

    };

    ~MahalanobisMetric(){
      tmp1.deallocate();
      tmp2.deallocate();
    }

    TPrecision distance(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
      Linalg<TPrecision>::Subtract(x1, x2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };

    TPrecision distance(Matrix<TPrecision> &X, int i1, Matrix<TPrecision> &Y, int i2){
      Linalg<TPrecision>::Subtract(X, i1, Y, i2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };
    
    TPrecision distance(Matrix<TPrecision> &X, int i1, Vector<TPrecision> &x2){
      Linalg<TPrecision>::Subtract(X, i1, x2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };



  private:
    DenseVector<TPrecision> tmp1; 
    DenseVector<TPrecision> tmp2; 
    DenseMatrix<TPrecision> P;

};
  

#endif
