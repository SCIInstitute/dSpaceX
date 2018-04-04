#ifndef OPTIMALEPANECHNIKOVKERNELRECONSTRUCTION_H
#define OPTIMALEPANECHNIKOVKERNELRECONSTRUCTION_H

#include "Matrix.h"
#include "EuclideanMetric.h"
#include "Linalg.h"
#include "Kernel.h"

#include <vector>

template <typename TPrecision>
class OptimalEpanechnikovKernelReconstruction{
  public:
   typedef typename std::vector< DenseVector<TPrecision> > VList;
   typedef typename VList::iterator VListIterator;


  private:
    //VList Ysamples;
    VList Y;
    VList X;
    VList fY;

    int dimensionX;
    int dimensionY;
    TPrecision kCutGrad;

    EuclideanMetric<TPrecision> l2metric;

    Kernel<TPrecision, TPrecision> &kernelX;
    Kernel<TPrecision, TPrecision> &kernelY;
    
    DenseMatrix<TPrecision> KY;
    DenseVector<TPrecision> sumKY;
    DenseMatrix<TPrecision> KYN;

    int nky;
    bool Xchanged;

  public:
   
   KernelMap(TPrecision paramY, TPrecision paramX, int dim, int N) : 
     kernelX(kx), kernelY(ky), fY(N){

     KY = DenseMatrix<TPrecision>(N, N);
     sumKY = DenseVector<TPrecision>(N);
     KYN = DenseMatrix<TPrecision>(N, N);
     Linalg<TPrecision>::Set(sumKY, 0);
     nky = 0;
     
     dimensionX = dim;
     dimensionY = 0;
     kCutGrad = kernelCutoff;

     for(int i=0; i<N; i++){
      fY[i] = DenseVector<TPrecision>(dimensionX);
     }

     Xchanged = true;

   };

   
   int add(DenseVector<TPrecision> y, DenseVector<TPrecision> x){
     X.push_back(x);
     Y.push_back(y);
     dimensionY = y.N();
    
     return X.size() - 1;
   };


   int add(DenseVector<TPrecision> y){
     dimensionY = y.N();
     
     DenseVector<TPrecision> x(dimensionX);
     f(y, x);
     
     X.push_back(x);
     Y.push_back(y);
     
     return X.size() - 1;
   }  
    

   TPrecision evaluate(bool leaveout){
     TPrecision e = 0;

     computefY(leaveout);
     DenseVector<TPrecision> gfy(dimensionY);
     for(int i=0; i < Y.size(); i++){
        if(leaveout){
          g(fY[i], gfy, i);
        }
        else{
          g(fY[i], gfy);
        }
        e += l2metric.distanceSquared(Y[i], gfy); 
     }

     gfy.deallocate();

     return e;
   };
   

   //Gradient desxcent for all points 
   void gradDescent(int nIterations, TPrecision scaling, bool async, bool
       leaveout = false){
     DenseVector<TPrecision> gx(dimensionX);


     DenseMatrix<TPrecision> sync;
     if(!async){
      sync = DenseMatrix<TPrecision>(dimensionX, X.size());
     }

     std::cout << "gradDescent all:" << std::endl;
     for(int i=0; i<nIterations; i++){

      for(int j=0; j < X.size(); j++){
        gradX(j, gx, leaveout);
        if(async){
          DenseVector<TPrecision> tmp = X[j];
          Linalg<TPrecision>::AddScale(tmp, -scaling, gx, tmp);
          Xchanged = true;
        }
        else{
          for(int k=0; k<dimensionX; k++){
            sync(k, j) = gx(k);
          }
        }
      }

      if(!async){
        for(int j=0; j < sync.N(); j++){
          DenseVector<TPrecision> tmp = X[j];
          for(int k=0; k<dimensionX;k++){
            tmp(k) += -scaling * sync(k, j);
          }
        }
        Xchanged = true;
      }
      std::cout << std::endl;
      std::cout << "---------------------------Iteration: " << i << ", ";
      std::cout << "---------------------------E: " << evaluate(leaveout);
      std::cout << std::endl << std::endl << std::endl;
     }

     sync.deallocate();
     gx.deallocate();
   };
  




   
   void f( Vector<TPrecision> &y, Vector<TPrecision> &out, int lo = -1 ){
     Linalg<TPrecision>::Set(out, 0);

     DenseVector<TPrecision> k(Y.size());
     TPrecision sum = 0;
     int index = 0;
     for(VListIterator it = Y.begin(); it != Y.end(); ++it){
       k(index) = kernelY.f(*it, y); 
       sum += k(index);
       ++index;
     }

     for(int i=0; i<Y.size(); i++){
       if(k(i)!=0){
       //  std::cout << k(i) << ", "; 
        Linalg<TPrecision>::AddScale(out, k(i)/sum, X[i], out); 
       }
     };
     //std::cout << std::endl;

     k.deallocate();
     
   };



