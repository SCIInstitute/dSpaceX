#ifndef FASTCEM_H
#define FASTCEM_H


#include "Geometry.h"
#include "Random.h"
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

#ifndef R_PACKAGE
#define myprintf printf
#else
#define myprintf Rprintf
#endif

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

enum Risk {MSE, ORTHO, ORTHO_NORM_1, ORTHO_NORM_2, ORTHO_NORM_3,
  ORTHO_NORM_4};

enum Penalty {NONE, UNIT, DENSITY};

template <typename TPrecision>
struct Objective{
  
  TPrecision mse;
  TPrecision ortho;
  TPrecision penalty;

  TPrecision total;
};



//Conditional Expectation Manifolds
template <typename TPrecision>
class FastCEM{

  private:    
    FortranLinalg::DenseMatrix<TPrecision> Y;
    //FortranLinalg::DenseMatrix<TPrecision> fY;

    FortranLinalg::DenseMatrix<TPrecision> lambdaY;
    FortranLinalg::DenseMatrix<TPrecision> lambdaZ;
    FortranLinalg::DenseMatrix<TPrecision> lambdafY;
    
    unsigned int knnX;
    unsigned int knnY;
    TPrecision sX;

    EuclideanMetric<TPrecision> l2metric;
    SquaredEuclideanMetric<TPrecision> sl2metric;

   // FortranLinalg::DenseMatrix<int> KNNX;
   // FortranLinalg::DenseMatrix<TPrecision> KNNXD;

    FortranLinalg::DenseMatrix<int> KNNY;
    FortranLinalg::DenseMatrix<TPrecision> KNNYD;
    FortranLinalg::DenseMatrix<TPrecision> KY;


    GaussianKernel<TPrecision> kernelX;
    GaussianKernel<TPrecision> kernelY;
    GaussianKernel<TPrecision> kernelZ;


    bool qfit;


    Random<TPrecision> rand;

    FortranLinalg::DenseVector<TPrecision> Sigma;
    FortranLinalg::DenseVector<TPrecision> pEstimate;


  public:


    static Risk toRisk(int rType){
      Risk risk = ORTHO;
      switch(rType){
        case 0:
          risk = MSE;
          break;
        case 1:
          risk = ORTHO;
          break;
        case 2:
          risk = ORTHO_NORM_1;
          break;
        case 3:
          risk = ORTHO_NORM_2;
          break;
        case 4:
          risk = ORTHO_NORM_3;
          break;
        case 5:
          risk = ORTHO_NORM_4;
          break;
      }
      return risk;
    };



    static Penalty toPenalty(int pType){
      Penalty penalty = NONE;
      if(pType == 1){
        penalty = UNIT;
      }
      else if(pType == 2){
        return DENSITY;
      }
      return penalty;
    };




    virtual void cleanup(){      
      partialCleanup();
      Y.deallocate();
      lambdaZ.deallocate();
      lambdaY.deallocate();
      lambdafY.deallocate();

      
    };



    void partialCleanup(){  
      //KNNX.deallocate();
      //KNNXD.deallocate();	   
      
      KNNY.deallocate();
      KNNYD.deallocate();
      KY.deallocate();
      //fY.deallocate();
    };


    //Create Condtional Expectation Manifold 
    FastCEM(FortranLinalg::DenseMatrix<TPrecision> Ydata,
        FortranLinalg::DenseMatrix<TPrecision> Yinit, int nnY,
        FortranLinalg::DenseMatrix<TPrecision> Zinit, int nnX,
        TPrecision sigmaZ, TPrecision sigmaY, TPrecision sigmaX, bool
        sigmaAsFactor = true, bool quadratic = false ) : Y(Ydata),
        lambdaY(Yinit), lambdaZ(Zinit), knnX(nnX), knnY(nnY), qfit(quadratic){

      init();
      if( sigmaAsFactor ){
        computeKernelY(sigmaY);
        computeKernelZ(sigmaZ);
        update();
        computeKernelX2(sigmaX);
      }
      else{
        kernelY.setKernelParam(sigmaY);
        kernelZ.setKernelParam(sigmaZ);
        kernelX.setKernelParam(sigmaX);

        updateKY();
        update();
        //updateKNNX();
      }

    };




    //evaluate mean squared projection distance
    TPrecision mse(){
      using namespace FortranLinalg;
      TPrecision e = 0;
      DenseVector<TPrecision> gfy(Y.M());
      for(unsigned int i=0; i < Y.N(); i++){
        g(i, gfy);
        e += sl2metric.distance(Y, i, gfy);
      }
      gfy.deallocate();
      return e/Y.N();
    }







    Objective<TPrecision> objective(Risk risk, Penalty penalty){
      using namespace FortranLinalg;
      static DenseVector<int> dummy;
      return objective(risk, penalty, dummy);
    };






    //evalue objective function
    Objective<TPrecision> objective(Risk risk, Penalty penalty, FortranLinalg::DenseVector<int> points){
      using namespace FortranLinalg;
      Objective<TPrecision> obj;

      obj.ortho = 0;
      obj.mse = 0;
      obj.penalty = 0;      

      //Jacobian of g(x)
      DenseMatrix<TPrecision> J(Y.M(), lambdaZ.M());

      //Temp vars.
      DenseVector<TPrecision> y(Y.M());
      DenseVector<TPrecision> x(lambdaZ.M());
      DenseVector<TPrecision> gfy(Y.M());
      DenseVector<TPrecision> diff(Y.M());
      DenseVector<TPrecision> pDot(lambdaZ.M());

      //Length of Jacobian / 
      DenseMatrix<TPrecision> J2( J.N(), J.N() );
      Linalg<TPrecision>::Zero(J2);

      unsigned int nPoints = points.N();
      if(nPoints == 0){
        nPoints = Y.N(); 
      }
      for(unsigned int is=0; is < nPoints; is++){
        int i = is;
        if(nPoints < Y.N() ){
          i = points(is);
        }

        Linalg<TPrecision>::ExtractColumn(Y, i, y);
        f(y, x);
        g(x, gfy, J);

        Linalg<TPrecision>::Subtract(gfy, Y, i, diff);  
        obj.mse += Linalg<TPrecision>::SquaredLength(diff);

        //arc length penalty
        if(penalty == UNIT){
          DenseMatrix<TPrecision> Jtmp = Linalg<TPrecision>::Multiply(J, J, true);
          for(unsigned int k = 0; k<J2.M(); k++){
            for(unsigned int l = 0; l<J2.N(); l++){
              TPrecision tmp = Jtmp(k, l);
              if(k == l){
                tmp -= 1;
              }
              J2(k, l) += tmp*tmp;
            }
          }
        }


        //normalize length of Jacobian and residual as needed by the risk
        if(risk == ORTHO_NORM_1){
          Linalg<TPrecision>::QR_inplace(J);
        }
        if(risk == ORTHO_NORM_2 || risk == MSE){
          Linalg<TPrecision>::QR_inplace(J);
          Linalg<TPrecision>::Normalize(diff);
        }
        if(risk == ORTHO_NORM_3){
          Linalg<TPrecision>::Normalize(diff);
        }

        //measure orthogonality under the given normalization
        Linalg<TPrecision>::Multiply(J, diff, pDot, true);

        for(unsigned int n=0; n< pDot.N(); n++){
          obj.ortho += pDot(n) * pDot(n);
        }  
      }
      
      if(penalty == UNIT){
        obj.penalty = Linalg<TPrecision>::Sum(J2);
      }
      

      pDot.deallocate();
      gfy.deallocate();
      diff.deallocate();
      J.deallocate();
      J2.deallocate();
      y.deallocate();
      x.deallocate();

      obj.ortho /= nPoints;
      obj.penalty /= nPoints;
      obj.mse /= nPoints;

      if(risk == MSE){
        obj.total = obj.mse;
      }
      else{
        obj.total = obj.ortho;
      }

      if(penalty == UNIT){
        obj.total += obj.penalty;
      }
      
      return obj;
    };







