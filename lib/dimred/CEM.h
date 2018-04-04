#ifndef CEM_H
#define CEM_H


#include "Geometry.h"
#include "Matrix.h"
#include "EuclideanMetric.h"
#include "SquaredEuclideanMetric.h"
#include "KernelDensity.h"
#include "Linalg.h"
#include "GaussianKernel.h"
#include "MahalanobisKernel.h"
#include "SymmetricEigensystem.h"

#include <list>
#include <iterator>
#include <stdlib.h>
#include <limits>
#include <math.h>

//Minimize MSE
#define CEM_MSE 0
//Minimize <g(f(y)) - y , g'(f(y))>
#define CEM_ORTHO 1
//Minimize <g(f(y)) - y , g'(f(y))> with g'(f(y)) normalized
#define CEM_ORTHO_NORMALIZE 2 
//Minimize <g(f(y)) - y , g'(f(y))> with g'(f(y)) and g(f(y)) - y  normalized
#define CEM_ORTHO_NORMALIZE2 3 
//Minimize <g(f(y)) - y , g'(f(y))> with  g(f(y)) - y  normalized
#define CEM_ORTHO_NORMALIZE3 4


//Conditional Expectation Manifolds
template <typename TPrecision>
class CEM{

  private:    
    DenseMatrix<TPrecision> Y;
    DenseMatrix<TPrecision> Z;
    DenseMatrix<TPrecision> fY;

    unsigned int knnY;
    unsigned int knnX;
    TPrecision sX;

    EuclideanMetric<TPrecision> l2metric;
    SquaredEuclideanMetric<TPrecision> sl2metric;


    DenseMatrix<int> KNNY;
    DenseMatrix<TPrecision> KNNYD;
    //DenseMatrix<TPrecision> KY;
    //DenseVector<TPrecision> sumKY;
    //DenseMatrix<TPrecision> KYN;


    DenseMatrix<int> KNNX;
    DenseMatrix<TPrecision> KNNXD;
    //DenseMatrix<TPrecision> KX;
    //DenseVector<TPrecision> sumKX;
    //DenseMatrix<TPrecision> KXN;



    GaussianKernel<TPrecision> kernelX;
    GaussianKernel<TPrecision> kernelY;


    bool qfit;


  public:


    virtual void cleanup(){      
      partialCleanup();
      Y.deallocate();
      Z.deallocate();

    };

    void partialCleanup(){  
      KNNX.deallocate();
      KNNXD.deallocate();	   
      //KX.deallocate();
      //sumKX.deallocate();
      //KXN.deallocate();
      KNNY.deallocate();
      KNNYD.deallocate();
      //KY.deallocate();
      //sumKY.deallocate();
      //KYN.deallocate();
      fY.deallocate();
    };


    //Create Condtional Expectation Manifold 
    CEM(DenseMatrix<TPrecision> Ydata, DenseMatrix<TPrecision> Zinit,
        unsigned int nnY, unsigned int nnX, TPrecision sigmaY, TPrecision sigmaX,
        bool sigmaAsFactor = true, bool quadratic = false ) :
      Y(Ydata), Z(Zinit), knnY(nnY), knnX(nnX), qfit(quadratic){

        init();
        if(sigmaAsFactor){
          computeKernelY(sigmaY);
          //computeKY();
          update();
          computeKernelX(sigmaX);
          //computeKX();
        }
        else{
          kernelY.setKernelParam(sigmaY);
          //computeKY();
          kernelX.setKernelParam(sigmaX);
          update();
        }
      };




    //evalue objective function, squared error
    TPrecision mse(){
      TPrecision e = 0;
      DenseVector<TPrecision> gfy(Y.M());
      for(unsigned int i=0; i < Y.N(); i++){
        g(i, gfy);
        e += sl2metric.distance(Y, i, gfy);
      }
      gfy.deallocate();
      return e/Y.N();
    }


    //evalue objective function, squared error
    TPrecision mse(TPrecision &o, int type){
      o=0;
      TPrecision e = 0;

      //Jacobian of g(x)
      DenseMatrix<TPrecision> J(Y.M(), Z.M());

      //Temp vars.
      DenseVector<TPrecision> gfy(Y.M());
      DenseVector<TPrecision> diff(Y.M());
      DenseVector<TPrecision> pDot(Z.M());

      DenseVector<TPrecision> ml(J.N());
      Linalg<TPrecision>::Zero(ml);


      for(unsigned int i=0; i < Y.N(); i++){
        g(i, gfy, J);
        //e += sl2metric.distance(Y, i, gfy);

        Linalg<TPrecision>::Subtract(gfy, Y, i, diff);  
        e += Linalg<TPrecision>::SquaredLength(diff);

        //Debug 
        for(int k = 0; k<J.N(); k++){
          TPrecision length = 0;
          for(int l = 0; l<J.M(); l++){
            length += J(l, k) * J(l, k);
          }
          ml(k) += sqrt(length);
        }


        if(type == CEM_ORTHO_NORMALIZE){
          Linalg<TPrecision>::QR_inplace(J);
        }
        if(type == CEM_ORTHO_NORMALIZE2 || type == CEM_MSE){
          Linalg<TPrecision>::QR_inplace(J);
          Linalg<TPrecision>::Normalize(diff);
        }
        if(type == CEM_ORTHO_NORMALIZE3){
          Linalg<TPrecision>::Normalize(diff);
        }
        Linalg<TPrecision>::Multiply(J, diff, pDot, true);

        for(unsigned int n=0; n< pDot.N(); n++){
          //o += acos(sqrt(pDot(n)*pDot(n)));
          o += pDot(n) * pDot(n);
        }  
      }
      //o = o/(Z.M()*Z.N())/ M_PI * 180;
      o = o/Z.N();

      pDot.deallocate();
      gfy.deallocate();
      diff.deallocate();
      J.deallocate();

      ml.deallocate();


      return e/Y.N();
    };







