#ifndef WASSERSTEIN_H
#define WASSERSTEIN_H

#include "SVD.h"
#include "SquaredEuclideanMetric.h"

#include <cmath>

template <typename TPrecision>
class Wasserstein{

  private:
    SquaredEuclideanMetric<TPrecision> metric;
  
  public:
 
    Wasserstein(){}; 
    ~Wasserstein(){};

    TPrecision distance(FortranLinalg::DenseMatrix<TPrecision> U1,
        FortranLinalg::DenseVector<TPrecision> S1,
        FortranLinalg::DenseVector<TPrecision> C1,
        FortranLinalg::DenseMatrix<TPrecision> U2,
        FortranLinalg::DenseVector<TPrecision> S2,
        FortranLinalg::DenseVector<TPrecision> C2){ 
      TPrecision md = metric.distance(C1, C2);
      TPrecision cd = covarianceDistSquared(U1, S1, U2, S2);
      return sqrt(md + cd);
    
    };
   

    //U eigenvectors, S variances of covariance matrices
    TPrecision covarianceDistSquared(FortranLinalg::DenseMatrix<TPrecision> U1,
        FortranLinalg::DenseVector<TPrecision> S1,
        FortranLinalg::DenseMatrix<TPrecision> U2,
        FortranLinalg::DenseVector<TPrecision> S2){
        using namespace FortranLinalg;
      TPrecision ts1 = Linalg<TPrecision>::Sum(S1);
      TPrecision ts2 = Linalg<TPrecision>::Sum(S2);

      //std::cout << U1.M() << " x " << U1.N() << std::endl;
      //std::cout << U2.M() << " x " << U2.N() << std::endl;
      TPrecision tc = 0;
      if(U1.N() == 0 || U2.N() == 0 || U1.M() == 0 || U2.M() == 0){
        //nothing todo tc = 0
      }
      else{
        DenseMatrix<TPrecision> U1tU2 = Linalg<TPrecision>::Multiply(U1, U2,
            true);
        DenseMatrix<TPrecision> U2tU1 = Linalg<TPrecision>::Transpose(U1tU2);
        for(int i=0; i< S2.N(); i++){
          Linalg<TPrecision>::ScaleColumn(U1tU2, i, S2(i));
        }

        //std::cout << U1tU2.M() << " x " << U1tU2.N() << std::endl;
        //std::cout << U2tU1.M() << " x " << U2tU1.N() << std::endl << std::endl;

        DenseMatrix<TPrecision> A = Linalg<TPrecision>::Multiply(U1tU2, U2tU1);
        for(int i=0; i< S1.N(); i++){
          TPrecision s= sqrt( S1(i) );
          Linalg<TPrecision>::ScaleColumn(A, i, s);
          Linalg<TPrecision>::ScaleRow(A, i, s);
        }

        SVD<TPrecision> svd(A);

        for(int i=0; i<svd.S.N(); i++){
          tc += sqrt(svd.S(i));
        }
        
        
        svd.deallocate();
        A.deallocate();
        U1tU2.deallocate();
        U2tU1.deallocate();
      }


      return ts1 + ts2 - 2*tc;
        
    };








    TPrecision covarianceDistSquared(FortranLinalg::DenseMatrix<TPrecision> Cov1,
        FortranLinalg::DenseMatrix<TPrecision> Cov2){ 
      using namespace FortranLinalg;
      
      SVD<TPrecision> svd1(Cov1);
      SVD<TPrecision> svd2(Cov2);

      if( svd1.U.M() < svd2.U.M() ){
        DenseMatrix<TPrecision> U = Linalg<TPrecision>::ExpandRows(svd1.U, svd2.U.M());
        svd1.U.deallocate(); 
        svd1.U = U;
      }
      else if( svd2.U.M() < svd1.U.M() ){
        DenseMatrix<TPrecision> U = Linalg<TPrecision>::ExpandRows(svd2.U, svd1.U.M());
        svd2.U.deallocate(); 
        svd2.U = U;
      }
  
      TPrecision d = distance2(svd1.U, svd1.S, svd2.U, svd2.S);

      svd1.deallocate();
      svd2.deallocate();
     
      return d;
    };







    //U eigenvectors, S variances of covariance matrices
    FortranLinalg::DenseMatrix<TPrecision> covarianceMap(FortranLinalg::DenseMatrix<TPrecision> U1,
        FortranLinalg::DenseVector<TPrecision> S1,
        FortranLinalg::DenseMatrix<TPrecision> U2,
        FortranLinalg::DenseVector<TPrecision> S2){
      
      using namespace FortranLinalg;
      TPrecision ts1 = Linalg<TPrecision>::Sum(S1);
      TPrecision ts2 = Linalg<TPrecision>::Sum(S2);

      //std::cout << U1.M() << " x " << U1.N() << std::endl;
      //std::cout << U2.M() << " x " << U2.N() << std::endl;
      TPrecision tc = 0;
      if(U1.N() == 0 || U2.N() == 0 || U1.M() == 0 || U2.M() == 0){
        //use identity map
        DenseMatrix<TPrecision> T(S2.N(), S1.N());
        Linalg<TPrecision>::Zero(T);
        for(int i=0; i < std::min(T.N(), T.M()); i++){
          T(i,i) = 1;
        }
        return T;
      }
      else{
        DenseMatrix<TPrecision> U1tU2 = Linalg<TPrecision>::Multiply(U1, U2,
            true);
        DenseMatrix<TPrecision> U2tU1 = Linalg<TPrecision>::Transpose(U1tU2);
        for(int i=0; i< S2.N(); i++){
          Linalg<TPrecision>::ScaleColumn(U1tU2, i, S2(i));
        }

        //std::cout << U1tU2.M() << " x " << U1tU2.N() << std::endl;
        //std::cout << U2tU1.M() << " x " << U2tU1.N() << std::endl << std::endl;

        DenseMatrix<TPrecision> A = Linalg<TPrecision>::Multiply(U1tU2, U2tU1);
        for(int i=0; i< S1.N(); i++){
          TPrecision s= sqrt(S1(i));
          Linalg<TPrecision>::ScaleColumn(A, i, s);
          Linalg<TPrecision>::ScaleRow(A, i, s);
        }


        DenseMatrix<TPrecision> U1t = Linalg<TPrecision>::Transpose(U1);
        for(int i=0; i< S1.N(); i++){
          Linalg<TPrecision>::ScaleRow(U1t, i, 1.0/sqrt(S1(i)));
        }
        DenseMatrix<TPrecision> C1 = Linalg<TPrecision>::multiply(U1, U1t);
        DenseMatrix<TPrecision> T1 = Linalg<TPrecision>::Multiply(C1, A);
        DenseMatrix<TPrecision> T = Linalg<TPrecision>::Multiply(T1, C1);

        A.deallocate();
        T1.deallocate();
        C1.deallocate();
        U1t.deallocate();
        U1tU2.deallocate();
        U2tU1.deallocate();
        return T;
      
      }


        
    };



    
};
#endif