    //Gradient descent 
    void gradDescent(int nIterations, int nPoints, TPrecision
        scalingZ=0.5, TPrecision scalingBW=0.1, int verbose=1, Risk risk=
        ORTHO, Penalty penalty = NONE, bool optimalSigmaX = true){
      using namespace FortranLinalg;

      if( nPoints > Y.N()){
        nPoints = Y.N();
      }

      bool updatedZ = true;
      for(int i=0; i<nIterations; i++){

        int cr = updateBandwidthX(scalingBW, verbose, risk, penalty, nPoints);
        bool updatedBW = cr != 0;          
        
        //optimize conditional expectation bandwidth for g
        if(risk != CEM_MSE && optimalSigmaX){
          int pr = 0;
          while( cr != 0 && pr+cr != 0){ 
            pr = cr;
            cr = updateBandwidthX(scalingBW, verbose, risk, penalty, nPoints);
          }
        }
        
        if(!updatedBW  && !updatedZ){
          break;
        }

        //optimize f
        updatedZ = optimizeZ(1, nPoints, scalingZ, scalingBW, verbose, risk,
            penalty); 
     
      }       

    };


/*
    //f(x_index) - coordinate mapping
    void f(unsigned int index, FortranLinalg::Vector<TPrecision> &out){
      using namespace FortranLinalg;
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractColumn(Y, index);
      f(x, out);
      x.deallocate();
      
      //TPrecision sumw = 0;
      //Linalg<TPrecision>::Zero(out);
      //for(unsigned int i=0; i <= knnY; i++){
      //  TPrecision w = KY(i, index) * kernelZ.f( sl2metric.distance(lambdaZ, KNNY(0, index), lambdaZ, KNNY(i, index)) );
      //  Linalg<TPrecision>::AddScale(out, w, lambdaZ, KNNY(i, index), out);
      //  sumw += w;
      //}     
      //Linalg<TPrecision>::Scale(out, 1.f/sumw, out);
   
    };
*/

    //f(x) - coordinate mapping
    void f( FortranLinalg::DenseVector<TPrecision> &y, FortranLinalg::Vector<TPrecision> &out){
      using namespace FortranLinalg;
      Linalg<TPrecision>::Zero(out);
      TPrecision sumw = 0;
            
      DenseVector<int> knn =  DenseVector<int>(knnY+1);
      DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnY+1);
      Geometry<TPrecision>::computeKNN(lambdaY, y, knn, knnD, sl2metric);


      for(unsigned int i=0; i < knnD.N(); i++){
        TPrecision w = kernelY.f( knnD(i) ) * kernelZ.f( sl2metric.distance(lambdaZ, knn(0), lambdaZ, knn(i)) );
        Linalg<TPrecision>::AddScale(out, w, lambdaZ, knn(i), out);
        sumw += w;
      }     
      Linalg<TPrecision>::Scale(out, 1.f/sumw, out);

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



/*
    //------g 1-order
    //g(x_index) - reconstruction mapping
    void g(unsigned int index, FortranLinalg::Vector<TPrecision> &out){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> sol = LeastSquares(index);

      for(unsigned int i=0; i<Y.M(); i++){
        out(i) = sol(0, i);
      }

      sol.deallocate();
    };  
*/


/*
    //g(x_index) - reconstruction mapping plus tangent plane
    void g(unsigned int index, FortranLinalg::Vector<TPrecision> &out, FortranLinalg::Matrix<TPrecision> &J){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> sol = LeastSquares(index);

      for(unsigned int i=0; i<Y.M(); i++){
        out(i) = sol(0, i);
      }
      for(unsigned int i=0; i< lambdaZ.M(); i++){
        for(unsigned int j=0; j< Y.M(); j++){
          J(j, i) = sol(1+i, j);
        }
      }

      sol.deallocate();
    };

*/


    //g(x) - reconstruction mapping
    void g( FortranLinalg::Vector<TPrecision> &x, FortranLinalg::Vector<TPrecision> &out){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> sol = LeastSquares(x);

      for(unsigned int i=0; i<Y.M(); i++){
        out(i) = sol(0, i);
      }

      sol.deallocate();
    };   




    //g(x) - reconstruction mapping + tangent plane
    void g( FortranLinalg::Vector<TPrecision> &x, FortranLinalg::Vector<TPrecision> &out, FortranLinalg::Matrix<TPrecision> &J){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> sol = LeastSquares(x);
      for(unsigned int i=0; i<Y.M(); i++){
        out(i) = sol(0, i);
      }     
      for(unsigned int i=0; i< lambdaZ.M(); i++){
        for(unsigned int j=0; j< Y.M(); j++){
          J(j, i) = sol(1+i, j);
        }
      }

      sol.deallocate();
    };




    //get original data
    FortranLinalg::DenseMatrix<TPrecision> getY(){
      return Y;
    };



    //get Z (parameters for f)
    FortranLinalg::DenseMatrix<TPrecision> getZ(){
      return lambdaZ;
    };


    //coordinate mapping of Ypoints
    FortranLinalg::DenseMatrix<TPrecision> parametrize(FortranLinalg::DenseMatrix<TPrecision> &Ypoints){
      using namespace FortranLinalg;

      DenseMatrix<TPrecision> proj(lambdaZ.M(), Ypoints.N());
      parametrize(Ypoints, proj);

      return proj;
    };