    //Gradient descent 
    void gradDescent( unsigned int nIterations, TPrecision scalingZ=1,
        TPrecision scalingBW=1, int verbose=1, int type =
        CEM_ORTHO, bool optimalSigmaX = true){

      bool updatedZ = true;
      for(int i=0; i<nIterations; i++){

        int cr = updateBandwidthX(scalingBW, verbose, type);
        bool updatedBW = cr != 0;          
        //optimize conditional expectation bandwidth for g
        if(type != CEM_MSE && optimalSigmaX){
          int pr = 0;
          while( cr != 0 && pr+cr != 0){ 
            pr = cr;
            cr = updateBandwidthX(scalingBW, verbose, type);
          }
        }
        if(!updatedBW  && !updatedZ){
          break;
        }

        //optimize f
        updatedZ = optimizeZ(1, scalingZ, scalingBW, verbose, type);
      }       

    };






    //f(x_index) - coordinate mapping
    void f(unsigned int index, Vector<TPrecision> &out){
      Linalg<TPrecision>::Zero(out);
      TPrecision sumw = 0;
      for(unsigned int i=0; i < knnY; i++){
        //leave one out
        //if(i==index) continue;
        int nn = KNNY(i, index);
        // TPrecision w = KY(i, index);
        TPrecision w = kernelY.f( KNNYD(i, index) );
        Linalg<TPrecision>::AddScale(out, w, Z, nn, out);
        sumw += w;
      }     
      Linalg<TPrecision>::Scale(out, 1.f/sumw, out);
    };




    //f(x) - coordinate mapping
    void f( DenseVector<TPrecision> &y, Vector<TPrecision> &out){
      Linalg<TPrecision>::Zero(out);
      DenseVector<int> knn(knnY);
      DenseVector<TPrecision> knnd(knnY);
      Geometry<TPrecision>::computeKNN(Y, y, knn, knnd, sl2metric);
      TPrecision sumw = 0;
      for(unsigned int i=0; i < knnY; i++){
        TPrecision w = kernelY.f( knnd(i) );
        Linalg<TPrecision>::AddScale(out, w, Z, knn(i), out);
        sumw += w;
      }     
      Linalg<TPrecision>::Scale(out, 1.f/sumw, out);

      knnd.deallocate();
      knn.deallocate();
    };