   void f( int yi, Vector<TPrecision> &out, bool lo){
     Linalg<TPrecision>::Set(out, 0);

     computeKY(lo);
     TPrecision sum = sumKY(yi);
     for(int i=0; i<Y.size(); i++){
        if(KY(i, yi) != 0){
          Linalg<TPrecision>::AddScale(out, KY(i, yi)/sum, X[i], out);
        }
     }

   };
  




   void g( Vector<TPrecision> &x, Vector<TPrecision> &out, int lo = -1){
     Linalg<TPrecision>::Set(out, 0);

     DenseVector<TPrecision> k(Y.size());
     TPrecision sum = 0;
     int index = 0;

     computefY( lo>=0 );
     for(VListIterator it = fY.begin(); it != fY.end(); ++it){
         
       if(index == lo){
         k(index) = 0;
       }
       else{
         k(index) = kernelX.f(*it, x);
       }

       if(k(index) != 0){
         sum += k(index);
       }
       ++index;
     }

     for(int i=0; i<Y.size(); i++){
       if( k(i) != 0){
        //std::cout << "i: " << i << "-" << k(i)/sum << ", "; 
        Linalg<TPrecision>::AddScale(out, k(i)/sum, Y[i], out); 
       } 
     } 
     //std::cout << std::endl << std::endl;


     k.deallocate();
   };

   

   //Compute gradient of f, e.g. \hat{x}_index 
   void grad( TPrecision &paramY, TPrecision &paramX, bool leaveout = true){
     paramY = 0;
     paramX = 0;
     
     //g(f(y_index))
     DenseVector<TPrecision> gfy(dimensionY);

     //kernel values & derivativesfor K(f(y_index) - f(y_j));
     DenseVector<TPrecision> kx(X.size());
     
     //
     DenseVector<TPrecision> yParamX(dimensionY);
     DenseVector<TPrecision> yParamY(dimensionY);

     //kernel parameter derivatives
     DenseVector<TPrecision> dxp(X.size());
     DenseVector<TPrecision> dyp(X.size());


     //Temp vars.
     DenseVector<TPrecision> gxtmp(dimensionX);
     DenseVector<TPrecision> diff(dimensionY);
     TPrecision tmp;
     
     //Update Y kernel values if necessary
     computeKY(leaveout);
     //update fY if necessary
     computefY(leaveout);

     //debug
     /*
     int ni = 0;
     int nk = 0;
     int niZero = 0;
     */

     //Compute gradient
     for(int i=0; i < Y.size(); i++){
       
      if(KYN(i, index) <= kCutGrad){ 
        //niZero++;
        continue;
      }
      
      //x-kernel values & derivatves at f(y)
      TPrecision sumkx = 0;
      TPrecision sumdxp = 0;

      for(int j=0; j< X.size(); j++){
        
        //derivative and kernel value
        if(i == j && leaveout){
          kx(j) = 0;
          dxp(j)= 0;
        }
        else{
          kx(j) = kernelX.f(fY[i], fY[j], gxtmp);
          dxp(j) = kernelX.gradKernelParam(Y[i], fY[j]);

        }
        sumkx += kx(j);
        sumdxp += dxp(j);
      }

      if(sumkx == 0){
        continue; 
      }

      TPrecision sumkx2 = sumkx*sumkx;

      //g(f(y_i)) 
      Linalg<TPrecision>::Set(gfy, 0);
      for(int j=0; j<Y.size(); j++){
        if( kx(j) != 0){
          Linalg<TPrecision>::AddScale(gfy, kx(j)/sumkx, Y[j], gfy); 
        } 
      }

      yParamX.zero();
      for(int j=0; j<Y.size(); j++){
        if(kx(j) != 0){
          Linalg<TPrecision>::ScaleAdd(Y[j], (dxp(j)*sumkx - kx(j) * sumdxp)/sumkx2, yParamX); 
        }
      }

      //d E / d \hat{x}_index
      Linalg<TPrecision>::Subtract(gfy, Y[i], diff);
      Linalg<TPrecision>::Scale(diff, 2, diff);
      paramX += Linalg<TPrecision>::Dot(yParamX, diff);
      
     }


     yParamX.deallocate();
     yParamY.deallocate();
     diff.deallocate();
     kx.deallocate();
     kxd.deallocate();
     sumkxd.deallocate();
     gfy.deallocate();

/*
     std::cout << "uncut: " << ni ;
     std::cout << ", nonzero Y: " << Y.size() - niZero;
     std::cout << ", nonzero X: " << nk/((double)ni) << " | ";
*/
   };





