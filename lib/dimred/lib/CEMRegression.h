#ifndef CEMRegression_H
#define CEMRegression_H


#include "Geometry.h"
#include "Matrix.h"
#include "EuclideanMetric.h"
#include "SquaredEuclideanMetric.h"
#include "KernelDensity.h"
#include "Linalg.h"
#include "GaussianKernel.h"
#include "MahalanobisKernel.h"

#include <list>
#include <iterator>
#include <stdlib.h>
#include <limits>
#include <math.h>


//Conditional Expectation Manifolds
template <typename TPrecision>
class CEMRegression{

  protected:    
    DenseMatrix<TPrecision> Y;
    DenseMatrix<TPrecision> Z;
    DenseMatrix<TPrecision> fY;
    DenseMatrix<TPrecision> S;

    unsigned int knnX;
    unsigned int knnY;
    
    EuclideanMetric<TPrecision> l2metric;
    SquaredEuclideanMetric<TPrecision> sl2metric;

    
    DenseMatrix<int> KNNY;
    DenseMatrix<TPrecision> KNNYD;
    DenseMatrix<TPrecision> KY;
    DenseVector<TPrecision> sumKY;
    DenseMatrix<TPrecision> KYN;


    
    DenseMatrix<int> KNNX;
    DenseMatrix<TPrecision> KNNXD;
    DenseMatrix<TPrecision> KX;
    DenseVector<TPrecision> sumKX;
    DenseMatrix<TPrecision> KXN;


    
    GaussianKernel<TPrecision> kernelX;
    MahalanobisKernel<TPrecision> *kernelY;
    
    
    DenseMatrix<TPrecision> coeff;
    TPrecision lambda;


  
  private:
    TPrecision sX;
    DenseVector<TPrecision> gfyTmp;

  
  
  public:
  
   virtual void cleanup(){      
     partialCleanup();
     Y.deallocate();
     Z.deallocate();
     
   };

   void partialCleanup(){  
     KNNX.deallocate();
     KNNXD.deallocate();	   
     KX.deallocate();
     sumKX.deallocate();
     KXN.deallocate();
     KNNY.deallocate();
     KNNYD.deallocate();
     KY.deallocate();
     sumKY.deallocate();
     KYN.deallocate();
     fY.deallocate();    
     S.deallocate();
     coeff.deallocate();
     gfyTmp.deallocate();
     delete[] kernelY;
   };




   //Create Condtional Expectation Manifold 
   CEMRegression(DenseMatrix<TPrecision> Ydata, DenseMatrix<TPrecision> Zinit, 
       DenseMatrix<TPrecision> labels, TPrecision alpha, unsigned int nnY,
       unsigned int nnX, TPrecision lambdaa) :
       Y(Ydata), Z(Zinit), knnY(nnY), knnX(nnX), S(labels), lambda(lambdaa){
             init();
        initKY();
        update();
        computeKernelX(alpha);
        updateKNNX();
        updateKY();
        update();

   };






   CEMRegression(DenseMatrix<TPrecision> Ydata, DenseMatrix<TPrecision> Zopt, 
       DenseMatrix<TPrecision> labels, unsigned int nnY, unsigned int nnX,
       double sigmaX, TPrecision lambdaa,
        MahalanobisKernel<TPrecision> *kY): Y(Ydata), Z(Zopt), knnY(nnY), knnX(nnX),
       S(labels), lambda(lambdaa) {
    
	       init();
      kernelX.setKernelParam(sigmaX);
      sX = kernelX.getKernelParam();  
      updateKY(false);
      update();
      init();
   }; 





   //evalue objective function, squared error
   TPrecision mse(int verbose=0){
     TPrecision e = 0;
     TPrecision re = 0;
     for(unsigned int i=0; i < Y.N(); i++){
       g(i, gfyTmp);
       e += sl2metric.distance(Y, i, gfyTmp);
       re += regressionResidual(i);
     }
    
     if(verbose > 0){
       std::cout << "manifold fit residual: " << e << std::endl;
       std::cout << "regression residual: " << lambda*re << std::endl << std::endl;
     }

     return (e+lambda*re)/Y.N();
   }