    /*
    //---g 0-order

    //g(y_index) - reconstruction mapping
    void g(unsigned int index, Vector<TPrecision> &out){
    Linalg<TPrecision>::Set(out, 0);

    TPrecision sum = 0;

    computefY();


    for(unsigned int i=0; i < knnX; i++){
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




    //g(x) - reconstruction mapping + tangent plane
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




    //get original data
    DenseMatrix<TPrecision> getY(){
      return Y;
    };



    //get Z (parameters for f)
    DenseMatrix<TPrecision> getZ(){
      return Z;
    };


    //coordinate mapping of Ypoints
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



    //Density of f(Y) evaluated at Xt
    DenseVector<TPrecision> densityX( DenseMatrix<TPrecision> &Xt){


      DenseVector<TPrecision> px(Xt.N());

      //compute variances at Xt
      for(unsigned int i=0; i < Xt.N(); i++){
        TPrecision sum = 0;
        for(unsigned int j=0; j < fY.N(); j++){
          TPrecision k = kernelX.f(Xt, i, fY, j);
          sum += k;
        } 
        px(i) = sum/fY.N();
      }

      return px;
    };



    //Compute local variance orthogonal to the manifold. 
    //-Xt manifold parametrization at which to compute variance
    DenseVector<TPrecision> variance( DenseMatrix<TPrecision> &Xt){

      //update fY if necessary

      DenseMatrix<TPrecision> Yp = reconstruct(fY);


      //compute trainig set squared distances
      DenseVector<TPrecision> sdist(Y.N());
      for(unsigned int i=0; i< Y.N(); i++){
        sdist(i) = sl2metric.distance(Y, i, Yp, i);
      }

      DenseVector<TPrecision> var(Xt.N());

      //compute variances at Xt
      for(unsigned int i=0; i < Xt.N(); i++){

        TPrecision sum = 0;
        TPrecision vartmp = 0;
        for(unsigned int j=0; j < fY.N(); j++){
          TPrecision k = kernelX.f(Xt, i, fY, j);
          sum += k;
          vartmp += sdist(j) * k; 
        } 
        var(i) = vartmp / sum;
      }

      sdist.deallocate(); 
      Yp.deallocate();

      return var;
    };




    TPrecision getSigmaX(){
      return kernelX.getKernelParam();
    };



    TPrecision getSigmaY(){
      return kernelY.getKernelParam();
    };



    GaussianKernel<TPrecision> getKernelX(){
      return kernelX;
    };




    GaussianKernel<TPrecision> getKernelY(){
      return kernelY;
    };




    DenseMatrix<TPrecision> geodesic(DenseVector<TPrecision> &xs,
        DenseVector<TPrecision> &xe, double step=0.01, int n=100, int
        nIter=100){

      DenseMatrix<TPrecision> px(Z.M(), n); 
      DenseVector<TPrecision> dx = Linalg<TPrecision>::Subtract(xe, xs);
      DenseVector<TPrecision> cx(Z.M());
      DenseVector<TPrecision> cy(Y.M());

      for(int i=0; i<n; i++){
        Linalg<TPrecision>::AddScale(xs, i/(n-1.0), dx, cx);
        Linalg<TPrecision>::SetColumn(px, i, cx);
      }

      DenseMatrix<TPrecision> pxC = Linalg<TPrecision>::Copy(px);
      DenseMatrix<TPrecision> py = reconstruct(px);

      double stepSize = kernelX.getKernelParam()*step;
      DenseVector<TPrecision> gx(Z.M());
      double prev = -1;
      double cur = lengthPath(py);
      for(int k=0; k<nIter; k++){
        for(int i=1; i<(n-1); i++){
          Linalg<TPrecision>::ExtractColumn(px, i, cx);
          for(int j=0; j<px.M(); j++){
            cx(j) += stepSize;
            g(cx, cy);
            gx(j) = deltaPath(py, i, cy);
            cx(j) -= stepSize;
          }
          Linalg<TPrecision>::Normalize(gx);
          Linalg<TPrecision>::Scale(gx, stepSize, gx);
          Linalg<TPrecision>::AddColumn(px, i, gx, pxC);

          Linalg<TPrecision>::Add(gx, px, i, cx); 
          g(cx, cy);
          Linalg<TPrecision>::SetColumn(py, i, cy);
        }


        reconstruct(pxC, py);
        prev=cur;
        cur = lengthPath(py);
        std::cout << "Prev: " << prev << std::endl;
        std::cout << "Cur: " << cur << std::endl;
        if(prev < cur){
          break;
        }
        else{
          DenseMatrix<TPrecision> tmp = px;
          px = pxC;
          pxC = tmp;
        }

      }

      dx.deallocate();
      cx.deallocate();
      gx.deallocate();
      cy.deallocate();
      pxC.deallocate();
      py.deallocate();



      return px;
    };






    void advect(DenseVector<TPrecision> y, DenseVector<TPrecision> xEnd,
        TPrecision stepSize = 0.000001 ){
      stepSize *= kernelY.getKernelParam();
      DenseVector<TPrecision> xCur(xEnd.N());

      DenseVector<TPrecision> step(xCur.N());
      DenseVector<TPrecision> dir(y.N());

      DenseVector<TPrecision> yR(y.N());
      DenseVector<TPrecision> r(y.N());
      DenseMatrix<TPrecision> J(Y.M(), Z.M());

      TPrecision delta = kernelX.getKernelParam();
      TPrecision prevDelta = delta; 
      while(delta > kernelX.getKernelParam() * 0.01 ){
        f(y, xCur);
        Linalg<TPrecision>::Subtract(xEnd, xCur, step);
        delta = Linalg<TPrecision>::Length(step);
        if(prevDelta < delta){
          stepSize /= 2;
        }
        prevDelta = delta;
        std::cout << "Distance to Target: " << delta << std::endl;
        
        Linalg<TPrecision>::Scale(step, stepSize/delta, step);
        g(xCur, yR, J);
        Linalg<TPrecision>::Multiply(J, step, dir);
        
        Linalg<TPrecision>::Subtract(y, yR, r);

        for(int i=0; i<r.N(); i++){
          dir(i) *= (1+r(i));
        }

        
        
        Linalg<TPrecision>::Add(y, dir, y);
      }

      yR.deallocate();
      xCur.deallocate();
      dir.deallocate();
      J.deallocate();
      step.deallocate();
    };





  private:


    double deltaPath(DenseMatrix<TPrecision> &py, int changed,
        DenseVector<TPrecision> &cy){
      double d1 = l2metric.distance(py, changed-1, py, changed);
      double d2 = l2metric.distance(py, changed+1, py, changed);
      double d1c = l2metric.distance(py, changed-1, cy);
      double d2c = l2metric.distance(py, changed+1, cy);
      return (d1 + d2) - (d1c + d2c);
    };


    double lengthPath(DenseMatrix<TPrecision> &py){
      double d = 0;
      for(int i=1; i<py.N(); i++){
        d += l2metric.distance(py, i-1, py, i);
      }
      return d;
    };



    void init(){
      kernelX = GaussianKernel<TPrecision>( Z.M());
      kernelY = GaussianKernel<TPrecision>( Z.M());
      fY = Linalg<TPrecision>::Copy(Z);

      unsigned int N = Y.N();

      if(knnX <= Z.M()){
        knnX = Z.M()+1;
      }	  
      if(knnX >= N){
        knnX = N;
      }
      if(knnY > N){
        knnY = N;
      }

      //KY = DenseMatrix<TPrecision>(N, N);
      //sumKY = DenseVector<TPrecision>(N);
      //Linalg<TPrecision>::Set(sumKY, 0);
      //KYN = DenseMatrix<TPrecision>(N, N);
      KNNY =  DenseMatrix<int>(knnY, N);
      KNNYD = DenseMatrix<TPrecision>(knnY, N);
      Geometry<TPrecision>::computeKNN(Y, KNNY, KNNYD, sl2metric);

      KNNX = DenseMatrix<int>(knnX, N);
      KNNXD = DenseMatrix<TPrecision>(knnX, N);
      kernelX = GaussianKernel<TPrecision>( Z.M());
      //KX = DenseMatrix<TPrecision>(N, N);
      //sumKX = DenseVector<TPrecision>(N);
      //Linalg<TPrecision>::Set(sumKX, 0);
      //KXN = DenseMatrix<TPrecision>(N, N);


    };     





    //update coordinate mapping and nearest neighbors
    void update(){
      computefY();
      updateKNNX();
      //computeKX();
    };





    //compute cooridnates of trainign data fY = f(Y)
    void computefY(){
      DenseVector<TPrecision> tmp(Z.M());
      for(unsigned int i=0; i<Y.N(); i++){
        f(i, tmp);
        Linalg<TPrecision>::SetColumn(fY, i, tmp);
      }
      tmp.deallocate();
    };






    //update nearest nieghbors of f(Y) for faster gradient computation
    void updateKNNX(){
      unsigned int N = Y.N();
      Geometry<TPrecision>::computeKNN(fY, KNNX, KNNXD, sl2metric);
      sX = 0;
      for(unsigned int i = 0; i < KNNXD.N(); i++){
        sX += sqrt(KNNXD(1, i));
      }
      sX /= KNNXD.N();

    }; 

    /*void computeKX(){

      unsigned int N = Y.N();
      for(unsigned int i=0; i < N; i++){
      sumKX(i) = 0;
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

      };*/





