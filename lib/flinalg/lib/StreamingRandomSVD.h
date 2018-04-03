#ifndef STREAMINGRANDOMSVD_H
#define STREAMINGRANDOMSVD_H

#include "SVD.h"
#include "StreamingRandomRange.h"

template <typename TPrecision>
class StreamingRandomSVD{

  private:
    StreamingRandomRange<TPrecision> range;

  public:
    DenseMatrix<TPrecision> U;
    DenseVector<TPrecision> S;
   

    StreamingRandomSVD(int d, int nPowerIt = 0) : range(d, nPowerIt) {};


    void AddVector(DenseVector<TPrecision> &x ){
      range.AddVector(x);
    }


    void SVD(){

      DenseMatrix<TPrecision> Q = range.FindRange();

      DenseMatrix<TPrecision> B = Linalg<TPrecision>::Multiply(Q, X, true);

      SVD<TPrecision> svd(B, false);
      S.deallocate();
      U.deallocate();

      S = svd.S;
      U = Linalg<TPrecision>::Multiply(Q, svd.U);

      svd.U.deallocate();
      svd.Vt.deallocate();
      Q.deallocate();
      B.deallocate();
    };



    void deallocate(){
      U.deallocate();
      S.deallocate();
      range.deallcoate();
    };


};


#endif 