   //evalue objective function, squared error
   virtual TPrecision mse(TPrecision &o, int verbose=0 ){
     o=0;
     TPrecision e = 0;
     TPrecision re = 0;
     
     //Jacobian of g(x)
     DenseMatrix<TPrecision> J(Y.M(), Z.M());
     
     //Temp vars.
     DenseVector<TPrecision> diff(Y.M());
     DenseVector<TPrecision> pDot(Z.M());

     for(unsigned int i=0; i < Y.N(); i++){
       g(i, gfyTmp, J);
       e += sl2metric.distance(Y, i, gfyTmp);
       re += regressionResidual(i);

       Linalg<TPrecision>::Subtract(gfyTmp, Y, i, diff);  
   
       for(unsigned int n=0; n< J.N(); n++){
         TPrecision norm = 0;
         for(unsigned int j=0; j<J.M(); j++){
           norm += J(j, n) * J(j, n);
         }
         if(norm == 0){
           norm = 0.0001;
         };
         norm = sqrt(norm);
         
         for(unsigned int j=0; j<J.M(); j++){
           J(j, n) /= norm;
         }
       }

       Linalg<TPrecision>::Normalize(diff);
       Linalg<TPrecision>::Multiply(J, diff, pDot, true);

       for(unsigned int n=0; n< pDot.N(); n++){
        o += acos(sqrt(pDot(n)*pDot(n)));
       }  
     }
     o = o/(Z.M()*Z.N())/ M_PI * 180;
          
     pDot.deallocate();
     diff.deallocate();
     J.deallocate();

     if(verbose > 0){
       std::cout << "manifold fit residual: " << e << std::endl;
       std::cout << "regression residual: " << lambda*re << std::endl << std::endl;
     }

     return (e+lambda*re)/Y.N();
   };

  

   
     


   //Gradient descent for all points 
   void gradDescent(unsigned int nIterations, TPrecision scaling, int verbose=1, int burn = 4){
     TPrecision orthoPrev =0;
     TPrecision ortho;

     TPrecision objPrev = mse(orthoPrev, verbose);

     if(verbose > 0){
       std::cout << "Mse start: " << objPrev << std::endl;
       std::cout << "Ortho start: " << orthoPrev << std::endl;
     }


     //---Storage for syncronous updates 
     DenseMatrix<TPrecision> sync(Z.M(), Z.N());

     //---Do nIterations of gradient descent     
     DenseMatrix<TPrecision> Ztmp(Z.M(), Z.N());
     DenseMatrix<TPrecision> Zswap;

     //gradient direction
     DenseVector<TPrecision> gx(Z.M());     
     if(verbose > 0){
       std::cout << "Start Gradient Descent" << std::endl << std::endl;
     }

     for(unsigned int i=0; i<nIterations; i++){
      updateKY();
      update();
      //compute gradient for each point
      TPrecision maxL = 0;
      for(unsigned int j=0; j < Z.N(); j++){
        //compute gradient
        //gradX(j, gx);
	      numGradX(j, gx, sX/10.f);
        
        //store gradient for syncronous updates
        TPrecision l = Linalg<TPrecision>::Length(gx);
      	if(maxL < l){
          maxL = l;
	      }
	      //Linalg<TPrecision>::Scale(gx, 1.f/l, gx);
        for(unsigned int k=0; k<Z.M(); k++){
          sync(k, j) = gx(k);
        }
      }



      //sync updates
      TPrecision s;
      if(maxL == 0 )
	      s = scaling;
      else{
	      s = scaling * sX/maxL;
      }     
      if(verbose > 1){
        std::cout << "scaling: " << s << std::endl;
      }
      

      //Approximate line search with quadratic fit
      DenseMatrix<TPrecision> A(3, 3);
      DenseMatrix<TPrecision> b(3, 1);
      Linalg<TPrecision>::Zero(A);

      b(0, 0) = mse();
      Linalg<TPrecision>::AddScale(Z, -1*s, sync, Ztmp);
      Zswap = Z;
      Z = Ztmp;
      Ztmp = Zswap;
      update();

      b(1, 0) = mse();
      Linalg<TPrecision>::AddScale(Zswap, -2*s, sync, Z);
      update();

      b(2, 0) = mse();
      
      if(verbose > 1){
        std::cout << "line search: " << std::endl;
        std::cout << b(0, 0) << std::endl;
        std::cout << b(1, 0) << std::endl;
        std::cout << b(2, 0) << std::endl;
      }
        
      A(0, 2) = 1;
      A(1, 0) = 1*s*s;
      A(1, 1) = -1*s;
      A(1, 2) = 1;
      A(2, 0) = 4*s*s;
      A(2, 1) = -2*s;
      A(2, 2) = 1;

      DenseMatrix<TPrecision> q = Linalg<TPrecision>::Solve(A, b);

      //do step
      if( q(0, 0) > 0){
        TPrecision h = -q(1, 0)/(2*q(0, 0));
        if(h < -2*s){
         h = -2*s;
        }
        else if( h > 1){
          h = 1;
        }
        Linalg<TPrecision>::AddScale(Ztmp, h, sync, Z);
      }
      else if( b(0,0) > b(1, 0) ){
        //do nothing step to -10*s
      }
      else{
        //stop gradient descent - no step
        //Zswap = Ztmp;
        //Ztmp = Z;
        //Z = Zswap;
        Linalg<TPrecision>::AddScale(Ztmp, -0.3*s, sync, Z);
      }

      A.deallocate();
      b.deallocate();
      q.deallocate();


      //Linalg<TPrecision>::AddScale(Z, -s, sync, Z);
      update();

      TPrecision obj = mse(ortho, verbose); 
      if(verbose > 0){
        std::cout << "Iteration: " << i << std::endl;
        std::cout << "MSE: " <<  obj << std::endl;     
        std::cout << "Ortho: " <<  ortho << std::endl << std::endl;
      }   
      if(objPrev <= obj){// || orthoPrev >= ortho){
	if(i > burn){
          break;
        }
      }
      objPrev = obj;      

     }


     //cleanup 
     sync.deallocate();
     gx.deallocate();
     Ztmp.deallocate();
   
   };
 

 