    //Kernel bandwidth for manifold mapping -  non-adaptive isotropic
    void computeKernelX(TPrecision alpha){
      TPrecision sigma = 0;
      for(unsigned int i=0; i < Z.N(); i++){
        sigma += sqrt( KNNXD(knnX-1, i) );
      }
      sigma *= alpha/Z.N();

      kernelX.setKernelParam(sigma);
    };




    //Kernel bandwidth estimate for coordinate mapping non-adaptive 
    //isotropic  based on knnY distances
    void computeKernelY(TPrecision factorY){
      unsigned int N = Y.N();
      TPrecision sigma = 0;
      for(unsigned int i=0; i<N; i++){
        sigma += sqrt( KNNYD(knnY-1, i) ); 
      }
      sigma *= factorY/N;
      kernelY.setKernelParam(sigma);

    };

    /*void computeKY(){
      unsigned int N = Y.N();
      for(unsigned int i=0; i < N; i++){
      sumKY(i) = 0;
      for(unsigned int j=0; j < N; j++){
      KY(j, i) = kernelY.f(Y, j, Y, i);
      sumKY(i) += KY(j, i);
      }
      }

      for(unsigned int i=0; i < KY.M(); i++){
      for(unsigned int j=0; j< KY.N(); j++){
      KYN(i, j) = KY(i, j) / sumKY(j); 
      }
      }

      };*/



    double numGradSigmaX(int type, TPrecision scaling){
      TPrecision sigmaX = kernelX.getKernelParam();
      TPrecision sigmaX2 = sigmaX*scaling;

      TPrecision o = 0;
      TPrecision  e = mse(o, type);

      kernelX.setKernelParam(sigmaX2);
      //computeKX();

      TPrecision og = 0;
      TPrecision eg = mse(og, type);

      double g=0;
      if(type == CEM_MSE){ 
        g = ( eg - e ) / fabs(sigmaX2 - sigmaX);
      }
      else{
        g = ( og - o ) / fabs(sigmaX2 - sigmaX);
      }
      kernelX.setKernelParam(sigmaX);
      //computeKX();

      return -g;

    };

    double numGradSigmaY(int type, TPrecision scaling){
      TPrecision sigmaY = kernelY.getKernelParam();
      TPrecision sigmaY2 = sigmaY*scaling;

      TPrecision o = 0;
      TPrecision  e = mse(o, type);

      kernelY.setKernelParam(sigmaY2);
      //computeKY();
      update();

      TPrecision og = 0;
      TPrecision eg = mse(og, type);

      double g=0;
      if(type == CEM_MSE){ 
        g = ( eg - e ) / fabs(sigmaY2 - sigmaY);
      }
      else{
        g = ( og - o ) / fabs(sigmaY2 - sigmaY);
      }

      kernelY.setKernelParam(sigmaY);
      //computeKY();
      update();

      return -g;

    };

    //numerical gradient at point r of the training data
    //epsilon = finite difference delta
    void numGradX(int r, DenseVector<TPrecision> &gx, TPrecision epsilon, int type){

      TPrecision eg = 0;
      TPrecision og = 0;
      //TPrecision e = mse(r);
      TPrecision o = 0;
      TPrecision  e = mse(r, o, type);
      //vary each coordinate
      for(unsigned int i=0; i<gx.N(); i++){
        Z(i, r) += epsilon;
        numGradLocalUpdate(r);
        //eg = mse(r);
        eg = mse(r, og, type);
        if(type == CEM_MSE){ 
          gx(i) = ( eg - e ) / epsilon;
        }
        else{
          gx(i) = ( og - o ) / epsilon;
        }
        Z(i, r) -= epsilon;
      }

      //update nearest neighbors
      numGradLocalUpdate(r);

    };


    TPrecision mse(int index, TPrecision &o, int type){
      o=0;
      TPrecision e = 0;

      //Jacobian of g(x)
      DenseMatrix<TPrecision> J(Y.M(), Z.M());

      //Temp vars.
      DenseVector<TPrecision> gfy(Y.M());
      DenseVector<TPrecision> diff(Y.M());
      DenseVector<TPrecision> pDot(Z.M());

      int knn = std::min(knnX, knnY);
      //for(int nt=0; nt<1; nt++){
      for(unsigned int i=0; i < knn; i++){
        //  int nn = 0;
        //  if(nt == 0){
        int    nn = KNNX(i, index);
        //  }
        //  else{
        //    nn= KNNY(i, index);
        //   }

        g(nn, gfy, J);
        //e += sl2metric.distance(Y, nn, gfy);

        Linalg<TPrecision>::Subtract(gfy, Y, nn, diff);  
        e += Linalg<TPrecision>::SquaredLength(diff);
        if(type == CEM_ORTHO_NORMALIZE){
          Linalg<TPrecision>::QR_inplace(J);
        }
        if(type == CEM_ORTHO_NORMALIZE2){
          Linalg<TPrecision>::QR_inplace(J);
          Linalg<TPrecision>::Normalize(diff);
        }
        if(type == CEM_ORTHO_NORMALIZE3){
          Linalg<TPrecision>::Normalize(diff);
        }
        Linalg<TPrecision>::Multiply(J, diff, pDot, true);

        for(unsigned int n=0; n< pDot.N(); n++){
          o += pDot(n)*pDot(n);
        }  
      }
      //}
      o = o/knn;

      pDot.deallocate();
      gfy.deallocate();
      diff.deallocate();
      J.deallocate();

      return e/knn;
    };