    //
    void parametrize(FortranLinalg::DenseMatrix<TPrecision> &Ypoints, FortranLinalg::DenseMatrix<TPrecision> &proj){
      using namespace FortranLinalg;

      DenseVector<TPrecision> tmp(Y.M()); 
      DenseVector<TPrecision> xp(lambdaZ.M()); 

      for(unsigned int i=0; i < Ypoints.N(); i++){
        Linalg<TPrecision>::ExtractColumn(Ypoints, i, tmp);
        f(tmp, xp);
        Linalg<TPrecision>::SetColumn(proj, i, xp);
      }
      xp.deallocate();
      tmp.deallocate();
    };




    FortranLinalg::DenseMatrix<TPrecision> parametrize(){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> res = parametrize(Y);
      return res;
    };



    FortranLinalg::DenseMatrix<TPrecision> reconstruct(FortranLinalg::DenseMatrix<TPrecision> &Xpoints){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> proj(Y.M(), Xpoints.N());
      reconstruct(Xpoints, proj);     
      return proj;
    };





    void reconstruct(FortranLinalg::DenseMatrix<TPrecision> &Xpoints, FortranLinalg::DenseMatrix<TPrecision> &proj){
      using namespace FortranLinalg;
      DenseVector<TPrecision> tmp(lambdaZ.M()); 
      DenseVector<TPrecision> yp(Y.M()); 
      for(unsigned int i=0; i < Xpoints.N(); i++){
        Linalg<TPrecision>::ExtractColumn(Xpoints, i, tmp);
        g(tmp, yp);
        Linalg<TPrecision>::SetColumn(proj, i, yp);
      }
      yp.deallocate();
      tmp.deallocate();
    };   


    FortranLinalg::DenseMatrix<TPrecision> reconstruct(){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> proj(Y.M(), Y.N());
      DenseVector<TPrecision> yp(Y.M()); 
      DenseVector<TPrecision> y(Y.M()); 
      DenseVector<TPrecision> x(lambdaZ.M()); 
      for(unsigned int i=0; i < Y.N(); i++){
        Linalg<TPrecision>::ExtractColumn(Y, i, y);
        f(y,x);
        g(x, yp);
        Linalg<TPrecision>::SetColumn(proj, i, yp);
      }
      yp.deallocate();
      y.deallocate();
      x.deallocate();
      return proj;
    };





    TPrecision getSigmaX(){
      return kernelX.getKernelParam();
    };

    TPrecision getSigmaY(){
      return kernelY.getKernelParam();
    };


    TPrecision getSigmaZ(){
      return kernelZ.getKernelParam();
    };



    GaussianKernel<TPrecision> getKernelX(){
      return kernelX;
    };




    GaussianKernel<TPrecision> getKernelY(){
      return kernelY;
    };


    double geodesic(FortranLinalg::DenseMatrix<TPrecision> &px, double step =
        0.01, int nIter = 100){
      using namespace FortranLinalg;
     
     int n = px.N(); 
      DenseVector<TPrecision> cx(lambdaZ.M());
      DenseVector<TPrecision> cy(lambdaY.M());


      //DenseMatrix<TPrecision> pxC = Linalg<TPrecision>::Copy(px);
      DenseMatrix<TPrecision> py = reconstruct(px);

      double stepSize = kernelX.getKernelParam()*step;
      DenseMatrix<TPrecision> gx(lambdaZ.M(), px.N() );
      Linalg<TPrecision>::Zero(gx);
      double prev = -1;
      double cur = lengthPath(py);
      for(int k=0; k<nIter; k++){
        for(int i=1; i<(n-1); i++){
          Linalg<TPrecision>::ExtractColumn(px, i, cx);
          for(unsigned int j=0; j<px.M(); j++){
            cx(j) += stepSize;
            g(cx, cy);
            gx(j, i) = deltaPath(py, i, cy);
            cx(j) -= stepSize;
          }
          //Linalg<TPrecision>::Normalize(gx);
          //Linalg<TPrecision>::Scale(gx, stepSize, gx);
          //Linalg<TPrecision>::AddColumn(px, i, gx, pxC);
          
          //Linalg<TPrecision>::AddColumn(px, i, gx, pxC);

          //Linalg<TPrecision>::Add(gx, px, i, cx); 
          //g(cx, cy);
          //Linalg<TPrecision>::SetColumn(py, i, cy);
        }

        Linalg<TPrecision>::NormalizeColumns(gx);
        Linalg<TPrecision>::Scale(gx, stepSize, gx);
        Linalg<TPrecision>::Add(px, gx, px);

        reconstruct(px, py);
        prev=cur;
        cur = lengthPath(py);
        //myprintf( "Prev: " << prev );
        //myprintf( "Cur: " << cur );
        if(prev <= cur){
          break;
        }
       // else{
       //   DenseMatrix<TPrecision> tmp = px;
       //   px = pxC;
       //   pxC = tmp;
       // }

      }

      cur = lengthPath(py);

      cx.deallocate();
      gx.deallocate();
      cy.deallocate();
      //pxC.deallocate();
      py.deallocate();

      return cur;

    };


    FortranLinalg::DenseMatrix<TPrecision> geodesic(FortranLinalg::DenseVector<TPrecision> &xs,
        FortranLinalg::DenseVector<TPrecision> &xe, double step=0.01, int n=100, int
        nIter=100){
      using namespace FortranLinalg;

      DenseMatrix<TPrecision> px(lambdaZ.M(), n); 
      DenseVector<TPrecision> dx = Linalg<TPrecision>::Subtract(xe, xs);
      DenseVector<TPrecision> cx(lambdaZ.M());

      for(int i=0; i<n; i++){
        Linalg<TPrecision>::AddScale(xs, i/(n-1.0), dx, cx);
        Linalg<TPrecision>::SetColumn(px, i, cx);
      }

      geodesic(px, step, nIter); 

      dx.deallocate();
      cx.deallocate();

      return px;
    };


    FortranLinalg::DenseMatrix<TPrecision> geodesic(FortranLinalg::DenseVector<TPrecision> &xs,
        FortranLinalg::DenseVector<TPrecision> &xe, double step=0.01, double epsilon = 0.001){
      using namespace FortranLinalg;

      DenseMatrix<TPrecision> px = geodesic(xs, xe, step, 3, 100);
      TPrecision prev = lengthPath(px);

      geodesic(px, step, 100); 


      return px;
    };



