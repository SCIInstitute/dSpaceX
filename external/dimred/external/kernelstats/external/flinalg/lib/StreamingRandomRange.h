#ifndef STREAMINGRANDOMRANGE_H
#define STREAMINGRANDOMRANGE_H

#include "Random.h"

namespace FortranLinalg{

template <typename TPrecision>
class StreaminRandomRange{

  private:
    DenseMatrix<TPrecision> N;

    std::list<DenseVector<TPrecision> > q

  public:

    public StreamingRandomRange(int d, int nPowerIt = 0){
     
      static Random<TPrecision> rand;
      
      DenseMatrix<TPrecision> N(d, X.N());
      for(int i=0; i< N.M(); i++){
        for(int j=0; j< N.N(); j++){
          N(i, j) = rand.Normal();
        }
      }

    };



    void AddVector(DenseVector<TPrecision> &x){
      q.push_back( Linalg<TPrecision>::Multiply(N, x) );
    };



    DenseMatrix<TPrecision> FindRange(){
   
      DenseMatrix<TPrecision> Q(d, q.size());
      int index = 0;
      for(std::list<DenseVector<TPrecision> >::iterator it=q.begin(); it !=
          q.end(); ++it, ++index){
        Linalg<TPrecision>::SetColum(Q, index, *it);    
      } 
      Linalg<TPrecision>::QR_inplace(Q);


      if(nPowerIt > 0){
        DenseMatrix<TPrecision> Z( X.N(), Q.N() );
        for(int i=0; i<nPowerIt; i++){
          Linalg<TPrecision>::Multiply(X, Q, Z, true);
          Linalg<TPrecision>::QR_inplace(Z);

          Linalg<TPrecision>::Multiply(X, Z, Q);
          Linalg<TPrecision>::QR_inplace(Q);
        }
        Z.deallocate();
      }

      return Q;

    };


    void deallocate(){
        for(std::list<DenseVector<TPrecision> >::iterator it=q.begin(); it !=
          q.end(); ++it){
        (*it).deallocate();    
      } 
      N.deallocate();
    };


};

}

#endif 
