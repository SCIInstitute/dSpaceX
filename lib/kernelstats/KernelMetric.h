#ifndef KERNELMETRIC_H
#define KERNELMETRIC_H

#include "Geometry.h"
#include "Matrix.h"
#include "EuclideanMetric.h"
#include "SquaredEuclideanMetric.h"
#include "Linalg.h"
#include "LinalgIO.h"
#include "GaussianKernel.h"
#include "Random.h"

#include <stdlib.h>
#include <limits>

template <typename TPrecision>
class KernelMetric{

  private:
    DenseMatrix<TPrecision> Y;
    DenseMatrix<TPrecision> Z;
    DenseMatrix<TPrecision> L;
    DenseMatrix<TPrecision> fY;
    DenseMatrix<TPrecision> DfY;
    DenseMatrix<TPrecision> DL;

    unsigned int knnSigma;

    EuclideanMetric<TPrecision> l2metric;
    SquaredEuclideanMetric<TPrecision> sl2metric;

    GaussianKernel<TPrecision> kernelY;
    
    DenseMatrix<TPrecision> KY;
    DenseVector<TPrecision> sumKY;
    DenseMatrix<TPrecision> KYN;

    bool Zchanged;

    


  public:
  
   void cleanup(){      
    KY.deallocate();
    sumKY.deallocate();
    KYN.deallocate();
    Y.deallocate();
    Z.deallocate();
    fY.deallocate();
    L.deallocate();
    DL.deallocate();
    DfY.deallocate();
   };

   //Create KernelMap 
   KernelMetric(DenseMatrix<TPrecision> Ydata, DenseMatrix<TPrecision> labels, unsigned int nnSigma) :
       Y(Ydata), Z(labels), L(labels), knnSigma(nnSigma){
     init();
   };


   void init(){
     fY = DenseMatrix<TPrecision>(Z.M(), Z.N()); 
     
     DL = DenseMatrix<TPrecision>(fY.N(), fY.N());
     for(unsigned int i=0; i<fY.N(); i++){
       for(unsigned j=i; j<fY.N(); j++){
         DL(i, j) = sl2metric.distance(L, i, L, j);
         DL(j, i) = DL(i,j);
       }
     }
 
     DfY = DenseMatrix<TPrecision>(fY.N(), fY.N());
     
     DenseMatrix<int> nn(knnSigma+1, Y.N());
     DenseMatrix<TPrecision> dnn(knnSigma+1, Y.N());
     Geometry<TPrecision>::computeKNN(Y, nn, dnn, sl2metric);
     TPrecision sigma = 0;
     for(unsigned int i=0; i<Y.N(); i++){
       sigma += sqrt(dnn(knnSigma, i));
     }
     sigma /= Y.N();
     std::cout << sigma << std::endl;
     kernelY = GaussianKernel<TPrecision>(sigma, 1);//(int) Y.M());
     nn.deallocate();
     dnn.deallocate();
     

     computeKY();

     Zchanged = true;
     computefY();
   };     
   

   //evalue objective function, squared error
   TPrecision evaluate(){
     TPrecision e=0;
     for(unsigned int i=0; i<fY.N(); i++){
       for(unsigned int j=0; j<fY.N(); j++){
         TPrecision df = sl2metric.distance(fY, i, fY, j);
         TPrecision dl = sl2metric.distance(L, i, L, j);
         dl -= df;
         e += dl*dl;
       }
     } 
     return e;
   };
  