   //Mean curvature vector and Gauss curvature, - see "Curvature Computations for n-manifolds in
   //R^{n+m} and solution to an open problem proposed by R. Goldman", Qin Zhang
   //and Guoliang Xu 
   FortranLinalg::DenseVector<TPrecision> curvature(FortranLinalg::DenseVector<TPrecision> &x, TPrecision
       &meanC, TPrecision &gaussC, TPrecision &detg, TPrecision &detB,
       TPrecision &frob){
      using namespace FortranLinalg;
     
     //Block hessian matrix
     DenseMatrix<TPrecision> T[lambdaZ.M()];
     for(unsigned int i=0; i<lambdaZ.M(); i++){
       T[i] = DenseMatrix<TPrecision>(Y.M(), lambdaZ.M());     
       Linalg<TPrecision>::Zero(T[i]);
     }
        
     //Jacobian of g(x)
     DenseMatrix<TPrecision> J(Y.M(), lambdaZ.M());
     Linalg<TPrecision>::Zero(J); 
     
     //Kernel values
     DenseVector<TPrecision> k(knnX+1);
     //Kernel gradients
     DenseMatrix<TPrecision> kg(lambdaZ.M(), knnX+1);
     //Kernel hessians
     DenseMatrix<TPrecision> kh[knnX+1];
           
    // DenseVector<int> knn =  DenseVector<int>(knnX+1);
    // DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
    // Geometry<TPrecision>::computeKNN(lambdafY, x, knn, knnD, sl2metric);

     //Precomputed sums of kernel value
     TPrecision sumk = 0;
     DenseVector<TPrecision> sumkg(lambdaZ.M());
     Linalg<TPrecision>::Zero(sumkg);
     DenseMatrix<TPrecision> sumkh(lambdaZ.M(), lambdaZ.M());
     Linalg<TPrecision>::Zero(sumkh);

     //Temp vars.
     DenseVector<TPrecision> gtmp(lambdaZ.M());
     DenseVector<TPrecision> fy(lambdaZ.M());

     //update fY if necessary

     //Precompte kernel values and first and second order derivatives 
     //(gradient and hessian of kernel function)
     
     for(unsigned int i=0; i < lambdafY.N(); i++){

       Linalg<TPrecision>::ExtractColumn(lambdafY, i, fy);

       k(i) = kernelX.gradf(x, fy, gtmp);
       kh[i] = kernelX.hessian(x, fy);
       sumk += k(i);

        for(unsigned int k=0; k < lambdaZ.M(); k++){
          kg(k, i) = gtmp(k);
          sumkg(k) += kg(k, i);
          for( unsigned int l=0; l<lambdaZ.M(); l++){
            sumkh(k, l) += kh[i](k, l);
          }
        }
      }

      TPrecision sumk2 = sumk*sumk;
      TPrecision sumk4 = sumk2 * sumk2;

      //Build T - block hessian matrix and J - Jacobian
      for(unsigned int n = 0; n < lambdafY.N(); n++){
        for(unsigned int i=0; i<lambdaZ.M(); i++){
          
          //T
          for(unsigned int j = 0; j<lambdaZ.M(); j++){
            //First order and second order kernel derivatives dx_i, dx_j
            TPrecision c  = kh[n](i, j) * sumk      +  kg(j, n) * sumkg(i)   ;
                       c -= kg(i, n)    * sumkg(j)  +  k(n)     * sumkh(i, j);
                       c *= sumk2;
                       c -= (kg(j, n) * sumk        -  k(n)     * sumkg(j)) * 2.0 * sumk * sumkg(i);
            //times dependant variable y
            for(unsigned int r=0; r < Y.M() ; r++){
              T[i](r, j) +=  c * lambdaY(r, n );
            }
          }
          
          //J
          TPrecision c  = kg(i, n) * sumk      -  k(n) * sumkg(i)   ;
          for(unsigned int r=0; r< lambdaY.M(); r++){
            J(r, i) += c * lambdaY(r, n );
          }
        }
      }    
   
     Linalg<TPrecision>::Scale(J, 1.0/sumk2, J);

     for(unsigned int i=0; i<lambdaZ.M(); i++){
      Linalg<TPrecision>::Scale(T[i], 1.0/sumk4, T[i]);
     }


     //Measure tensor / first fundamental form
     DenseMatrix<TPrecision> g = Linalg<TPrecision>::Multiply(J, J, true, false);
     DenseMatrix<TPrecision> gInv = Linalg<TPrecision>::Inverse(g);
    
     
     //Curvatures
     
     //Normal projection
     DenseMatrix<TPrecision> Q1 = Linalg<TPrecision>::Multiply(gInv, J, false, true);
     DenseMatrix<TPrecision> Q = Linalg<TPrecision>::Multiply(J, Q1);
     Linalg<TPrecision>::Scale(Q, -1.0, Q);

     for(unsigned int i = 0; i < Q.M(); i++){
       Q(i, i) = 1 + Q(i, i);
     }
     
     //Block trace of TgInv
     DenseMatrix<TPrecision> TgInv[lambdaZ.M()];
     DenseMatrix<TPrecision> QTgInv[lambdaZ.M()];
     for(unsigned int i=0; i < lambdaZ.M(); i++){
      TgInv[i] = Linalg<TPrecision>::Multiply(T[i], gInv);
      QTgInv[i] = Linalg<TPrecision>::Multiply(Q, TgInv[i]);
     }
     DenseVector<TPrecision> H1(Y.M());
     Linalg<TPrecision>::Zero(H1);
     frob = 0;
     for(unsigned int i = 0; i < lambdaZ.M(); i++){
        for(unsigned int r = 0; r < lambdaY.M(); r++){
           H1(r) += TgInv[i](r, i);
           for(unsigned int q=0; q< lambdaZ.M(); q++){
            frob += QTgInv[i](r, q) * QTgInv[i](r, q); 
           }
        }
     }
     frob = sqrt(frob);
     
     DenseVector<TPrecision> H = Linalg<TPrecision>::Multiply(Q, H1);
     Linalg<TPrecision>::Scale(H, 1.0/ lambdaZ.M(), H);
     
     //Mean curvature
     meanC = Linalg<TPrecision>::Length(H);
     
     
     //Gauss curvature
     Linalg<TPrecision>::Scale(H, 1.0/meanC, H);
    
     DenseMatrix<TPrecision> B(lambdaZ.M(), lambdaZ.M());
     for(unsigned int i=0; i<B.M(); i++){
        for(unsigned int j=0; j< B.N(); j++){
          TPrecision dot = 0;
          for(unsigned int r=0; r<lambdaY.M(); r++){
            dot += T[i](r, j) * H(r);
          }
          B(i, j) = dot;
        }
     }
     
     detB = Linalg<TPrecision>::Det(B);
     detg = Linalg<TPrecision>::Det(g);
     
     gaussC =  detB / detg;


     //cleanup
     Q.deallocate();
     Q1.deallocate();
     for(unsigned int i=0; i < lambdaZ.M(); i++){
      TgInv[i].deallocate();
      QTgInv[i].deallocate();
      T[i].deallocate();
     }
     g.deallocate();
     gInv.deallocate();
     H1.deallocate();
     B.deallocate();
     J.deallocate();
     gtmp.deallocate();
     k.deallocate();
     kg.deallocate();
     for(unsigned int i=0; i<=knnX; i++){
        kh[i].deallocate();
     }
     sumkg.deallocate();
     sumkh.deallocate();
  
      
     return H;

   };



