    //Local mse
    TPrecision mse(int index){
      TPrecision e = 0;
      DenseVector<TPrecision> gfy(Y.M());
      int knn = std::min(knnX, knnY);
      for(unsigned int i=0; i < knn; i++){
        int nn = KNNX(i, index);
        g(nn, gfy);
        e += sl2metric.distance(Y, nn, gfy); 
      }
      gfy.deallocate();
      return e/knn;
    };




    //update local neighborhood
    void numGradLocalUpdate(int r){
      DenseVector<TPrecision> fy(Z.M());
      int knn = std::min(knnX, knnY);
      for(unsigned int k=0; k<knn; k++){
        int nn = KNNX(k, r);
        f(nn, fy);
        Linalg<TPrecision>::SetColumn(fY, nn, fy);

        nn = KNNY(k, r);
        f(nn, fy);
        Linalg<TPrecision>::SetColumn(fY, nn, fy);
      }

      fy.deallocate();
    };





    //Least squares fitting procedures for g

    DenseMatrix<TPrecision> LeastSquares(Vector<TPrecision> &x){
      if(qfit){
        return LeastSquares2(x);
      }
      else{
        return LeastSquares1(x);
      }
    };

    DenseMatrix<TPrecision> LeastSquares(int n){
      if(qfit){
        return LeastSquares2(n);
      }
      else{
        return LeastSquares1(n);
      }
    };





    //locally quardatic regression