   //Gradient descent for all points 
   void gradDescent(unsigned int nIterations, TPrecision scaling){
     
     TPrecision msePrev = evaluate();
     std::cout << "MSE start: " << msePrev/Y.N() << std::endl;
     
     //---Storage for syncronous updates 
     DenseMatrix<TPrecision> sync(Z.M(), Z.N());

     //---Do nIterations of gradient descent     
     
     DenseMatrix<TPrecision> Ztmp(Z.M(), Z.N());
     DenseMatrix<TPrecision> Zswap;

     //gradient direction
     DenseVector<TPrecision> gx(Z.M());
     std::cout << "gradDescent all:" << std::endl;
     for(unsigned int i=0; i<nIterations; i++){

      //compute gradient for each point
      TPrecision maxL = 0;
      for(unsigned int j=0; j < Z.N(); j++){
        //compute gradient
        gradX(j, gx);
        
        //store gradient for syncronous updates
        TPrecision l = Linalg<TPrecision>::Length(gx);
        if(l > maxL){
          maxL = l;
        }
        for(unsigned int k=0; k<Z.M(); k++){
          sync(k, j) = -gx(k);
        }
      }

      //sync updates
      TPrecision s = scaling / maxL;
      if(s == std::numeric_limits<TPrecision>::infinity()){
        s = scaling;
      }
      std::cout << "scaling: " << s << std::endl;
      
      //evaluate();
      
      //Linalg<TPrecision>::AddScale(Z, -s, sync, Z);

      
      //Approximate line search with quadratic fit
      DenseMatrix<TPrecision> A(3, 3);
      DenseMatrix<TPrecision> b(3, 1);
      Linalg<TPrecision>::Zero(A);

      b(0, 0) = evaluate();
      std::cout << b(0, 0) << std::endl;
      Linalg<TPrecision>::AddScale(Z, -5*s, sync, Ztmp);
      Zswap = Z;
      Z = Ztmp;
      Ztmp = Zswap;
      Zchanged = true;
      b(1, 0) = evaluate();
      std::cout << b(1, 0) << std::endl;
      Linalg<TPrecision>::AddScale(Zswap, -10*s, sync, Z);
      Zchanged = true;
      b(2, 0) = evaluate();
      std::cout << b(2, 0) << std::endl;
        
      A(0, 2) = 1;
      A(1, 0) = 25;
      A(1, 1) = -5;
      A(1, 2) = 1;
      A(2, 0) = 100;
      A(2, 1) = -10;
      A(2, 2) = 1;

      DenseMatrix<TPrecision> q = Linalg<TPrecision>::Solve(A, b);

      //do step
      if( q(0, 0) > 0){
        //move Z to minimal point
        TPrecision h = -q(1, 0)/(2*q(0, 0));
        if(h<0){
          /*if(h< -10){
            h = -10;
          }*/
          std::cout << "a1 Step: " << h*s << std::endl;
          Linalg<TPrecision>::AddScale(Ztmp, h*s, sync, Z);
          //if(abs(h) < s/100){
          //  break;
          //}
        }
        else if(b(0,0) < b(1, 0)){
         //small step
         std::cout << "a2 Step: " << -s << std::endl;
         Linalg<TPrecision>::AddScale(Ztmp, -s, sync, Z);
        }
      }
      else if( b(0,0) > b(1, 0) ){
        //do nothing step to -10*s
        std::cout << "b Step: " << -10*s << std::endl;
      }
      else{
        //small step
        std::cout << "c Step: " << -s << std::endl;
        Linalg<TPrecision>::AddScale(Ztmp, -s, sync, Z);
      }

      A.deallocate();
      b.deallocate();
      q.deallocate();
      

      Zchanged = true;



      std::cout << std::endl;
      std::cout << "Iteration: " << i << std::endl;
      TPrecision mse = evaluate()/Y.N(); 
      std::cout << "MSE: " <<  mse << std::endl;
      if(msePrev < mse){
        break;
      }
      msePrev = mse;

      std::cout << std::endl << std::endl;
     }



     //cleanup
     sync.deallocate();
     gx.deallocate();
     Ztmp.deallocate();
   
   };
  




   //f(y) - coordinate mapping
   void f( DenseVector<TPrecision> &y, DenseVector<TPrecision> &out ){
     Linalg<TPrecision>::Set(out, 0);


     //do kernel regression 
     TPrecision sum = 0;
     for(unsigned int i=0; i<fY.N(); i++){
       TPrecision w = kernelY.f(y, Y, i);
       sum += w;
       Linalg<TPrecision>::AddScale(out, w, Z, i, out); 
     }
     Linalg<TPrecision>::Scale(out, 1.f/sum, out);
   };




