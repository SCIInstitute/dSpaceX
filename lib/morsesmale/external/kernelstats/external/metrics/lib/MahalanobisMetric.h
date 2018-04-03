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
    MahalanobisMetric(FortranLinalg::DenseMatrix<TPrecision> &W){
      P = W;
      tmp1 = FortranLinalg::DenseVector<TPrecision>(P.M());
      tmp2 = FortranLinalg::DenseVector<TPrecision>(P.N());

    };

    ~MahalanobisMetric(){
      tmp1.deallocate();
      tmp2.deallocate();
    }

    TPrecision distance(FortranLinalg::Vector<TPrecision> &x1, FortranLinalg::Vector<TPrecision> &x2){
      FortranLinalg::Linalg<TPrecision>::Subtract(x1, x2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };

    TPrecision distance(FortranLinalg::Matrix<TPrecision> &X, int i1, FortranLinalg::Matrix<TPrecision> &Y, int i2){
      FortranLinalg::Linalg<TPrecision>::Subtract(X, i1, Y, i2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };
    
    TPrecision distance(FortranLinalg::Matrix<TPrecision> &X, int i1, FortranLinalg::Vector<TPrecision> &x2){
      FortranLinalg::Linalg<TPrecision>::Subtract(X, i1, x2, tmp1); 
      FortranLinalg::Linalg<TPrecision>::Multiply(P, tmp1, tmp2, true);
      return sqrt( FortranLinalg::Linalg<TPrecision>::Dot(tmp2, tmp2) ); 
    };



  private:
    FortranLinalg::DenseVector<TPrecision> tmp1; 
    FortranLinalg::DenseVector<TPrecision> tmp2; 
    FortranLinalg::DenseMatrix<TPrecision> P;

};
  

#endif