    //Least squares for locally linear regression at x
    DenseMatrix<TPrecision> LeastSquares2(Vector<TPrecision> &x){
      //Compute KNN
      DenseVector<int> knn(knnX);
      DenseVector<TPrecision> knnDist(knnX);
      Geometry<TPrecision>::computeKNN(fY, x, knn, knnDist, sl2metric);



      DenseVector<TPrecision> xc(Z.M());
      DenseMatrix<TPrecision> kr(Z.M(), Z.M());

      //Linear system
      DenseMatrix<TPrecision> X1(knnX, Z.M() * (Z.M()+1) +1);
      DenseMatrix<TPrecision> Y1(knnX, Y.M());
      DenseVector<TPrecision> W(knnX);


      //Setup linear system
      for(unsigned int i=0; i < knnX; i++){
        unsigned int nn = knn(i);

        W(i) = kernelX.f(knnDist(i));

        Linalg<TPrecision>::ExtractColumn(fY, nn, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< Z.M(); j++){
          X1(i, jindex) = xc(j);
          ++jindex;
        }

        for(unsigned int j=0; j< kr.M(); j++){
          for(unsigned int k=0; k< kr.N(); k++){
            X1(i, jindex) = kr(j, k);
            ++jindex;
          }
        }
        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i, m) = Y(m, nn);
        }
      }

      Linalg<TPrecision>::OuterProduct(x, x, kr);
      DenseVector<TPrecision> x2(X1.N());
      x2(0) = 1;
      int jindex = 1;
      for(unsigned int j=0; j< Z.M(); j++){
        x2(jindex) = x(j);
        ++jindex;
      }

      for(unsigned int j=0; j< kr.M(); j++){
        for(unsigned int k=0; k< kr.N(); k++){
          x2(jindex) = kr(j, k);
          ++jindex;
        }
      }


      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x2);

      //cleanup
      knn.deallocate();
      knnDist.deallocate();
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      x2.deallocate();

      return sol;
    };





    //Linear least squares for locally linear regression within training data
    DenseMatrix<TPrecision> LeastSquares2(unsigned int n){

      DenseVector<TPrecision> xc(Z.M());
      DenseMatrix<TPrecision> kr(Z.M(), Z.M());

      //Linear system
      DenseMatrix<TPrecision> X1(knnX, Z.M() * (Z.M()+1) +1);
      DenseMatrix<TPrecision> Y1(knnX, Y.M());
      DenseVector<TPrecision> W(knnX);

      //Setup linear system with pre-computed weights
      for(unsigned int i=0; i < knnX; i++){

        //leave one out
        //unsigned int nn = KNNX(i+1, n);
        unsigned int nn = KNNX(i, n);

        //TPrecision w = KX(nn, n);
        W(i) = kernelX.f( KNNXD(i,n) );


        Linalg<TPrecision>::ExtractColumn(fY, nn, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< Z.M(); j++){
          X1(i, jindex) = xc(j);
          ++jindex;
        }

        for(unsigned int j=0; j< kr.M(); j++){
          for(unsigned int k=0; k< kr.N(); k++){
            X1(i, jindex) = kr(j, k);
            ++jindex;
          }
        }
        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i, m) = Y(m, nn);
        }
      }


      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractRow(X1, 0);
      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x);


      //cleanup
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      xc.deallocate();
      kr.deallocate();
      x.deallocate();

      return sol;

    };






    //locally linear regression

    //Least squares for locally linear regression at x
    DenseMatrix<TPrecision> LeastSquares1(Vector<TPrecision> &x){
      //Compute KNN
      DenseVector<int> knn(knnX);
      DenseVector<TPrecision> knnDist(knnX);
      Geometry<TPrecision>::computeKNN(fY, x, knn, knnDist, sl2metric);


      //Linear system
      DenseMatrix<TPrecision> X1(knnX, Z.M()+1);
      DenseMatrix<TPrecision> Y1(knnX, Y.M());
      DenseVector<TPrecision> W(knnX);


      //Setup linear system
      for(unsigned int i=0; i < knnX; i++){
        unsigned int nn = knn(i);
        W(i) = kernelX.f(knnDist(i));
        X1(i, 0) = 1;
        for(unsigned int j=0; j< Z.M(); j++){
          X1(i, j+1) = fY(j, nn);
        }

        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i, m) = Y(m, nn);
        }
      }


      DenseVector<TPrecision> x2(x.N()+1);
      x2(0) = 1;
      for(int i=1; i<x2.N(); i++){
        x2(i) = x(i-1);
      }

      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x2);

      //cleanup
      X1.deallocate();
      Y1.deallocate();
      knn.deallocate();
      knnDist.deallocate();
      W.deallocate();
      x2.deallocate();

      return sol;
    };





    //Linear least squares for locally linear regression within training data
    DenseMatrix<TPrecision> LeastSquares1(unsigned int n){

      //Linear system
      DenseMatrix<TPrecision> X1(knnX, Z.M()+1);
      DenseMatrix<TPrecision> Y1(knnX, Y.M());
      DenseVector<TPrecision> W(knnX);

      //Setup linear system with pre-computed weights
      for(unsigned int i=0; i < knnX; i++){

        //leave one out
        //unsigned int nn = KNNX(i+1, n);
        unsigned int nn = KNNX(i, n);

        //TPrecision w = KX(nn, n);
        W(i) = kernelX.f( KNNXD(i, n) );

        X1(i, 0) = 1;

        for(unsigned int j=0; j< Z.M(); j++){
          X1(i, j+1) = fY(j, nn);
        }

        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i, m) = Y(m, nn);
        }
      }


      //Augment solution to store gradient of the estimate and not the estimated gradient
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractRow(X1, 0);
      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x);

      //cleanup
      x.deallocate();
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();

      return sol;

    };



    DenseMatrix<TPrecision> gEstimate(DenseMatrix<TPrecision> &X1, DenseMatrix<TPrecision> Y1, 
        DenseVector<TPrecision> &W, DenseVector<TPrecision> &x){

      //W:  knnX
      //X1:  knnX x Z
      //Y1:  knnX x Y
      TPrecision sumw = 1;//Linalg<TPrecision>::Sum(W);
      DenseMatrix<TPrecision> A(X1.M(), X1.N());
      for(int i=0; i<knnX; i++){
        Linalg<TPrecision>::ScaleRow(X1, i, W(i)/sumw, A);
      }



      DenseMatrix<TPrecision> Q = Linalg<TPrecision>::Multiply(X1, A, true); // Z x Z
      //Linalg<TPrecision>::Print(Q);
      SymmetricEigensystem<TPrecision> eigs(Q);
      TPrecision thres =  0.00000001*eigs.ew(eigs.ew.N()-1);
      //Linalg<TPrecision>::Print(eigs.ev);
      //exit(0);
      DenseMatrix<TPrecision> tmp = Linalg<TPrecision>::Copy(eigs.ev);
      for(int i=0; i<eigs.ew.N(); i++){
        if(eigs.ew(i) < thres){ 
          Linalg<TPrecision>::SetColumn(tmp, i, 0);
        }
        else{
          Linalg<TPrecision>::ScaleColumn( tmp, i, 1.0/eigs.ew(i) );
        }
      }
      DenseMatrix<TPrecision> Qi = Linalg<TPrecision>::Multiply(tmp, eigs.ev, false, true);
      tmp.deallocate();
      eigs.cleanup();


      //DenseMatrix<TPrecision> Qi = Linalg<TPrecision>::InverseSPD(Q);  // Z x Z
      /*
         std::cout << "X1: " << std::endl;
         Linalg<TPrecision>::Print(X1);
         std::cout << "W: " << std::endl;
         Linalg<TPrecision>::Print(W);
         std::cout << "Qi: " << std::endl;
         Linalg<TPrecision>::Print(Qi);
         std::cout << "Q: " << std::endl;
         Linalg<TPrecision>::Print(Q);
         std::cout << std::endl;
         std::cout << std::endl;
         exit(0); 
       */

      DenseMatrix<TPrecision> dQ(A.N(), A.N()); // Z x Z
      DenseMatrix<TPrecision> dA(A.M(), A.N()); // knnX x Z

      DenseMatrix<TPrecision> AY = Linalg<TPrecision>::Multiply(A, Y1, true); // Z x Y
      DenseMatrix<TPrecision> dAY(A.N(), Y.M());


      DenseMatrix<TPrecision> T1(A.N(), A.N());
      DenseMatrix<TPrecision> T2(A.N(), A.N());

      DenseVector<TPrecision> C1(Y.M());
      DenseVector<TPrecision> C2(Y.M());
      DenseVector<TPrecision> C3(Y.M());


      DenseVector<TPrecision> v(A.N());
      DenseVector<TPrecision> xv = Linalg<TPrecision>::Multiply(Qi, x, true); 

      double sx = kernelX.getKernelParam();
      sx *= sx;

      DenseMatrix<TPrecision> sol( Z.M()+1, Y.M() );
      for(int k=0; k< Z.M(); k++){
        for(int i=0; i<knnX; i++){
          double tmp =  ( X1(i, k+1) - x(k+1) ) / sx;
          Linalg<TPrecision>::ScaleRow(A, i, tmp, dA);
        }
        Linalg<TPrecision>::Multiply(dA, Y1, dAY, true); 





        Linalg<TPrecision>::Multiply(X1, dA, dQ, true);
        Linalg<TPrecision>::Multiply(Qi, dQ, T1);
        Linalg<TPrecision>::Multiply(T1, Qi, T2);


        Linalg<TPrecision>::ExtractRow(Qi, 1+k, v); 
        Linalg<TPrecision>::Multiply(AY, v, C1, true);

        Linalg<TPrecision>::Multiply(T2, x, v, true); 
        Linalg<TPrecision>::Multiply(AY, v, C2, true);



        Linalg<TPrecision>::Multiply(dAY, xv, C3, true);




        for(unsigned int j=0; j< Y.M(); j++){
          sol(1+k, j) = C1(j)  - C2(j) + C3(j);
        }

      }



      Linalg<TPrecision>::Multiply(AY, xv, C1, true);
      for(unsigned int j=0; j< Y.M(); j++){
        sol(0, j) = C1(j);
      }




      A.deallocate();

      Q.deallocate();
      Qi.deallocate();

      dQ.deallocate();
      dA.deallocate();

      AY.deallocate();
      dAY.deallocate();

      T1.deallocate();
      T2.deallocate();

      C1.deallocate();
      C2.deallocate();
      C3.deallocate();

      v.deallocate();
      xv.deallocate();


      return sol;


    };


    /*

       void gradient(unsigned int r, DenseVector<TPrecision> &gx){
       Linalg<TPrecision>::Zero(gx);

       unsigned int YM = Y.M();
       unsigned int ZM = Z.M();
       unsigned int N = Y.N(); 


       DenseVector<TPrecision> W(knnX);
       DenseVector<TPrecision> dW(knnX);
       DenseMatrix<TPrecision> B(ZM+1, knnX);
       DenseMatrix<TPrecision> BdW(ZM+1, knnX);
       DenseMatrix<TPrecision> BW(ZM+1, knnX);
       DenseMatrix<TPrecision> dB(ZM, knnX);
       DenseMatrix<TPrecision> dBW(ZM, knnX);
       DenseMatrix<TPrecision> Ynn(knnX, YM);

       DenseMatrix<TPrecision> Q=(ZM+1, ZM+1);
       DenseMatrix<TPrecision> QI=(ZM+1, ZM+1);
       DenseMatrix<TPrecision> dQ=(ZM+1, ZM+1);

       DenseMatrix<TPrecision> R=(ZM+1, YM);
       DenseMatrix<TPrecision> T1=(ZM+1, YM);
       DenseMatrix<TPrecision> TZ=(ZM+1, YM);

       DenseVector<TPrecision> diff(YM);
       DenseVector<TPrecision> gfy(YM);

       DenseMatrix<TPrecision> dfy(ZM, ZM);
       DenseVector<TPrecision> fy(ZM);

       DenseVector<TPrecision> term1(ZM, YM);
       DenseVector<TPrecision> term2(ZM, YM);
       DenseVector<TPrecision> term3(ZM, YM);

       for(int i = 0; i< knnY; i++){
       unsigned int nny = KNNY(i, r);
       g(nny, gfy);
       Linalg<TPrecision>::Subtract(gfy, Y, yi, diff);

//Setup matrics
for(unsigned int j=0; j < knnX; j++){

unsigned int nnx = KNNX(j, nny);

W(i) = KX(nnx, nny);
dW(i) = 0;
B(i, 0) = 1;
BW(i, 0) = W(i)
for(unsigned int j=0; j< Z.M(); j++){
B(i, j+1) = fY(j, nnx);
BW(i, j+1) = fY(i, nnx) * W(i);
dB(i, j) = KXN(nnx, nny);
dW(i) += dKX[nny, nnx](j) * (KX(r, nny) - KX(r, nnx))
}          
for(unsigned int j=0; j< Z.M(); j++){
dBW(i, j) = dB(i, j) * W(i);
BdW(i, j) = B(i, j) * dW(i);
}

Linalg<TPrecision>::SetRow(Ynn, i, Y, nnx);
}   


Linalg<TPrecision>::ExtractColumn(dB, 0, fy)


Linalg<TPrecision>::Multiply(B, BW, Q, false, true);
Linalg<TPrecision>::InverseSPD(Q, QI);
Linalg<TPrecision>::Multiply(BW, Ynn, R);

//term1
Linalg<TPrecision>::Multiply(QI, R, T1);
Linalg<TPrecision>::Multiply(dfy, T1, term1);

//term2
Linalg<TPrecision>::Multiply(dB, dB2, T2, false, true);
Linalg<TPrecision>::Multiply(B, BdW, T3, false, true);
Linalg<TPrecision>::AddScale(T3, 2, T2, T3);
Linalg<TPrecision>::Multiply(QI, T3, T3);
Linalg<TTrecision>::Multiply(T3, QI, T4);
Linalg<TPrecision>::Multiply(T4, R, T5);
Linalg<TPrecision>::Multiply(T5, fy, term2, true);

}        



};
*/