  //f(x_index) - coordinate mapping
  void f(unsigned int index, Vector<TPrecision> &out, int start = 0){
    Linalg<TPrecision>::Zero(out);
    TPrecision sumw = 0;
    //std::cout  <<index << ":  "; 
    for(unsigned int i=start; i < Y.N(); i++){
      //if(i==index) continue;
      //int nn = KNNY(i, index);
      TPrecision w = KY(i, index);
      //std::cout << nn << ":" << w << " ";
      Linalg<TPrecision>::AddScale(out, w, Z, i, out);
      sumw += w;

    } 
    //std::cout << std::endl;    
    Linalg<TPrecision>::Scale(out, 1.f/sumw, out);
  };




   //f(x) - cooridnate mapping
   void f( DenseVector<TPrecision> &y, Vector<TPrecision> &out){
    Linalg<TPrecision>::Zero(out);
    TPrecision sumw = 0;
    for(unsigned int i=0; i < Y.N(); i++){
      TPrecision w = kernelY.f(y, Y, i);
      Linalg<TPrecision>::AddScale(out, w, Z, i, out);
      sumw += w;
    }     
    Linalg<TPrecision>::Scale(out, 1.f/sumw, out);
   };



  //---g 0-order
/*
  //g(y_index) - reconstruction mapping
  void g(unsigned int index, Vector<TPrecision> &out){
     Linalg<TPrecision>::Set(out, 0);

     TPrecision sum = 0;

     computefY();

     
     for(unsigned int i=0; i < 2*knnSigma; i++){
	int j = KNNX(i, index);
        double w = kernelX.f(fY, j, fY, index);
        Linalg<TPrecision>::AddScale(out, w, Y, j, out); 
        sum += w;
     }

     Linalg<TPrecision>::Scale(out, 1.f/sum, out);
   };


   //g(x) - reconstruction mapping
   void g( Vector<TPrecision> &x, Vector<TPrecision> &out){
     Linalg<TPrecision>::Set(out, 0);

     computefY();

     TPrecision sum = 0;
     for(unsigned int i=0; i < Y.N(); i++){ 
        TPrecision w = kernelX.f(x, fY, i);
        Linalg<TPrecision>::AddScale(out, w, Y, i, out); 
        sum += w;
     }
     Linalg<TPrecision>::Scale(out, 1.f/sum, out);
   };
  
   */

   
  //------g 1-order
  //g(x_index) - reconstruction mapping
  void g(unsigned int index, Vector<TPrecision> &out){
     DenseMatrix<TPrecision> sol = LeastSquares(index);
     
     for(unsigned int i=0; i<Y.M(); i++){
       out(i) = sol(0, i);
     }

     sol.deallocate();
   };  
   