  private:



    void init(){
      using namespace FortranLinalg;
      kernelX = GaussianKernel<TPrecision>( lambdaZ.M());
      kernelY = GaussianKernel<TPrecision>( lambdaZ.M());
      kernelZ = GaussianKernel<TPrecision>( lambdaZ.M());
      //fY = DenseMatrix<TPrecision>(lambdaZ.M(), Y.N());
      lambdafY = DenseMatrix<TPrecision>(lambdaZ.M(), lambdaY.N());

      if(knnX <= lambdaZ.M()){
        knnX = lambdaZ.M()+1;
      }	  
      if(knnX >= lambdaY.N()){
        knnX = lambdaY.N()-1;
      }

      if(knnY <= lambdaZ.M()){
        knnY = lambdaZ.M()+1;
      }	  
      if(knnY >= lambdaY.N()){
        knnY = lambdaY.N()-1;
      }

      //KNNX = DenseMatrix<int>(knnX+1, lambdaZ.N() );
      //KNNXD = DenseMatrix<TPrecision>(knnX+1, lambdaZ.N() );

      KNNY = DenseMatrix<int>(knnY+1, lambdaZ.N() );
      KNNYD = DenseMatrix<TPrecision>(knnY+1, lambdaZ.N() );
      Geometry<TPrecision>::computeKNN(lambdaY, KNNY, KNNYD, sl2metric);
      
      KY = DenseMatrix<TPrecision>(knnY+1, lambdaZ.N() );
//      kernelX = GaussianKernel<TPrecision>( lambdaZ.M());
    };     



    //update coordinate mapping and nearest neighbors
    void update(){
      //computefY();
      computeLambdafY();
      //updateKNNX();

    };



  /*  
    //compute cooridnates of trainign data fY = f(Y)
    void computefY(){
      using namespace FortranLinalg;

      DenseVector<TPrecision> tmp(lambdaZ.M());
      for(unsigned int i=0; i<Y.N(); i++){
        f(i, tmp);
        Linalg<TPrecision>::SetColumn(fY, i, tmp);
      }
      tmp.deallocate();
    };

*/
    //compute coordinates of training data fY = f(Y)
    void computeLambdafY(){
      using namespace FortranLinalg;
      DenseVector<TPrecision> tmp(lambdaZ.M());
      DenseVector<TPrecision> y(lambdaY.M());
      for(unsigned int i=0; i<lambdaY.N(); i++){
        Linalg<TPrecision>::ExtractColumn(lambdaY, i, y);
        f(y, tmp);
        Linalg<TPrecision>::SetColumn(lambdafY, i, tmp);
      }
      y.deallocate();
      tmp.deallocate();
    };



    /*
    //update nearest nieghbors of f(Y) for faster gradient computation
    void updateKNNX(){
      unsigned int N = Y.N();
      Geometry<TPrecision>::computeKNN(lambdafY, KNNX, KNNXD, sl2metric);
      //sX = 0;
      //for(unsigned int i = 0; i < KNNXD.N(); i++){
      //  sX += sqrt(KNNXD(1, i));
      //}
      //sX /= KNNXD.N();    
      //
      //
    }
*/



/*
    //Kernel bandwidth for manifold mapping -  non-adaptive isotropic
    void computeKernelX(TPrecision alpha){
      TPrecision sigma = 0;
      for(unsigned int i=0; i < KNNXD.N(); i++){
        sigma += sqrt( KNNXD(knnX-1, i) );
      }
      sigma *= alpha/KNNXD.N();

      kernelX.setKernelParam(sigma);
    };
*/



    //Kernel bandwidth estimate for coordinate mapping non-adaptive 
    //isotropic  based on knnY distances
    void computeKernelX2(TPrecision factorY){
      using namespace FortranLinalg;
      DenseMatrix<int> KNNX =  DenseMatrix<int>(knnX+1, lambdafY.N());
      DenseMatrix<TPrecision> KNNXD = DenseMatrix<TPrecision>(knnX+1, lambdafY.N());
      Geometry<TPrecision>::computeKNN(lambdafY, KNNX, KNNXD, sl2metric);

      TPrecision sigma = 0;
      for(unsigned int i=0; i<KNNXD.N(); i++){
        sigma += sqrt( KNNXD(knnX, i) ); 
      }
      sigma *= factorY/KNNXD.N();
      sX = sigma;
      kernelX.setKernelParam(sigma);

      KNNX.deallocate();
      KNNXD.deallocate();

    };




    //Kernel bandwidth estimate for coordinate mapping non-adaptive 
    //isotropic  based on knnY distances
    void computeKernelY(TPrecision factorY){
      using namespace FortranLinalg;
      //DenseMatrix<int> KNNY =  DenseMatrix<int>(knnX+1, lambdaY.N());
      //DenseMatrix<TPrecision> KNNYD = DenseMatrix<TPrecision>(knnX+1, lambdaY.N());

      TPrecision sigma = 0;
      for(unsigned int i=0; i<KNNYD.N(); i++){
        sigma += sqrt( KNNYD(knnY, i) ); 
      }
      sigma *= factorY/KNNYD.N() ;
      kernelY.setKernelParam(sigma);      
      
      updateKY();
    };

    //Kernel bandwidth estimate for coordinate mapping non-adaptive 
    //isotropic  based on knnY distances
    void computeKernelZ(TPrecision factorZ){
      using namespace FortranLinalg;
    
      DenseMatrix<int> KNNZ =  DenseMatrix<int>(knnY+1, lambdaY.N());
      DenseMatrix<TPrecision> KNNZD = DenseMatrix<TPrecision>(knnY+1, lambdaY.N());

      Geometry<TPrecision>::computeKNN(lambdaZ, KNNZ, KNNZD, sl2metric);

      TPrecision sigma = 0;
      for(unsigned int i=0; i<KNNZD.N(); i++){
        sigma += sqrt( KNNZD(knnY, i) ); 
      }
      sigma *= factorZ/KNNZD.N() ;
      kernelZ.setKernelParam(sigma);      
    
    };



