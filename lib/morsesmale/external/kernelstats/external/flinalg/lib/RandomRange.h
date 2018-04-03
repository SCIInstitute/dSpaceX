#ifndef RANDOMRANGE_H
#define RANDOMRANGE_H

#include "SVD.h"
#include "Random.h"


namespace FortranLinalg{

template <typename TPrecision>
class RandomRange{

  public:


    static DenseMatrix<TPrecision> FindRange(DenseMatrix<TPrecision> Xin, int
        d, int nPowerIt = 0, bool center=false){

      DenseMatrix<TPrecision> X = Xin;
      if(center){
        X = Linalg<TPrecision>::Copy(Xin);
        DenseVector<TPrecision> c = Linalg<TPrecision>::SumColumns(X);
        Linalg<TPrecision>::Scale(c, 1.0/X.N(), c);
        Linalg<TPrecision>::SubtractColumnwise(X, c, X);
        c.deallocate();
      }


      static Random<TPrecision> rand;
      
      DenseMatrix<TPrecision> N(X.N(), d);
      for(unsigned int i=0; i< N.M(); i++){
        for(unsigned int j=0; j< N.N(); j++){
          N(i, j) = rand.Normal();
        }
      }

      
      DenseMatrix<TPrecision> Q = Linalg<TPrecision>::Multiply(X, N);
      Linalg<TPrecision>::QR_inplace(Q);


      if(nPowerIt > 0){
        DenseMatrix<TPrecision> Z( X.N(), Q.N() );
        for( int i=0; i<nPowerIt; i++){
          Linalg<TPrecision>::Multiply(X, Q, Z, true);
          Linalg<TPrecision>::QR_inplace(Z);

          Linalg<TPrecision>::Multiply(X, Z, Q);
          Linalg<TPrecision>::QR_inplace(Q);
        }
        Z.deallocate();
      }
      

      N.deallocate();
      if(center){
        X.deallocate();
      }

      return Q;

    };

};

}

#endif 