   //g(x_index) - reconstruction mapping plus tangent plane
   void g(unsigned int index, Vector<TPrecision> &out, Matrix<TPrecision> &J){
     DenseMatrix<TPrecision> sol = LeastSquares(index);
     
     for(unsigned int i=0; i<Y.M(); i++){
       out(i) = sol(0, i);
     }
     for(unsigned int i=0; i< Z.M(); i++){
       for(unsigned int j=0; j< Y.M(); j++){
         J(j, i) = sol(1+i, j);
       }
     }

     sol.deallocate();
   };




   //g(x) - reconstruction mapping
   void g( Vector<TPrecision> &x, Vector<TPrecision> &out){
     DenseMatrix<TPrecision> sol = LeastSquares(x);
     
     for(unsigned int i=0; i<Y.M(); i++){
       out(i) = sol(0, i);
     }

     sol.deallocate();
   };   
  


   
   //g(x) - reconstruction mapping + tangent plance
   void g( Vector<TPrecision> &x, Vector<TPrecision> &out, Matrix<TPrecision> &J){
     DenseMatrix<TPrecision> sol = LeastSquares(x);
     for(unsigned int i=0; i<Y.M(); i++){
       out(i) = sol(0, i);
     }     
     for(unsigned int i=0; i< Z.M(); i++){
       for(unsigned int j=0; j< Y.M(); j++){
         J(j, i) = sol(1+i, j);
       }
     }

     sol.deallocate();
   };


 


   //get original Y's
   DenseMatrix<TPrecision> getY(){
     return Y;
   };
   


   //get Z (parameters for f
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
     return fY;
   };
  


   DenseMatrix<TPrecision> reconstruct(DenseMatrix<TPrecision> &Xpoints){
     DenseMatrix<TPrecision> proj(Y.M(), Xpoints.N());
     reconstruct(Xpoints, proj);     
     return proj;
   };



  

   void reconstruct(DenseMatrix<TPrecision> &Xpoints, DenseMatrix<TPrecision> &proj){
     DenseVector<TPrecision> tmp(Z.M()); 
     DenseVector<TPrecision> yp(Y.M()); 
     for(unsigned int i=0; i < Xpoints.N(); i++){
       Linalg<TPrecision>::ExtractColumn(Xpoints, i, tmp);
       g(tmp, yp);
       Linalg<TPrecision>::SetColumn(proj, i, yp);
     }
     yp.deallocate();
     tmp.deallocate();
   };   
   
   
   DenseMatrix<TPrecision> reconstruct(){
     DenseMatrix<TPrecision> proj(Y.M(), Y.N());
     DenseVector<TPrecision> yp(Y.M()); 
     for(unsigned int i=0; i < Y.N(); i++){
       g(i, yp);
       Linalg<TPrecision>::SetColumn(proj, i, yp);
     }
     yp.deallocate();
     return proj;
   };




   //Compute the log probability of Yt_i belonging to this manifold, by computing a
   //local variance orthogonal to the manifold. The probability is the a
   //gaussian according to this variance and mean zero off the manifold
   //-Yt  data to test
   //-Ytp projection of Yt onto this manifold
   //-Xt manifold paprametrization of Yt
   //-Yp rpojection of the training data set onto this manifold
   //-p - output of pdf values for each point
   //-var - variances for each point
   void pdf(DenseMatrix<TPrecision> Yt, DenseMatrix<TPrecision> Ytp,
       DenseMatrix<TPrecision> Xt,
       DenseVector<TPrecision> &p, DenseVector<TPrecision> &var,
       DenseVector<TPrecision> &pk, bool useDensity){

     //update fY if necessary

     DenseMatrix<TPrecision> Yp = reconstruct(fY);
     
     TPrecision cod = (TPrecision) (Y.M() - fY.M())/2.0;

     //compute trainig set squared distances
     DenseVector<TPrecision> sdist(Y.N());
     for(unsigned int i=0; i< Y.N(); i++){
       sdist(i) = sl2metric.distance(Y, i, Yp, i);
     }

     DenseVector<TPrecision> k(Y.N());

     //compute variances and pdf values
     TPrecision c = -cod * log(2*M_PI);
     DenseVector<TPrecision> xt(Xt.M());

     for(unsigned int i=0; i < Xt.N(); i++){

       TPrecision sum = 0;
       TPrecision vartmp = 0;
       for(unsigned int j=0; j < fY.N(); j++){
         k(j) = kernelX.f(Xt, i, fY, j);
         sum += k(j);
         vartmp += sdist(j) * k(j); 
       } 
       var(i) = vartmp / sum;
       
       TPrecision d = sl2metric.distance(Yt, i, Ytp, i);
       p(i) = c - cod * log(var(i)) - d / ( 2 * var(i) ) ;
     }
     
     if(useDensity){
       TPrecision n = log(fY.N());
       KernelDensity<TPrecision> kd(fY, kernelX);
       for(unsigned int i=0; i<p.N(); i++){
         pk(i) = log( kd.p(Xt, i, true)) - n;
       }
     }


     xt.deallocate(); 
     sdist.deallocate(); 
     k.deallocate();
     Yp.deallocate();
   };


   TPrecision getSigmaX(){
     return kernelX.getKernelParam();
   };

   GaussianKernel<TPrecision> getKernelX(){
     return kernelX;
   };




      MahalanobisKernel<TPrecision> *getKernelsY(){
        return kernelY;
      };