    void updateKY(){ 
      for(unsigned int i=0; i<KNNYD.N(); i++){
        TPrecision sumw = 0;
        for(unsigned int j=0; j<KNNYD.M(); j++){
          KY(j, i) = kernelY.f( KNNYD(j,i) );
          sumw += KY(j, i);
        }
        for(unsigned int j=0; j<KNNYD.M(); j++){
          KY(j, i) /=sumw;
        }

      }
    };






    double numGradSigmaX(Risk risk, Penalty penalty, TPrecision scaling, int nPoints){
      using namespace FortranLinalg;
      TPrecision sigmaX = kernelX.getKernelParam();
      TPrecision sigmaX2 = sigmaX*scaling;


      DenseVector<int> sample(nPoints);
      for(int i=0; i<nPoints; i++){
        sample(i) = (int) ( FastCEM<TPrecision>::rand.Uniform()*Y.N() );
      }


      Objective<TPrecision> obj1 = objective(risk, penalty, sample);
      kernelX.setKernelParam(sigmaX2);
      Objective<TPrecision> obj2 = objective(risk, penalty, sample);

      double g = ( obj2.total - obj1.total ) / fabs(sigmaX2 - sigmaX);
      
      kernelX.setKernelParam(sigmaX);
      sample.deallocate();

      return -g;

    };





    double numGradSigmaY(Risk risk, Penalty penalty, TPrecision scaling, int nPoints){
      using namespace FortranLinalg;
      TPrecision sigmaY = kernelY.getKernelParam();
      TPrecision sigmaY2 = sigmaY*scaling;

      DenseVector<int> sample(nPoints);
      for(int i=0; i<nPoints; i++){
        sample(i) = (int) ( rand.Uniform()*Y.N() );
      }

      Objective<TPrecision> obj1 = objective(risk, penalty, sample);
      kernelY.setKernelParam(sigmaY2);
      updateKY();
      update();
      //updateKNNX();

      Objective<TPrecision> obj2 = objective(risk, penalty, sample);

      double g = ( obj2.total - obj1.total ) / fabs(sigmaY2 - sigmaY);
      //myprintf( "Update BW Y - total: " << obj1.total );
      //myprintf( "Update BW Y - total: " << obj2.total );



      kernelY.setKernelParam(sigmaY);
      updateKY();
      update();
      //updateKNNX();
      
      sample.deallocate();
      
      return -g;

    };









    //numerical gradient for paramater z_r of the coordinate mapping
    //epsilon = finite difference delta
    void numGradX(int r, FortranLinalg::DenseVector<TPrecision> &gx, TPrecision epsilon, Risk
        risk, Penalty penalty, int nPoints){
      using namespace FortranLinalg;

      //DenseVector<int> sample=Linalg<int>::ExtractColumn(KNNX,r);

      
      DenseVector<int> sample(nPoints);
      for(int i=0; i<nPoints; i++){
        sample(i) = (int)( rand.Uniform()*Y.N() );
      }
      


      Objective<TPrecision> obj1 = objective(risk, penalty, sample);
      //vary each coordinate
      for(unsigned int i=0; i<gx.N(); i++){
        lambdaZ(i, r) += epsilon;
        computeLambdafY();
        
        Objective<TPrecision> obj2 = objective(risk, penalty, sample);
        
        gx(i) = ( obj2.total - obj1.total ) / epsilon;
        
        lambdaZ(i, r) -= epsilon;
      }

      sample.deallocate();

      computeLambdafY();
    
    };









    //Least qquares fitting procedures for g
    FortranLinalg::DenseMatrix<TPrecision> LeastSquares(FortranLinalg::Vector<TPrecision> &x){
      if(qfit){
        return LeastSquares2(x);
      }
      else{
        return LeastSquares1(x);
      }
    };


    FortranLinalg::DenseMatrix<TPrecision> LeastSquares(int n){
      if(qfit){
        return LeastSquares2(n);
      }
      else{
        return LeastSquares1(n);
      }
    };





    //locally quardatic regression


    //Least squares for locally linear regression at x
    FortranLinalg::DenseMatrix<TPrecision> LeastSquares2(FortranLinalg::Vector<TPrecision> &x){
      using namespace FortranLinalg;


      DenseVector<TPrecision> xc(lambdaZ.M());
      DenseMatrix<TPrecision> kr(lambdaZ.M(), lambdaZ.M());

      //Linear system
      DenseMatrix<TPrecision> X1(lambdafY.N(), lambdaZ.M() * (lambdaZ.M()+1) +1);
      DenseMatrix<TPrecision> Y1(lambdafY.N(), Y.M());
      DenseVector<TPrecision> W(lambdafY.N());
            
      //DenseVector<int> knn =  DenseVector<int>(knnX+1);
      //DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
      //Geometry<TPrecision>::computeKNN(fY, x, knn, knnD, sl2metric);

      //Setup linear system
      for(unsigned int i2=0; i2 < lambdafY.N(); i2++){

        int i=i2; //knn(i2);
        //W(i2) = kernelX.f( knnD(i) );
        W(i2) = kernelX.f(x, lambdafY, i );

        Linalg<TPrecision>::ExtractColumn(lambdafY, i, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i2, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< lambdaZ.M(); j++){
          X1(i2, jindex) = xc(j);
          ++jindex;
        }

        for(unsigned int j=0; j< kr.M(); j++){
          for(unsigned int k=0; k< kr.N(); k++){
            X1(i2, jindex) = kr(j, k);
            ++jindex;
          }
        }
        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i2, m) = Y(m, i);
        }
      }

      Linalg<TPrecision>::OuterProduct(x, x, kr);
      DenseVector<TPrecision> x2(X1.N());
      x2(0) = 1;
      int jindex = 1;
      for(unsigned int j=0; j< lambdaZ.M(); j++){
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
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      x2.deallocate();

      //knn.deallocate();
      //knnD.deallocate();

      return sol;
    };



