#ifndef MAHALANOBISMETRIC2_H
#define MAHALANOBISMETRIC2_H

#include "Metric.h"
#include "Linalg.h"


template<typename TPrecision>
class MahalanobisMetric2 : public Metric<TPrecision>{
  public:

    // d(x, y) = x' * W * x
    //Caller is responsible for deallocationg W after use.
    MahalanobisMetric2(DenseMatrix<TPrecision> &W){
      P = W;
      tmp1 = DenseVector<TPrecision>(P.N());
      tmp2 = DenseVector<TPrecision>(P.N());

    };

    ~MahalanobisMetric2(){
      tmp1.deallocate();
      tmp2.deallocate();
    }

    TPrecision distance(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
      Linalg<TPrecision>::Subtract(x1, x2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };

    TPrecision distance(Matrix<TPrecision> &X, int i1, Matrix<TPrecision> &Y, int i2){
      Linalg<TPrecision>::Subtract(X, i1, Y, i2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };
    
    TPrecision distance(Matrix<TPrecision> &X, int i1, Vector<TPrecision> &x2){
      Linalg<TPrecision>::Subtract(X, i1, x2, tmp1); 
      Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };



  private:
    DenseVector<TPrecision> tmp1; 
    DenseVector<TPrecision> tmp2; 
    DenseMatrix<TPrecision> P;

};
  

#endif