private:

   


      void init(){
        kernelX = GaussianKernel<TPrecision>( Z.M());
        fY = Linalg<TPrecision>::Copy(Z);
        gfyTmp = DenseVector<TPrecision>(Y.M());
        unsigned int N = Y.N();

        if(knnX <= Z.M()){
          knnX = Z.M()+1;
        }	  
        if(knnX > N){
          knnX = N-1;
        }
        if(knnY > N){
          knnY = N;
        }

        KY = DenseMatrix<TPrecision>(N, N);
        sumKY = DenseVector<TPrecision>(N);
        Linalg<TPrecision>::Set(sumKY, 0);
        KYN = DenseMatrix<TPrecision>(N, N);
        KNNY =  DenseMatrix<int>(knnY, N);
        KNNYD = DenseMatrix<TPrecision>(knnY, N);
        Geometry<TPrecision>::computeKNN(Y, KNNY, KNNYD, sl2metric);

        KNNX = DenseMatrix<int>(knnX+1, N);
        KNNXD = DenseMatrix<TPrecision>(knnX+1, N);
        kernelX = GaussianKernel<TPrecision>( Z.M());
        KX = DenseMatrix<TPrecision>(N, N);
        sumKX = DenseVector<TPrecision>(N);
        Linalg<TPrecision>::Set(sumKX, 0);
        KXN = DenseMatrix<TPrecision>(N, N);


      };     





  void update(){
    computefY();
    updateKNNX();
    updateLM();
  };





  void computefY(){
    DenseVector<TPrecision> tmp(Z.M());
    for(unsigned int i=0; i<Y.N(); i++){
      f(i, tmp);
      Linalg<TPrecision>::SetColumn(fY, i, tmp);
    }
    tmp.deallocate();
  };





  void updateKNNX(){
     unsigned int N = Y.N();
     Geometry<TPrecision>::computeKNN(fY, KNNX, KNNXD, sl2metric);
     for(unsigned int i=0; i < N; i++){
      for(unsigned int j=0; j < N; j++){
        KX(j, i) = kernelX.f(fY, j, fY, i); 
        sumKX(i) += KX(j, i);
      }
    }

    for(unsigned int i=0; i < KX.M(); i++){
      for(unsigned int j=0; j< KX.N(); j++){
        KXN(i, j) = KX(i, j) / sumKX(j); 
      }
    }
  }; 





  void computeKernelX(TPrecision alpha){
    TPrecision sigma = 0;
    sX = 0;
    for(unsigned int i=0; i < Z.N(); i++){
      sigma += sqrt( KNNXD(knnX-1, i) );
    }
    sigma *= alpha/Z.N();
    sX = sigma/alpha;
    

    //std::cout << "sigmaX: " << sigma << std::endl;
    //std::cout << "scale: " << sX << std::endl;
    
    kernelX.setKernelParam(sigma);
  };

  
  



      void initKY(){
        unsigned int N = Y.N();
        kernelY = new MahalanobisKernel<TPrecision>[N];
        for(int i = 0; i<N; i++){
          DenseMatrix<TPrecision> ev(Y.M(), Z.M());
          DenseVector<TPrecision> vars(Z.M());
          DenseVector<TPrecision> mean = Linalg<TPrecision>::ExtractColumn(Y, i);
          MahalanobisKernelParam<TPrecision> param(ev, vars, 1, mean);
          kernelY[i].setKernelParam(param);
        }

        TPrecision sigma = 0;
        for(unsigned int i=0; i<N; i++){
          sigma += sqrt( KNNYD(knnY-1, i) ); 
        }
        sigma /= 3*N;
        GaussianKernel<TPrecision> kY = GaussianKernel<TPrecision>(sigma, Z.M());
        for(unsigned int i=0; i < N; i++){
          for(unsigned int j=0; j < N; j++){
            KY(j, i) = kY.f(Y, j, Y, i);
            sumKY(i) += KY(j, i);
          }
        }

        for(unsigned int i=0; i < KY.M(); i++){
          for(unsigned int j=0; j< KY.N(); j++){
            KYN(i, j) = KY(i, j) / sumKY(j); 
          }
        }

      };




      TPrecision varDecomposition(int index, DenseMatrix<TPrecision> ev,
          DenseVector<TPrecision> var, DenseVector<TPrecision> mean){
        TPrecision varO=0;
        TPrecision sumw = 0;
        TPrecision sumw2 = 0;
        for(int i=0; i<knnX; i++){
          int nn = KNNX(i, index);
          TPrecision w = KX(nn, index);
          g(nn, mean);
          varO += w* sl2metric.distance(Y, nn, mean);
          sumw += w;
          sumw2 += w*w;
        }
        TPrecision nw = sumw / (sumw*sumw - sumw2);
        varO = varO *nw;

        g(index, mean, ev);
        DenseMatrix<TPrecision> tmp = Linalg<TPrecision>::QR(ev);
        Linalg<TPrecision>::Copy(tmp, ev);
        tmp.deallocate();
        //DenseVector<TPrecision> ls = Linalg<TPrecision>::ColumnwiseNorm(ev);
        //for(int i=0; i<ev.N(); i++){
        //  Linalg<TPrecision>::ScaleColumn(ev, i, 1.0/ls(i)); 
        //}


        DenseVector<TPrecision> diff(Y.M());
        DenseVector<TPrecision> lPlane(Y.M());
        Linalg<TPrecision>::Zero(var);
        for(int i=0; i<knnY; i++){
          int nn = KNNY(i, index);
          //TPrecision w = KX(nn, index);
          Linalg<TPrecision>::Subtract(Y, nn, mean, diff);
          Linalg<TPrecision>::Multiply(ev, diff, lPlane, true);
          for(int j=0; j<var.N(); j++){
            var(j) += lPlane(j) *lPlane(j);
          }
        }
        Linalg<TPrecision>::Scale(var, 1.0/(knnY-1), var);
        

        diff.deallocate();
        lPlane.deallocate();
        //ls.deallocate();


        return varO;
      };
 




      void updateKY(bool updateK = true){
        unsigned int N = Y.N();

        if(updateK){
          DenseVector<TPrecision> varPlane(Z.M());
          TPrecision varOrtho = 0;
          for(int i=0; i<N; i++){
            MahalanobisKernelParam<TPrecision> &p = kernelY[i].getKernelParam();
            p.varOrtho = varDecomposition(i, p.ev, p.var, p.mean);
            varOrtho += p.varOrtho;
            Linalg<TPrecision>::Add(varPlane, p.var, varPlane);
          } 

          std::cout << "vo: " << varOrtho/Y.N() << std::endl;
          Linalg<TPrecision>::Scale(varPlane, 1.0/Y.N(), varPlane);
          for(int i=0; i<varPlane.N(); i++){
            std::cout << "vp: " << varPlane(i) << std::endl;
          }
          varPlane.deallocate();
        }

        for(unsigned int i=0; i < N; i++){
          for(unsigned int j=0; j < N; j++){
            KY(j, i) = kernelY[i].f(Y, j);
            sumKY(i) += KY(j, i);
          }
        }

        for(unsigned int i=0; i < KY.M(); i++){
          for(unsigned int j=0; j< KY.N(); j++){
            KYN(i, j) = KY(i, j) / sumKY(j); 
          }
        }

      }; 






   TPrecision localMSE(int index, int start=0){
     TPrecision e = 0;
     TPrecision re = 0;
     for(unsigned int i=start; i < knnX; i++){
       int nn = KNNX(i, index);

       //reconstruction error
       g(nn, gfyTmp);
       e += sl2metric.distance(Y, nn, gfyTmp);
       
       //regression error
       re += regressionResidual(nn);
        
     }
     return (e+lambda*re)/(knnX-start);
   };




   TPrecision regressionResidual(int index){
     TPrecision re = 0;
     for(int k=0; k<S.M(); k++){
       TPrecision tmp = coeff(0, k);
       for(int j=0; j<fY.M(); j++){
         tmp += coeff(j+1, k) * fY(j, index); 
       }
       tmp = tmp - S(k, index);
       re += tmp*tmp;
     }
     return re;
   };





   //Compute gradient of f, i.e. z_r 
   void gradX( int r, DenseVector<TPrecision> &gx){
     
     //update fY if necessary
     computefY();

     //initalize output to zero
     Linalg<TPrecision>::Zero(gx);
     
     unsigned int YM = Y.M();
     unsigned int ZM = Z.M();
     unsigned int N = Y.N();

     //Kernel values
     DenseVector<TPrecision> k(knnY);
     TPrecision sumk = 0;
     DenseVector<TPrecision> sumkg(ZM);
     DenseVector<TPrecision> dfkg[knnY];
     for(unsigned int i=0; i<knnY; i++){
       dfkg[i] = DenseVector<TPrecision>(ZM);
     }
     DenseVector<TPrecision> sumdfkg(ZM);
     
     //g(f(y_r))
     DenseVector<TPrecision> gfy(YM);
     DenseMatrix<TPrecision> dgfdz(YM, ZM);
     
     //Temp vars.     
     DenseVector<TPrecision> gtmp(ZM);
     DenseVector<TPrecision> diff(YM);

     //Compute gradient
     for(unsigned int knn=0; knn < knnY; knn++){

       int yi = KNNY(knn, r);

       //Precompute kernel values and first and second order derivatives 
       sumk=0;     
       Linalg<TPrecision>::Zero(sumdfkg);

       for(unsigned int i=0; i < knnY; i++){
	       int yj = KNNX(i, yi);
	       //df
	       TPrecision df = KYN( yi, r ) - KYN( yj, r );
         k(i) = kernelX.gradf(fY, yi, fY, yj, dfkg[i]);
         sumk += k(i);

         Linalg<TPrecision>::Scale(dfkg[i], df, dfkg[i]);
         Linalg<TPrecision>::Add(sumdfkg, dfkg[i], sumdfkg);
       }
       TPrecision sumk2 = sumk*sumk;
       if(sumk2 == 0){
         continue;
       }


       //g(f(y_i)) 
       Linalg<TPrecision>::Zero(gfy);
       for(unsigned int j=0; j < knnY; j++){
         Linalg<TPrecision>::AddScale(gfy, k(j), Y, j, gfy); 
       }
       Linalg<TPrecision>::Scale(gfy, 1.f/sumk, gfy);

 
       //d g(f(yi)) - yi / d z_r
       Linalg<TPrecision>::Zero(dgfdz);
       for(unsigned int j = 0; j < knnY; j++){
         //if(j == r) continue;
         for(unsigned int n=0; n<ZM; n++){
           TPrecision tmp =  ( dfkg[j](n) * sumk - k(j) * sumdfkg(n) ) / sumk2;
           for(unsigned int m=0; m < YM ; m++){
             dgfdz(m, n) +=  tmp * Y(m, j);
           }
         }
       }
       
       Linalg<TPrecision>::Subtract(gfy, Y, yi, diff);
       Linalg<TPrecision>::Scale(diff, 2, diff);
       Linalg<TPrecision>::Multiply(dgfdz, diff, gtmp, true);
       Linalg<TPrecision>::Add(gx, gtmp, gx);
     }


     	
     //cleanup
     dgfdz.deallocate();
     for(unsigned int i=0; i<knnY; i++){
       dfkg[i].deallocate();
     }
     gtmp.deallocate();
     diff.deallocate();
     k.deallocate();
     sumdfkg.deallocate();
     gfy.deallocate();
   };






   
   //numerical gradient computation
   virtual void numGradX(int r, DenseVector<TPrecision> &gx, TPrecision epsilon){

    int kmse = 5;
    TPrecision eg = 0;
    TPrecision e = localMSE(r);
    DenseVector<TPrecision> fy(Z.M());
    for(unsigned int i=0; i<gx.N(); i++){
      Z(i, r) += epsilon;
      //update nearest neighbors
      for(unsigned int k=0; k<knnX; k++){
        int nn = KNNX(k, r);
      	f(nn, fy);
        Linalg<TPrecision>::SetColumn(fY, nn, fy);
      }
      //f(r, fy);
      ///Linalg<TPrecision>::SetColumn(fY, r, fy);
      eg = localMSE(r); 
      gx(i) = ( eg - e ) / epsilon;
      Z(i, r) -= epsilon;
    }

    //f(r, fy);
    //Linalg<TPrecision>::SetColumn(fY, r, fy);

    //update nearest neighbors
    for(unsigned int k=0; k<knnX; k++){
      int nn = KNNX(k, r);
      f(nn, fy);
      Linalg<TPrecision>::SetColumn(fY, nn, fy);
    }

    fy.deallocate();
   };







   
  DenseMatrix<TPrecision> LeastSquares(Vector<TPrecision> &x){
    DenseVector<int> knn(knnX);
    DenseVector<TPrecision> knnDist(knnX);
     Geometry<TPrecision>::computeKNN(fY, x, knn, knnDist, sl2metric);

    DenseMatrix<TPrecision> A(knnX, Z.M()+1);
    DenseMatrix<TPrecision> b(knnX, Y.M());

    for(unsigned int i=0; i < knnX; i++){
       unsigned int nn = knn(i);
       TPrecision w = kernelX.f(knnDist(i));
       A(i, 0) = w;
       for(unsigned int j=0; j< Z.M(); j++){
         A(i, j+1) = (fY(j, nn)-x(j)) * w;
       }

       for(unsigned int m = 0; m<Y.M(); m++){
	       b(i, m) = Y(m, nn) *w;
       }
     }
     
     DenseMatrix<TPrecision> sol = Linalg<TPrecision>::LeastSquares(A, b);

     A.deallocate();
     b.deallocate();
     return sol;
  };






  DenseMatrix<TPrecision> LeastSquares(unsigned int n){

    DenseMatrix<TPrecision> A(knnX, Z.M()+1);
    DenseMatrix<TPrecision> b(knnX, Y.M());

    for(unsigned int i=0; i < knnX; i++){
       unsigned int nn = KNNX(i, n);
       TPrecision w = KX(nn, n);
       A(i, 0) = w;
       for(unsigned int j=0; j< Z.M(); j++){
         A(i, j+1) = (fY(j, nn)-fY(j, n)) * w;
       }

       for(unsigned int m = 0; m<Y.M(); m++){
         b(i, m) = Y(m, nn) *w;
       }
     }
     
     DenseMatrix<TPrecision> sol = Linalg<TPrecision>::LeastSquares(A, b);

     A.deallocate();
     b.deallocate();
     return sol;
    
  };  
  
  
  
  
  
  
  
  
  
  void updateLM(){      

    //update linear model
    DenseMatrix<TPrecision> A(fY.N(), fY.M()+1);
    for(unsigned int i=0; i<A.M(); i++){
      A(i, 0) = 1;
      for(unsigned int j=1; j<A.N(); j++){
        A(i, j) = fY(j-1, i);
      }
    }
      
    DenseMatrix<TPrecision> b(S.N(), S.M());
    for(unsigned int i=0; i<b.M(); i++){
      for(unsigned int j=0; j<b.N(); j++){
        b(i, j) = S(j, i);
      }
    }

    coeff.deallocate();
    coeff = Linalg<TPrecision>::LeastSquares(A, b);
    A.deallocate();
    b.deallocate();
  }

}; 


#endif