   //f(y_i) - coordinate mapping
   void f(unsigned int yi, Vector<TPrecision> &out){
     Linalg<TPrecision>::Set(out, 0);
     TPrecision sum = 0;
     for(unsigned int i=0; i < fY.N(); i++){
       TPrecision w = KY(i, yi);
       sum += w;
       Linalg<TPrecision>::AddScale(out, w, Z, i, out);
     }
     Linalg<TPrecision>::Scale(out, 1.f/sum, out);
   };
  
   

   //Compute gradient of f, i.e. z_r 
   void gradX( int r, DenseVector<TPrecision> &gx){
     
     //update fY if necessary
     computefY();

     //initalize output to zero
     Linalg<TPrecision>::Zero(gx);
     DenseVector<TPrecision> tmp(Z.M());
     
     for(unsigned int i=0; i<fY.N(); i++){
       for(unsigned int j=i; j<fY.N(); j++){
         TPrecision s = DfY(i, j) - DL(i, j);
         s*= KYN(i,r) - KYN(j, r);
         Linalg<TPrecision>::Subtract(fY, i, fY, j, tmp);
         Linalg<TPrecision>::AddScale(gx, s, tmp, gx);
       }
     }     
   };

   

   //get original Y's
   DenseMatrix<TPrecision> getY(){
     return Y;
   };
   


   //get X (parameters for f
   DenseMatrix<TPrecision> getZ(){
    return Z;
   };



   //coordinate mapping fo Ypoints
   DenseMatrix<TPrecision> parametrize(DenseMatrix<TPrecision> &Ypoints){

     DenseMatrix<TPrecision> proj(Z.M(), Ypoints.N());
     parametrize(Ypoints, proj);

     return proj;
   };


   //
   void parametrize(DenseMatrix<TPrecision> &Ypoints, DenseMatrix<TPrecision> &proj){

     DenseVector<TPrecision> tmp(Y.M()); 
     DenseVector<TPrecision> xp(Z.M()); 

     for(unsigned int i=0; i < Ypoints.N(); i++){
       Linalg<TPrecision>::ExtractColumn(Ypoints, i, tmp);
       f(tmp, xp);
       Linalg<TPrecision>::SetColumn(proj, i, xp);
     }
     xp.deallocate();
     tmp.deallocate();
   };

  


   DenseMatrix<TPrecision> &parametrize(){
     computefY();
     return fY;
   };
  




private:




  void computefY(){
    if(Zchanged){
      std::cout << "updating f(Y)" << std::endl;
      Zchanged = false;
      DenseVector<TPrecision> tmp(Z.M());
      for(unsigned int i=0; i<Y.N(); i++){
        f(i, tmp);
        Linalg<TPrecision>::SetColumn(fY, i, tmp);
      }
      tmp.deallocate();
    }

    for(unsigned int i=0; i<fY.N(); i++){
      for(unsigned int j=i; j< fY.N(); j++){
        DfY(i, j) = sl2metric.distance(fY, i, fY, j);
        DfY(j, i) = DfY(i, j);
      }
    }
  };
 


  

  void computeKY(){
    unsigned int N = Y.N();
    KY = DenseMatrix<TPrecision>(N, N);
    sumKY = DenseVector<TPrecision>(N);
    Linalg<TPrecision>::Set(sumKY, 0);
    KYN = DenseMatrix<TPrecision>(N, N);



    std::cout << "Compute KY" << std::endl;
    for(unsigned int i=0; i < N; i++){
      for(unsigned int j=0; j < N; j++){
        if(i == j){
          KY(j, i) = 0;
        }
        else{
          KY(j, i) = kernelY.f(Y, j, Y, i);
          sumKY(i) += KY(j, i);
        }
      }
    }

    for(unsigned int i=0; i < KY.M(); i++){
      for(unsigned int j=0; j< KY.N(); j++){
        KYN(i, j) = KY(i, j) / sumKY(j); 
      }
    }

  };

  
}; 


#endif
