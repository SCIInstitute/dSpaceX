#ifndef RANDOMSVD_H
#define RANDOMSVD_H

#include "SVD.h"
#include "RandomRange.h"



namespace FortranLinalg{

  
  
template <typename TPrecision>
class RandomSVD{

  public:
    DenseMatrix<TPrecision> U;
    DenseVector<TPrecision> S;
    DenseVector<TPrecision> c;


    RandomSVD(DenseMatrix<TPrecision> Xin, int d, int nPowerIt = 0, bool
        center=false){

      DenseMatrix<TPrecision> X = Xin;
      if(center){
        X = Linalg<TPrecision>::Copy(Xin);
        c = Linalg<TPrecision>::SumColumns(X);
        Linalg<TPrecision>::Scale(c, 1.0/X.N(), c);
        Linalg<TPrecision>::SubtractColumnwise(X, c, X);
      }


      static Random<TPrecision> rand;
      
      DenseMatrix<TPrecision> Q =
        RandomRange<TPrecision>::FindRange(X,d,nPowerIt);

      DenseMatrix<TPrecision> B = Linalg<TPrecision>::Multiply(Q, X, true);

      SVD<TPrecision> svd(B, false);
      S = svd.S;
      U = Linalg<TPrecision>::Multiply(Q, svd.U);

      svd.U.deallocate();
      svd.Vt.deallocate();
      Q.deallocate();
      B.deallocate();
      if(center){
        X.deallocate();
      }

    };



    void deallocate(){
      U.deallocate();
      S.deallocate();
      c.deallocate();
    };


};

}

#endif 