   DenseMatrix<TPrecision> getY(){
    DenseMatrix<TPrecision> Ytmp(dimensionY, Y.size());
    int index = 0;
    for(VListIterator yit = Y.begin(); yit != Y.end(); ++yit){
      for(int k=0; k < dimensionY; k++){
        Ytmp(k, index) = (*yit)(k);
      }
      ++index;
    }

    return Ytmp;
   }
   
  
/*
   DenseMatrix<TPrecision> getYsamples(){
    DenseMatrix<TPrecision> Ytmp(dimensionY, Ysamples.size());
    int index = 0;
    for(VListIterator yit = Ysamples.begin(); yit != Ysamples.end(); ++yit){
      for(int k=0; k < dimensionY; k++){
        Ytmp(k, index) = (*yit)(k);
      }
      ++index;

    }

    return Ytmp;
   }

*/


   DenseMatrix<TPrecision> getX(){
    DenseMatrix<TPrecision> Xtmp(dimensionX, X.size());
    int index = 0;
    for(VListIterator xit = X.begin(); xit != X.end(); ++xit){
        for(int k=0; k < dimensionX; k++){
          Xtmp(k, index) = (*xit)(k);
        }
        ++index;
      }

    return Xtmp;
   }


   //
   DenseMatrix<TPrecision> project(DenseMatrix<TPrecision> &Ypoints){

     DenseVector<TPrecision> tmp(dimensionY); 
     DenseVector<TPrecision> xp(dimensionX); 

     DenseMatrix<TPrecision> proj(dimensionX, Ypoints.N());
     for(int i=0; i < Ypoints.N(); i++){
       Linalg<TPrecision>::ExtractColumn(Ypoints, i, tmp);
       f(tmp, xp);
       Linalg<TPrecision>::SetColumn(proj, i, xp);
     }
     xp.deallocate();
     tmp.deallocate();
     return proj;
   };
  


   DenseMatrix<TPrecision> project(){

     DenseVector<TPrecision> xp(dimensionX); 

     DenseMatrix<TPrecision> proj(dimensionX, Y.size());
     for(int i=0; i < Y.size(); i++){
       f(i, xp, false);
       Linalg<TPrecision>::SetColumn(proj, i, xp);
     }
     xp.deallocate();
     return proj;
   };
  


   DenseMatrix<TPrecision> unproject(DenseMatrix<TPrecision> &Xpoints){

     DenseVector<TPrecision> tmp(dimensionX); 
     DenseVector<TPrecision> yp(dimensionY); 

     DenseMatrix<TPrecision> proj(dimensionY, Xpoints.N());
     for(int i=0; i < Xpoints.N(); i++){
       Linalg<TPrecision>::ExtractColumn(Xpoints, i, tmp);
       g(tmp, yp);
       Linalg<TPrecision>::SetColumn(proj, i, yp);
     }
     yp.deallocate();
     tmp.deallocate();
     return proj;
   };


private:


  void computefY(bool leaveout){
    if(Xchanged){
      std::cout << "updating f(Y)" << std::endl;
      Xchanged = false;
      for(int i=0; i<Y.size(); i++){
        f(i, fY[i], leaveout);
      }
    }
  };



  void computeKY(bool leaveout){
    if(nky == Y.size()){
      return;
    }

    std::cout << "Compute KY" << std::endl;
    for(int i=nky; i < Y.size(); i++){
      for(int j=0; j<Y.size(); j++){
        if(j == i && leaveout){
          KY(j, i) = 0; 
        }
        else{
          KY(j, i) = kernelY.f(Y[j], Y[i]); 
          sumKY(i) += KY(j, i);
        }
      }
    }
    for(int i=0; i < nky; i++){
      for(int j=nky; j<Y.size(); j++){ 
        if(j == i && leaveout){
          KY(j, i) = 0; 
        }
        else{
          KY(j, i) = kernelY.f(Y[j], Y[i]); 
          sumKY(i) += KY(j, i);
        }
      }
    }

    nky = Y.size();  

    //TODO: smarter update
    for(int i=0; i < nky; i++){
      for(int j=0; j< nky; j++){
        KYN(i, j) = KY(i, j) / sumKY(j); 
      }
    } 

  };

  
}; 


#endif