/*

    //Linear least squares for locally linear regression within training data
    FortranLinalg::DenseMatrix<TPrecision> LeastSquares2(unsigned int n){
      using namespace FortranLinalg;


      DenseVector<TPrecision> xc(lambdaZ.M());
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractColumn(fY, n);
      DenseMatrix<TPrecision> kr(lambdaZ.M(), lambdaZ.M());

      //Linear system
      DenseMatrix<TPrecision> X1(lambdafY.N(), lambdaZ.M() * (lambdaZ.M()+1) +1);
      DenseMatrix<TPrecision> Y1(lamdafY.N(), Y.M());
      DenseVector<TPrecision> W(lambdafY.N());
            
      //Setup linear system
      for(unsigned int i2=0; i2 < lambdafY.N(); i2++){

        int i=i2;//KNNX(i2, n);
        W(i2) = kernelX.f(lambdafY, n, lambdafY, i );

        Linalg<TPrecision>::ExtractColumn(fY, i, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i2, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< lambdaZ.M(); j++){
          X1(i2, jindex) = xc(j);
          ++jindex;
        }

        for(unsigned int j=0; j< kr.M(); j++){
          for(unsigned int k=0; k< kr.N(); k++){
            X1(i2, jindex) = kr(j, k);
            ++jindex;
          }
        }
        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i2, m) = lambdaY(m, i);
        }
      }

      Linalg<TPrecision>::OuterProduct(x, x, kr);
      DenseVector<TPrecision> x2(X1.N());
      x2(0) = 1;
      int jindex = 1;
      for(unsigned int j=0; j< lambdaZ.M(); j++){
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
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      x2.deallocate();
      x.deallocate();


      return sol;


    };

*/




    //locally linear regression

    //Least squares for locally linear regression at x
    FortranLinalg::DenseMatrix<TPrecision> LeastSquares1(FortranLinalg::Vector<TPrecision> &x){
      using namespace FortranLinalg;
      //Linear system
      DenseMatrix<TPrecision> X1(lambdaY.N(), lambdaZ.M()+1);
      DenseMatrix<TPrecision> Y1(lambdaY.N(), Y.M());
      DenseVector<TPrecision> W(lambdaY.N() );
      
      /*
      DenseVector<int> knn =  DenseVector<int>(knnX+1);
      DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
      Geometry<TPrecision>::computeKNN(fY, x, knn, knnD, sl2metric);
*/

      //Setup linear system
      for(unsigned int i2=0; i2 < lambdafY.N(); i2++){
        int i = i2;//knn(i2);
        //W(i2) = kernelX.f( knnD(i2) ); 
        W(i2) = kernelX.f(x, lambdafY, i );

        X1(i2, 0) = 1;
        for(unsigned int j=0; j< lambdaZ.M(); j++){
          X1(i2, j+1) = lambdafY(j, i);
        }

        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i2, m) = lambdaY(m, i);
        }
      }


      DenseVector<TPrecision> x2(x.N()+1);
      x2(0) = 1;
      for(unsigned int i=1; i<x2.N(); i++){
        x2(i) = x(i-1);
      }

      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x2);

      //cleanup
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      x2.deallocate();
      //knnD.deallocate();
      //knn.deallocate();

      return sol;
    };