//Gradient descent 
bool optimizeZ(unsigned int nIterations, TPrecision scalingZ, TPrecision scalingBW, int verbose=1, int type = CEM_ORTHO){

  //TPrecision sX = kernelX.getKernelParam(); 

  TPrecision orthoPrev = 0;
  TPrecision ortho;




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

  bool updated = false;
  for(unsigned int i=0; i<nIterations; i++){
    bool updateBW = updateBandwidthY(scalingBW, verbose, type); 
    TPrecision objPrev = mse(orthoPrev, type);

    if(verbose > 0){
      std::cout << "Mse start: " << objPrev << std::endl;
      std::cout << "Ortho start: " << orthoPrev << std::endl;
    }

    if(verbose > 1){
      std::cout << "Kernel X sigma: " << kernelX.getKernelParam() << std::endl; 
      std::cout << "Kernel Y sigma: " << kernelY.getKernelParam() << std::endl; 
    }

    //compute gradient for each point
    TPrecision maxL = 0;
    for(unsigned int j=0; j < Z.N(); j++){
      //compute gradient
      //gradX(j, gx);
      numGradX(j, gx, sX*0.1, type);


      //Asynchronous updates
      //Linalg<TPrecision>::ColumnAddScale(Z, j, -scaling*sX, gx);


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
      s = scalingZ * sX;
    else{
      s = scalingZ * sX/maxL;
    }     
    if(verbose > 1){
      std::cout << "sX: " << sX << std::endl;
      std::cout << "scaling: " << s << std::endl;
    }


    //Approximate line search with quadratic fit
    DenseMatrix<TPrecision> A(3, 3);
    DenseMatrix<TPrecision> b(3, 1);
    Linalg<TPrecision>::Zero(A);
    double orthoTmp = 0;
    double mseTmp = mse(orthoTmp, type);
    if(type == CEM_MSE){
      b(0, 0) = mseTmp;
    }
    else{
      b(0, 0) = orthoTmp;
    }

    Linalg<TPrecision>::AddScale(Z, -1*s, sync, Ztmp);
    Zswap = Z;
    Z = Ztmp;
    Ztmp = Zswap;
    update();
    mseTmp = mse(orthoTmp, type);
    if(type == CEM_MSE){
      b(1, 0) = mseTmp;
    }
    else{
      b(1, 0) = orthoTmp;
    }


    Linalg<TPrecision>::AddScale(Zswap, -2*s, sync, Z);
    update();
    mseTmp = mse(orthoTmp, type);
    if(type == CEM_MSE){
      b(2, 0) = mseTmp;
    }
    else{
      b(2, 0) = orthoTmp;
    }


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

    bool stop = false;
    //do step
    if( q(0, 0) > 0){
      TPrecision h = -q(1, 0)/(2*q(0, 0));
      if(h < -2*s){
        h = -2*s;
      }
      else if( h > 1){
        h = s;
      }
      Linalg<TPrecision>::AddScale(Ztmp, h, sync, Z);        

    }
    else if( b(0,0) > b(1, 0) ){
      //do nothing step to -10*s          
    }
    else{
      //stop gradient descent - no step
      //stop = true;
      //Linalg<TPrecision>::AddScale(Ztmp, -s, sync, Z);
    }

    A.deallocate();
    b.deallocate();
    q.deallocate();





    update();

    //bool updatedBW = updateBandwidth(scalingBW, verbose, type); 
    //stop = stop && !updatedBW;

    TPrecision obj = mse(ortho, type); 
    if(verbose > 0){
      std::cout << "Iteration: " << i << std::endl;
      std::cout << "MSE: " <<  obj << std::endl;     
      std::cout << "Ortho: " <<  ortho << std::endl << std::endl;
    }

    if(!stop){ 
      if(type == CEM_MSE){ 
        stop = objPrev <= obj;
      }
      else{
        stop = orthoPrev <= ortho;
      }
    }

    if(stop){          
      Zswap = Ztmp;
      Ztmp = Z;
      Z = Zswap;
      update();
      if(!updateBW)
        break;
    }

    updated=true; 
    objPrev = obj;      
    orthoPrev = ortho;

  }


  //cleanup 
  sync.deallocate();
  gx.deallocate();
  Ztmp.deallocate();

  return updated;
};


int updateBandwidthX(TPrecision scaling, int verbose, int type){      

  int updated = 0;

  TPrecision gradSX1 = numGradSigmaX(type, (1+0.01));
  TPrecision gradSX2 = numGradSigmaX(type, (1-0.01));

  if(verbose > 1){
    std::cout << "grad sigmaX: " << gradSX1 << "/" << gradSX2 << std::endl;
  }

  if(gradSX1 > gradSX2){
    if(gradSX1 > 0){
      kernelX.setKernelParam((1+0.1*scaling)*kernelX.getKernelParam());
      updated = 1;
    }

  }
  else{
    if(gradSX2 > 0){
      kernelX.setKernelParam((1-0.1*scaling)*kernelX.getKernelParam());
      updated = -1;
    }

  }

  return updated;
};



int updateBandwidthY(TPrecision scaling, int verbose, int type){      

  int updated = 0;

  TPrecision gradSY1 = numGradSigmaY(type, (1+0.01));
  TPrecision gradSY2 = numGradSigmaY(type, (1-0.01));

  if(verbose > 1){
    std::cout << "grad sigmaY: " << gradSY1 << "/" << gradSY2 << std::endl;
  }

  if(gradSY1 > gradSY2){
    if(gradSY1 > 0){
      kernelY.setKernelParam((1+0.1*scaling)*kernelY.getKernelParam());
      updated = 1;
    }

  }
  else{
    if(gradSY2 > 0){
      kernelY.setKernelParam((1-0.1*scaling)*kernelY.getKernelParam());
      updated = -1;
    }

  }


  if(updated != 0){
    //computeKY();
    update();
  }	
  return updated;
};




}; 


#endif

