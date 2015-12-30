#ifndef MAHALANOBISMETRIC2_H
#define MAHALANOBISMETRIC2_H

#include "Metric.h"
#include "Linalg.h"


template<typename TPrecision>
class MahalanobisMetric2 : public Metric<TPrecision>{
  public:

    // d(x, y) = x' * W * x
    //Caller is responsible for deallocationg W after use.
    MahalanobisMetric2(FortranLinalg::DenseMatrix<TPrecision> &W){
      P = W;
      tmp1 = FortranLinalg::DenseVector<TPrecision>(P.N());
      tmp2 = FortranLinalg::DenseVector<TPrecision>(P.N());

    };

    ~MahalanobisMetric2(){
      tmp1.deallocate();
      tmp2.deallocate();
    }

    TPrecision distance(FortranLinalg::Vector<TPrecision> &x1, FortranLinalg::Vector<TPrecision> &x2){
      FortranLinalg::Linalg<TPrecision>::Subtract(x1, x2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };

    TPrecision distance(FortranLinalg::Matrix<TPrecision> &X, int i1, FortranLinalg::Matrix<TPrecision> &Y, int i2){
      FortranLinalg::Linalg<TPrecision>::Subtract(X, i1, Y, i2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };
    
    TPrecision distance(FortranLinalg::Matrix<TPrecision> &X, int i1, FortranLinalg::Vector<TPrecision> &x2){
      FortranLinalg::Linalg<TPrecision>::Subtract(X, i1, x2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp1, tmp2) ); 
    };



  private:
    FortranLinalg::DenseVector<TPrecision> tmp1; 
    FortranLinalg::DenseVector<TPrecision> tmp2; 
    FortranLinalg::DenseMatrix<TPrecision> P;

};
  

#endif