/*

    //Linear least squares for locally linear regression within training data
    FortranLinalg::DenseMatrix<TPrecision> LeastSquares1(unsigned int n){
           using namespace FortranLinalg;
      //Linear system
      DenseMatrix<TPrecision> X1(lambdaY.N(), lambdaZ.M()+1);
      DenseMatrix<TPrecision> Y1(lambdaY.N(), Y.M());
      DenseVector<TPrecision> W(lambdaY.N());
      
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractColumn(fY, n);
      //Setup linear system
      for(unsigned int i2=0; i2 < lambdafY.N(); i2++){
        int i = i2;//KNNX(i2, n);
        W(i2) = kernelX.f(lambdafY, n, lambdafY, i);
        X1(i2, 0) = 1;
        for(unsigned int j=0; j< lambdaZ.M(); j++){
          X1(i2, j+1) = fY(j, i);
        }

        for(unsigned int m = 0; m<lambdaY.M(); m++){
          Y1(i2, m) = lambdaY(m, i);
        }
      }


      DenseVector<TPrecision> x2(x.N()+1);
      x2(0) = 1;
      for(unsigned int i=1; i<x2.N(); i++){
        x2(i) = x(i-1);
      }

      DenseMatrix<TPrecision> sol = gEstimate(X1, Y1, W, x2);

      //cleanup
      X1.deallocate();
      Y1.deallocate();
      W.deallocate();
      x2.deallocate();
      x.deallocate();

      return sol;
    }; 
      

*/


    FortranLinalg::DenseMatrix<TPrecision> gEstimate(FortranLinalg::DenseMatrix<TPrecision> &X1,
        FortranLinalg::DenseMatrix<TPrecision> Y1, FortranLinalg::DenseVector<TPrecision> &W,
        FortranLinalg::DenseVector<TPrecision> &x){
      using namespace FortranLinalg;

      //W:  knnX
      //X1:  knnX x Z
      //Y1:  knnX x Y
      TPrecision sumw = 1;//Linalg<TPrecision>::Sum(W);
      DenseMatrix<TPrecision> A(X1.M(), X1.N());
      for(unsigned int i=0; i<X1.M(); i++){
        Linalg<TPrecision>::ScaleRow(X1, i, W(i)/sumw, A);
      }



      DenseMatrix<TPrecision> Q = Linalg<TPrecision>::Multiply(X1, A, true); // Z x Z
      //Linalg<TPrecision>::Print(Q);
      SymmetricEigensystem<TPrecision> eigs(Q);
      TPrecision thres =  0.00000001*eigs.ew(eigs.ew.N()-1);
      //Linalg<TPrecision>::Print(eigs.ev);
      //exit(0);
      DenseMatrix<TPrecision> tmp = Linalg<TPrecision>::Copy(eigs.ev);
      for(unsigned int i=0; i<eigs.ew.N(); i++){
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

      DenseMatrix<TPrecision> sol( lambdaZ.M()+1, Y.M() );
      for(unsigned int k=0; k< lambdaZ.M(); k++){
        for(unsigned int i=0; i<X1.M(); i++){
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













    //Gradient descent 
    bool optimizeZ(int nIterations, int nPoints, TPrecision scalingZ,
        TPrecision scalingBW, int verbose=1, Risk risk = ORTHO, Penalty penalty =
        NONE){
      using namespace FortranLinalg;

      
      //TPrecision sX = kernelX.getKernelParam(); 

      //---Storage for syncronous updates 
      DenseMatrix<TPrecision> sync(lambdaZ.M(), lambdaZ.N());

      //---Do nIterations of gradient descent     
      DenseMatrix<TPrecision> Ztmp(lambdaZ.M(), lambdaZ.N());
      DenseMatrix<TPrecision> Zswap;

      //gradient direction
      DenseVector<TPrecision> gx(lambdaZ.M());     
      if(verbose > 0){
        myprintf( "Start Gradient Descent \n" );
      }

      bool updated = false;
      for(int i=0; i<nIterations; i++){
        bool updateBW = updateBandwidthY(scalingBW, verbose, risk, penalty, nPoints)
          != 0; 
        Objective<TPrecision> objPrev = objective(risk, penalty);

        if(verbose > 0){
          myprintf( "Mse start: %f \n" , objPrev.mse );
          myprintf( "Ortho start: %f  \n" , objPrev.ortho );
          myprintf( "Penalty start: %f \n" , objPrev.penalty );
          myprintf( "Total start: %f \n" , objPrev.total );
        }

        if(verbose > 1){
          myprintf( "Kernel X sigma: %f \n" , kernelX.getKernelParam() ); 
          myprintf( "Kernel Y sigma: %f \n" , kernelY.getKernelParam() ); 
        }
        sX = kernelX.getKernelParam()/2;
        //compute gradient for each point
        TPrecision maxL = 0;
        for(unsigned int j=0; j < lambdaZ.N(); j++){
          //compute gradient
          //gradX(j, gx);
          numGradX(j, gx, sX*0.1, risk, penalty, nPoints);


          //Asynchronous updates
          //Linalg<TPrecision>::ColumnAddScale(Z, j, -scaling*sX, gx);


          //store gradient for syncronous updates
          TPrecision l = Linalg<TPrecision>::Length(gx);
          if(maxL < l){
            maxL = l;
          }
          //Linalg<TPrecision>::Scale(gx, 1.f/l, gx);
          for(unsigned int k=0; k<lambdaZ.M(); k++){
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
          myprintf( "sX: %f \n" , sX );
          myprintf( "scaling: %f \n",  s );
        }


        //Approximate line search using a quadratic fit
        DenseMatrix<TPrecision> A(3, 3);
        DenseMatrix<TPrecision> b(3, 1);
        Linalg<TPrecision>::Zero(A);
        Objective<TPrecision> objTmp = objective(risk, penalty);
        b(0, 0) = objTmp.total;

        Linalg<TPrecision>::AddScale(lambdaZ, -1*s, sync, Ztmp);
        Zswap = lambdaZ;
        lambdaZ = Ztmp;
        Ztmp = Zswap;
        update();
        //updateKNNX();

        objTmp = objective(risk, penalty);
        b(1, 0) = objTmp.total;


        Linalg<TPrecision>::AddScale(Zswap, -2*s, sync, lambdaZ);
        update();
        //updateKNNX();
        objTmp = objective(risk, penalty);
        b(2, 0) = objTmp.total;


        if(verbose > 1){
          myprintf( "line search: \n" );
          myprintf( "%f\n", b(0, 0) );
          myprintf( "%f\n", b(1, 0) );
          myprintf( "%f\n", b(2, 0) );
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
          myprintf( "step: %f \n ", h );
          Linalg<TPrecision>::AddScale(Ztmp, h, sync, lambdaZ);        

        }
        else if( b(0,0) > b(1, 0) ){
          myprintf( "step: %f \n" , -2*s );
          //do nothing step to -2*s          
        }
        else{
          //stop gradient descent - no step
          stop = true;
          //Linalg<TPrecision>::AddScale(Ztmp, -s, sync, Z);
        }

        A.deallocate();
        b.deallocate();
        q.deallocate();





        update();
        //updateKNNX();

        bool updatedBW = updateBandwidthY(scalingBW, verbose, risk, penalty,
            nPoints) != 0; 
        stop = stop && !updatedBW;

        Objective<TPrecision> obj = objective(risk, penalty); 
        if(verbose > 0){
          myprintf( "Iteration: %d \n" , i );
          myprintf( "MSE: %f \n" ,  obj.mse );     
          myprintf( "Ortho: %f \n" ,  obj.ortho );
          myprintf( "Penalty: %f \n" , obj.penalty );
          myprintf( "Total: %f \n\n", obj.total  );
        }

        if(!stop){ 
          stop = objPrev.total <= obj.total;
        }

        if(stop){          
          Zswap = Ztmp;
          Ztmp = lambdaZ;
          lambdaZ = Zswap;
          update();
          if(!updateBW)
            break;
        }

        updated=true; 
        objPrev = obj;      

      }


      //cleanup 
      sync.deallocate();
      gx.deallocate();
      Ztmp.deallocate();

      return updated;
    };













    int updateBandwidthX(TPrecision scaling, int verbose, Risk risk, Penalty
        penalty, int nPoints){      
      using namespace FortranLinalg;

      int updated = 0;

      TPrecision scale = 1+scaling;
      TPrecision gradSX1 = numGradSigmaX(risk, penalty, scale, nPoints);
      TPrecision gradSX2 = numGradSigmaX(risk, penalty, 1.0/scale, nPoints);

      if(verbose > 1){
        myprintf( "grad sigmaX: %f / %f \n" , gradSX1 , gradSX2 );
      }

      if(gradSX1 > gradSX2){
        if(gradSX1 > 0){
          kernelX.setKernelParam(scale*kernelX.getKernelParam());
          updated = 1;
        }

      }
      else{
        if(gradSX2 > 0){
          kernelX.setKernelParam(kernelX.getKernelParam()/scale);
          updated = -1;
        }

      }

      return updated;
    };













    int updateBandwidthY(TPrecision scaling, int verbose, Risk risk, Penalty
        penalty, int nPoints){      
      using namespace FortranLinalg;

      int updated = 0;

      TPrecision scale = 1+scaling;
      TPrecision gradSY1 = numGradSigmaY(risk, penalty, scale, nPoints);
      TPrecision gradSY2 = numGradSigmaY(risk, penalty, 1.0/scale, nPoints);

      if(verbose > 1){
        myprintf( "grad sigmaY: %f / %f \n", gradSY1,  gradSY2 );
      }

      if(gradSY1 > gradSY2){
        if(gradSY1 > 0){
          kernelY.setKernelParam(scale * kernelY.getKernelParam());
          updated = 1;
        }

      }
      else{
        if(gradSY2 > 0){
          kernelY.setKernelParam( kernelY.getKernelParam() / scale );
          updated = -1;
        }

      }


      if(updated != 0){
        //computeKY();
        updateKY();
        update();
        //updateKNNX();
      }	

      return updated;
    };













    double deltaPath(FortranLinalg::DenseMatrix<TPrecision> &py, int changed,
        FortranLinalg::DenseVector<TPrecision> &cy){
      double d1 = l2metric.distance(py, changed-1, py, changed);
      double d2 = l2metric.distance(py, changed+1, py, changed);
      double d1c = l2metric.distance(py, changed-1, cy);
      double d2c = l2metric.distance(py, changed+1, cy);
      return (d1 + d2) - (d1c + d2c);
    };











    double lengthPath(FortranLinalg::DenseMatrix<TPrecision> &py){
      double d = 0;
      for(unsigned int i=1; i<py.N(); i++){
        d += l2metric.distance(py, i-1, py, i);
      }
      return d;
    };





}; 


#endif

