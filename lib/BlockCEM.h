#ifndef BLOCKCEM_H
#define BLOCKCEM_H


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




//Conditional Expectation Manifolds
template <typename TPrecision>
class BlockCEM{

  private:    
    FortranLinalg::DenseMatrix<TPrecision> X;
    FortranLinalg::DenseMatrix<TPrecision> Y;
    FortranLinalg::DenseMatrix<TPrecision> Yp;

    ANNkd_tree *annTree; 
    FortranLinalg::DenseVector<TPrecision> xMin;
    FortranLinalg::DenseVector<TPrecision> xMax; 

    unsigned int knnX;

    EuclideanMetric<TPrecision> l2metric;
    SquaredEuclideanMetric<TPrecision> sl2metric;


    GaussianKernel<TPrecision> kernelX;


    bool qfit;

    Random<TPrecision> rand;




  public:

    enum Risk {MSE, ORTHO, ORTHO_NORM_1, ORTHO_NORM_2, ORTHO_NORM_3,
      ORTHO_NORM_4};


    enum Penalty {NONE, UNIT, DENSITY};


    struct Objective{

      TPrecision mse;
      TPrecision ortho;
      TPrecision penalty;

      TPrecision total;
    };


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




    void cleanup(){      
      Y.deallocate();
      Yp.deallocate();
      X.deallocate();
      xMin.deallocate();
      xMax.deallocate();

      delete annTree;
      annClose();

    };





    //Create Condtional Expectation Manifold 
    BlockCEM(FortranLinalg::DenseMatrix<TPrecision> Ydata,
        FortranLinalg::DenseMatrix<TPrecision> Xinit, int nnX, TPrecision
        sigmaX, bool sigmaAsFactor = true, bool quadratic = false ) : Y(Ydata),
        X(Xinit), knnX(nnX), qfit(quadratic){

      using namespace FortranLinalg;
      kernelX = GaussianKernel<TPrecision>( X.M());

      if(knnX <= X.M()){
        knnX = X.M()+1;
      }	  
      if(knnX >= X.N()){
        knnX = X.N()-1;
      }
      xMin = Linalg<TPrecision>::RowMin(X);
      xMax = Linalg<TPrecision>::RowMax(X);

      Yp = DenseMatrix<TPrecision>(Y.M(), Y.N());
      if( sigmaAsFactor ){
        computeKernelX2(sigmaX);
      }
      else{
        kernelX.setKernelParam(sigmaX);
      }

      annTree = NULL;
      updateANNTree();
    };



    //evalue objective function
    Objective objective(Risk risk, Penalty penalty){
      using namespace FortranLinalg;
      static DenseVector<int> dummy;
      return objective(risk, penalty, dummy);
    };



    Objective objective(Risk risk, Penalty penalty, FortranLinalg::DenseVector<int> points){
      using namespace FortranLinalg;
      Objective obj;

      obj.ortho = 0;
      obj.mse = 0;
      obj.penalty = 0;      

      //Jacobian of g(x)
      DenseMatrix<TPrecision> J(Y.M(), X.M());

      //Temp vars.
      DenseVector<TPrecision> x(X.M());
      DenseVector<TPrecision> gx(Y.M());
      DenseVector<TPrecision> diff(Y.M());
      DenseVector<TPrecision> pDot(X.M());

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

        Linalg<TPrecision>::ExtractColumn(X, i, x);
        g(x, gx, J);

        Linalg<TPrecision>::Subtract(gx, Y, i, diff);  
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
      gx.deallocate();
      diff.deallocate();
      J.deallocate();
      J2.deallocate();
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



    Objective objective(FortranLinalg::DenseVector<TPrecision> x,
        FortranLinalg::DenseVector<TPrecision> y, Risk risk, Penalty penalty){
      using namespace FortranLinalg;
      Objective obj;

      obj.ortho = 0;
      obj.mse = 0;
      obj.penalty = 0;      

      //Jacobian of g(x)
      DenseMatrix<TPrecision> J(Y.M(), X.M());

      //Temp vars.
      DenseVector<TPrecision> gx(Y.M());
      DenseVector<TPrecision> diff(Y.M());
      DenseVector<TPrecision> pDot(X.M());

      //Length of Jacobian / 
      DenseMatrix<TPrecision> J2( J.N(), J.N() );
      Linalg<TPrecision>::Zero(J2);


      g(x, gx, J);

      Linalg<TPrecision>::Subtract(gx, y, diff);  
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
      
      if(penalty == UNIT){
        obj.penalty = Linalg<TPrecision>::Sum(J2);
      }
      

      pDot.deallocate();
      gx.deallocate();
      diff.deallocate();
      J.deallocate();
      J2.deallocate();

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
        scalingX=0.5, TPrecision scalingBW=0.1, int verbose=1, Risk risk=
        ORTHO, Penalty penalty = NONE, bool optimalSigmaX = true){
      using namespace FortranLinalg;

      if( nPoints > Y.N()){
        nPoints = Y.N();
      }


      //inital bandwidth
      {
        int cr = updateBandwidthX(scalingBW, verbose, risk, penalty, nPoints);
        //optimize conditional expectation bandwidth for g
        if(risk != CEM_MSE && optimalSigmaX){
          int pr = 0;
          while( cr != 0 && pr+cr != 0){ 
            pr = cr;
            cr = updateBandwidthX(scalingBW, verbose, risk, penalty, nPoints);
          }
        }
        

        if(verbose > 0){
          Objective obj = objective(risk, penalty);
          myprintf( "Mse after g: %f \n" , obj.mse );
          myprintf( "Ortho after g: %f  \n" , obj.ortho );
          myprintf( "Penalty after g: %f \n" , obj.penalty );
          myprintf( "Total after g: %f \n \n \n ---- \n \n" , obj.total );
        }
        
        updateANNTree();
      }


      for(int i=0; i<nIterations; i++){
      
        bool updatedX = true;

        Objective obj = objective(risk, penalty);
        if(verbose > 0){
          myprintf( "Mse start: %f \n" , obj.mse );
          myprintf( "Ortho start: %f  \n" , obj.ortho );
          myprintf( "Penalty start: %f \n" , obj.penalty );
          myprintf( "Total start: %f \n \n" , obj.total );
        }

        //optimize f
        updatedX = orthogonalizeX(scalingX, MSE, NONE); //risk, penalty);        
        //updatedX = optimizeX(10, nPoints, scalingX, scalingBW, verbose, risk,
        //    penalty); 
        Objective objN = objective(risk, penalty);
        if(  verbose > 0){
          myprintf( "Mse after ortho: %f \n" , objN.mse );
          myprintf( "Ortho after ortho: %f  \n" , objN.ortho );
          myprintf( "Penalty after ortho: %f \n" , objN.penalty );
          myprintf( "Total after g: %f \n \n" , objN.total );
        }

        updatedX = (objN.total - obj.total) < 0;
        

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
        

        if(verbose > 0){
          Objective obj = objective(risk, penalty);
          myprintf( "Mse after g: %f \n" , obj.mse );
          myprintf( "Ortho after g: %f  \n" , obj.ortho );
          myprintf( "Penalty after g: %f \n" , obj.penalty );
          myprintf( "Total after g: %f \n \n \n ---- \n \n" , obj.total );
        }


        if(!updatedBW  && !updatedX){
          break;
        }

        updateANNTree();
     
      }       

    };












    //Find best parametrization for y
    bool f(FortranLinalg::DenseVector<TPrecision> &y, FortranLinalg::DenseVector<TPrecision> &out, TPrecision stepX, Risk risk, Penalty penalty){
      int k=-1;
      double kd=-1;
      ANNpoint p = y.data();
      annTree->annkSearch( p, 1, &k, &kd, 0.00001);
      FortranLinalg::Linalg<TPrecision>::ExtractColumn(X, k, out);
      return orthogonalize(out, y, stepX, risk, penalty);
    };











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
      for(unsigned int i=0; i< X.M(); i++){
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
    FortranLinalg::DenseMatrix<TPrecision> getX(){
      return X;
    };


    
    
    FortranLinalg::DenseMatrix<TPrecision> parametrize(FortranLinalg::DenseMatrix<TPrecision> &Ypoints, Risk risk=MSE, Penalty penalty=NONE){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> proj(X.M(), Ypoints.N());
      parametrize(Ypoints, proj, risk, penalty);     
      return proj;
    };


    void parametrize(FortranLinalg::DenseMatrix<TPrecision> &Ypoints, FortranLinalg::DenseMatrix<TPrecision> &proj, Risk risk=MSE, Penalty penalty=NONE){
      using namespace FortranLinalg;
      
      DenseVector<TPrecision> y( Y.M() );
      DenseVector<TPrecision> x( X.M() );
      for(unsigned int i=0; i < Ypoints.N(); i++){
        Linalg<TPrecision>::ExtractColumn(Ypoints, i, y);
        f(y, x, 0.001, risk, penalty);
        Linalg<TPrecision>::SetColumn(proj, i, x);
      }
      x.deallocate();
      y.deallocate();

    }; 









    FortranLinalg::DenseMatrix<TPrecision> reconstruct(FortranLinalg::DenseMatrix<TPrecision> &Xpoints){
      using namespace FortranLinalg;
      DenseMatrix<TPrecision> proj(Y.M(), Xpoints.N());
      reconstruct(Xpoints, proj);     
      return proj;
    };





    void reconstruct(FortranLinalg::DenseMatrix<TPrecision> &Xpoints, FortranLinalg::DenseMatrix<TPrecision> &proj){
      using namespace FortranLinalg;
      DenseVector<TPrecision> tmp(X.M()); 
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
      DenseVector<TPrecision> gx(Y.M()); 
      DenseVector<TPrecision> x(X.M()); 
      for(unsigned int i=0; i < Y.N(); i++){
        Linalg<TPrecision>::ExtractColumn(X, i, x);
        g(x, gx);
        Linalg<TPrecision>::SetColumn(proj, i, gx);
      }
      gx.deallocate();
      x.deallocate();
      return proj;
    };










    TPrecision getSigmaX(){
      return kernelX.getKernelParam();
    };


    GaussianKernel<TPrecision> getKernelX(){
      return kernelX;
    };



    double geodesic(FortranLinalg::DenseMatrix<TPrecision> &px, double step =
        0.01, int nIter = 100){
      using namespace FortranLinalg;
     
     int n = px.N(); 
      DenseVector<TPrecision> cx(X.M());
      DenseVector<TPrecision> cy(Y.M());


      //DenseMatrix<TPrecision> pxC = Linalg<TPrecision>::Copy(px);
      DenseMatrix<TPrecision> py = reconstruct(px);

      double stepSize = kernelX.getKernelParam()*step;
      DenseMatrix<TPrecision> gx(X.M(), px.N() );
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

      DenseMatrix<TPrecision> px(X.M(), n); 
      DenseVector<TPrecision> dx = Linalg<TPrecision>::Subtract(xe, xs);
      DenseVector<TPrecision> cx(X.M());

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
      std::vector< DenseMatrix<TPrecision> > T(X.M());
     for(unsigned int i=0; i<X.M(); i++){
       T[i] = DenseMatrix<TPrecision>(Y.M(), X.M());     
       Linalg<TPrecision>::Zero(T[i]);
     }
        
     //Jacobian of g(x)
     DenseMatrix<TPrecision> J(Y.M(), X.M());
     Linalg<TPrecision>::Zero(J); 
     
     //Kernel values
     DenseVector<TPrecision> k(X.N());
     //Kernel gradients
     DenseMatrix<TPrecision> kg(X.M(), X.N());
     //Kernel hessians
     std::vector< DenseMatrix<TPrecision> > kh(X.N());
           
    // DenseVector<int> knn =  DenseVector<int>(knnX+1);
    // DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
    // Geometry<TPrecision>::computeKNN(X, x, knn, knnD, sl2metric);

     //Precomputed sums of kernel value
     TPrecision sumk = 0;
     DenseVector<TPrecision> sumkg(X.M());
     Linalg<TPrecision>::Zero(sumkg);
     DenseMatrix<TPrecision> sumkh(X.M(), X.M());
     Linalg<TPrecision>::Zero(sumkh);

     //Temp vars.
     DenseVector<TPrecision> gtmp(X.M());
     DenseVector<TPrecision> fy(X.M());

     //update fY if necessary

     //Precompte kernel values and first and second order derivatives 
     //(gradient and hessian of kernel function)
     
     for(unsigned int i=0; i < Y.N(); i++){

       Linalg<TPrecision>::ExtractColumn(X, i, fy);

       k(i) = kernelX.gradf(x, fy, gtmp);
       kh[i] = kernelX.hessian(x, fy);
       sumk += k(i);

        for(unsigned int k=0; k < X.M(); k++){
          kg(k, i) = gtmp(k);
          sumkg(k) += kg(k, i);
          for( unsigned int l=0; l < X.M(); l++){
            sumkh(k, l) += kh[i](k, l);
          }
        }
      }

      TPrecision sumk2 = sumk*sumk;
      TPrecision sumk4 = sumk2 * sumk2;

      //Build T - block hessian matrix and J - Jacobian
      for(unsigned int n = 0; n < Y.N(); n++){
        for(unsigned int i=0; i<X.M(); i++){
          
          //T
          for(unsigned int j = 0; j < X.M(); j++){
            //First order and second order kernel derivatives dx_i, dx_j
            TPrecision c  = kh[n](i, j) * sumk      +  kg(j, n) * sumkg(i)   ;
                       c -= kg(i, n)    * sumkg(j)  +  k(n)     * sumkh(i, j);
                       c *= sumk2;
                       c -= (kg(j, n) * sumk        -  k(n)     * sumkg(j)) * 2.0 * sumk * sumkg(i);
            //times dependant variable y
            for(unsigned int r=0; r < Y.M() ; r++){
              T[i](r, j) +=  c * Y(r, n );
            }
          }
          
          //J
          TPrecision c  = kg(i, n) * sumk      -  k(n) * sumkg(i)   ;
          for(unsigned int r=0; r< Y.M(); r++){
            J(r, i) += c * Y(r, n );
          }
        }
      }    
   
     Linalg<TPrecision>::Scale(J, 1.0/sumk2, J);

     for(unsigned int i=0; i < X.M(); i++){
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
     std::vector< DenseMatrix<TPrecision> > TgInv( X.M() );
     std::vector< DenseMatrix<TPrecision> > QTgInv( X.M() );
     for(unsigned int i=0; i < X.M(); i++){
      TgInv[i] = Linalg<TPrecision>::Multiply(T[i], gInv);
      QTgInv[i] = Linalg<TPrecision>::Multiply(Q, TgInv[i]);
     }
     DenseVector<TPrecision> H1(Y.M());
     Linalg<TPrecision>::Zero(H1);
     frob = 0;
     for(unsigned int i = 0; i < X.M(); i++){
        for(unsigned int r = 0; r < Y.M(); r++){
           H1(r) += TgInv[i](r, i);
           for(unsigned int q=0; q< X.M(); q++){
            frob += QTgInv[i](r, q) * QTgInv[i](r, q); 
           }
        }
     }
     frob = sqrt(frob);
     
     DenseVector<TPrecision> H = Linalg<TPrecision>::Multiply(Q, H1);
     Linalg<TPrecision>::Scale(H, 1.0/ X.M(), H);
     
     //Mean curvature
     meanC = Linalg<TPrecision>::Length(H);
     
     
     //Gauss curvature
     Linalg<TPrecision>::Scale(H, 1.0/meanC, H);
    
     DenseMatrix<TPrecision> B(X.M(), X.M());
     for(unsigned int i=0; i<B.M(); i++){
        for(unsigned int j=0; j< B.N(); j++){
          TPrecision dot = 0;
          for(unsigned int r=0; r < Y.M(); r++){
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
     for(unsigned int i=0; i < X.M(); i++){
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
     for(unsigned int i=0; i < X.N(); i++){
        kh[i].deallocate();
     }
     sumkg.deallocate();
     sumkh.deallocate();
  
      
     return H;

   };



















  private:

 





    //Kernel bandwidth estimate for coordinate mapping non-adaptive 
    //isotropic  based on knnY distances
    void computeKernelX2(TPrecision factorX){
      using namespace FortranLinalg;
      DenseMatrix<int> KNNX =  DenseMatrix<int>(knnX+1, X.N());
      DenseMatrix<TPrecision> KNNXD = DenseMatrix<TPrecision>(knnX+1, X.N());
      Geometry<TPrecision>::computeKNN(X, KNNX, KNNXD, sl2metric);

      TPrecision sigma = 0;
      for(unsigned int i=0; i<KNNXD.N(); i++){
        sigma += sqrt( KNNXD(knnX, i) ); 
      }
      sigma *= factorX/KNNXD.N();
      kernelX.setKernelParam(sigma);

      KNNX.deallocate();
      KNNXD.deallocate();

    };



    double numGradSigmaX(Risk risk, Penalty penalty, TPrecision scaling, int nPoints){
      using namespace FortranLinalg;
      TPrecision sigmaX = kernelX.getKernelParam();
      TPrecision sigmaX2 = sigmaX*scaling;


      DenseVector<int> sample(nPoints);
      for(int i=0; i<nPoints; i++){
        sample(i) = (int) ( rand.Uniform()*Y.N() );
      }


      Objective obj1 = objective(risk, penalty, sample);
      kernelX.setKernelParam(sigmaX2);
      Objective obj2 = objective(risk, penalty, sample);

      double g = ( obj2.total - obj1.total ) / fabs(sigmaX2 - sigmaX);
      
      kernelX.setKernelParam(sigmaX);
      sample.deallocate();

      return -g;

    };




    void updateANNTree(){
   
      reconstruct(X, Yp);
      if(annTree != NULL){
        delete annTree;
      }

      ANNpointArray pts= Yp.getColumnAccessor();
      annTree = new ANNkd_tree( pts, Yp.N(), Yp.M());
   
    };





    //Gradient descent 
    bool orthogonalizeX(TPrecision stepX, Risk risk = ORTHO, Penalty penalty = NONE){
      using namespace FortranLinalg;

      DenseVector<TPrecision> xMin = Linalg<TPrecision>::RowMin(X);
      DenseVector<TPrecision> xMax = Linalg<TPrecision>::RowMax(X);
      
      //TPrecision **yc = Y.getColumnAccessor();
      //TPrecision **xc = X.getColumnAccessor();


      bool updated = false;
      DenseMatrix<TPrecision> Xnew(X.M(), X.N());
      DenseVector<TPrecision> x(X.M());
      DenseVector<TPrecision> y(Y.M());
      
      for(int i=0; i<X.N(); i++){

        //DenseVector<TPrecision> y(Y.M(), yc[i]);
        Linalg<TPrecision>::ExtractColumn(Y, i, y);

        bool xUpdate = f(y, x, stepX, risk, penalty);
        updated = updated || xUpdate;

        Linalg<TPrecision>::SetColumn(Xnew, i, x);
        //Linalg<TPrecision>::SetColumn(X, i, x);
      }

      X.deallocate();
      X=Xnew;

      x.deallocate();
      y.deallocate();

      return updated;
    };



    bool orthogonalize(FortranLinalg::DenseVector<TPrecision> &x,
        FortranLinalg::DenseVector<TPrecision> &y, TPrecision stepX, Risk risk =
        MSE, Penalty penalty=NONE){ 
      
      using namespace FortranLinalg;


      DenseVector<TPrecision> gx(X.M());
      Objective obj = objective(x, y, risk, penalty);
      Objective prev = obj;
      prev.total += 1;
      int n=0;

      double epsilon = 0.000001;
      //for(int k=0; k<2 && obj.total - prev.total < 0 ; k++){
      while(obj.total - prev.total < 0 ){
        //vary each coordinate
        for(unsigned int j=0; j<gx.N(); j++){
          x(j) += epsilon;

          Objective obj2 = objective(x,y, risk, penalty);

          gx(j) = ( obj.total -obj2.total ) /epsilon;

          x(j) -= epsilon;
          //clamp to boundary
          if(x(j) + gx(j) > xMax(j)){
            gx(j) = xMax(j) - x(j);
          }
          if(x(j) +gx(j)  < xMin(j)){
            gx(j) = xMin(j) - x(j);
          }

        }

        Linalg<TPrecision>::Normalize(gx);
        Linalg<TPrecision>::Scale(gx, stepX, gx);
        TPrecision scale = 1;
        for(unsigned int j=0; j<gx.N(); j++){
          scale = std::min(scale, std::min(fabs(xMin(j) - x(j)), fabs(xMax(j) - x(j))  )/ gx(j) );
        }
        if(scale < 1){
          Linalg<TPrecision>::Scale(gx, scale, gx);
        }

        //Add gradient 
        Linalg<TPrecision>::Add(x, gx, x);

        prev = obj;
        obj = objective(x,y, risk, NONE);
          
        n++;
      }
        
      Linalg<TPrecision>::Subtract(x, gx, x);
      
      gx.deallocate();

      return n>1;
    };











    //numerical gradient for paramater z_r of the coordinate mapping
    //epsilon = finite difference delta
    void numGradX(int r, FortranLinalg::DenseVector<TPrecision> &gx, TPrecision epsilon, Risk
        risk, Penalty penalty, int nPoints){
      using namespace FortranLinalg;

      //DenseVector<int> sample=Linalg<int>::ExtractColumn(KNNX,r);

      
      DenseVector<int> sample(1);
      sample(0) = r;
      //DenseVector<int> sample(nPoints);
      //for(int i=0; i<nPoints; i++){
      //  sample(i) = (int) ( rand.Uniform()*Y.N() );
      //}

      Objective obj1 = objective(risk, penalty, sample);
      //vary each coordinate
      for(unsigned int i=0; i<gx.N(); i++){
        X(i, r) += epsilon;
        
        Objective obj2 = objective(risk, penalty, sample);
        
        gx(i) = ( obj2.total - obj1.total ) / epsilon;
        
        X(i, r) -= epsilon;

        if(X(i, r) + gx(i) > xMax(i)){
          gx(i) = xMax(i) - X(i, r);
        }
        if(X(i, r) +gx(i)  < xMin(i)){
          gx(i) = xMin(i) - X(i, r);
        }
      }

      sample.deallocate();
    

    };






    //Gradient descent 
    bool optimizeX(int nIterations, int nPoints, TPrecision scalingZ,
        TPrecision scalingBW, int verbose=1, Risk risk = ORTHO, Penalty penalty =
        NONE){
      using namespace FortranLinalg;

      
      //---Storage for syncronous updates 
      DenseMatrix<TPrecision> sync(X.M(), X.N());

      //---Do nIterations of gradient descent     
      DenseMatrix<TPrecision> Xtmp(X.M(), X.N());
      DenseMatrix<TPrecision> Xswap;

      //gradient direction
      DenseVector<TPrecision> gx(X.M());     
      if(verbose > 0){
        myprintf( "Start Gradient Descent \n" );
      }

      bool updated = false;
      for(int i=0; i<nIterations; i++){
        Objective objPrev = objective(risk, penalty);

        TPrecision sX = kernelX.getKernelParam()/2;
        //compute gradient for each point
        TPrecision maxL = 0;
        for(unsigned int j=0; j < X.N(); j++){
          //compute gradient
          //gradX(j, gx);
          numGradX(j, gx, 0.000001, risk, penalty, nPoints);


          //Asynchronous updates
          //Linalg<TPrecision>::ColumnAddScale(Z, j, -scaling*sX, gx);


          //store gradient for syncronous updates
          TPrecision l = Linalg<TPrecision>::Length(gx);
          if(maxL < l){
            maxL = l;
          }
          //Linalg<TPrecision>::Scale(gx, 1.f/l, gx);
          for(unsigned int k=0; k<X.M(); k++){
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
        Objective objTmp = objective(risk, penalty);
        b(0, 0) = objTmp.total;

        Linalg<TPrecision>::AddScale(X, -1*s, sync, Xtmp);
        Xswap = X;
        X = Xtmp;
        Xtmp = Xswap;

        objTmp = objective(risk, penalty);
        b(1, 0) = objTmp.total;


        Linalg<TPrecision>::AddScale(Xswap, -2*s, sync, X);
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
          Linalg<TPrecision>::AddScale(Xtmp, h, sync, X);        

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



        Objective obj = objective(risk, penalty); 

        if(!stop){ 
          stop = objPrev.total <= obj.total;
        }

        if(stop){          
          Xswap = Xtmp;
          Xtmp = X;
          X = Xswap;
          break;
        }

        updated=true; 
        objPrev = obj;      
      }


      //cleanup 
      sync.deallocate();
      gx.deallocate();
      Xtmp.deallocate();

      return updated;
    };








    int updateBandwidthX(TPrecision scaling, int verbose, Risk risk, Penalty
        penalty, int nPoints){      
      using namespace FortranLinalg;
      
      if(verbose > 1){
        myprintf( "sigmaX: %f \n" , kernelX.getKernelParam() );
      }

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
     
      if(verbose > 1){
        myprintf( "sigmaX: %f \n" , kernelX.getKernelParam() );
      }
      return updated;
    };






    //Functions for geodesic computations 
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


      DenseVector<TPrecision> xc(X.M());
      DenseMatrix<TPrecision> kr(X.M(), X.M());

      //Linear system
      DenseMatrix<TPrecision> X1(X.N(), X.M() * (X.M()+1) +1);
      DenseMatrix<TPrecision> Y1(X.N(), Y.M());
      DenseVector<TPrecision> W(X.N());
            
      //DenseVector<int> knn =  DenseVector<int>(knnX+1);
      //DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
      //Geometry<TPrecision>::computeKNN(fY, x, knn, knnD, sl2metric);

      //Setup linear system
      for(unsigned int i2=0; i2 < X.N(); i2++){

        int i=i2; //knn(i2);
        //W(i2) = kernelX.f( knnD(i) );
        W(i2) = kernelX.f(x, X, i );

        Linalg<TPrecision>::ExtractColumn(X, i, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i2, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< X.M(); j++){
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
      for(unsigned int j=0; j< X.M(); j++){
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


      DenseVector<TPrecision> xc(X.M());
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractColumn(fY, n);
      DenseMatrix<TPrecision> kr(X.M(), X.M());

      //Linear system
      DenseMatrix<TPrecision> X1(X.N(), X.M() * (X.M()+1) +1);
      DenseMatrix<TPrecision> Y1(lamdafY.N(), Y.M());
      DenseVector<TPrecision> W(X.N());
            
      //Setup linear system
      for(unsigned int i2=0; i2 < X.N(); i2++){

        int i=i2;//KNNX(i2, n);
        W(i2) = kernelX.f(X, n, X, i );

        Linalg<TPrecision>::ExtractColumn(fY, i, xc);
        Linalg<TPrecision>::OuterProduct(xc, xc, kr);

        int jindex = 0;
        X1(i2, jindex) = 1;
        ++jindex;
        for(unsigned int j=0; j< X.M(); j++){
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
      for(unsigned int j=0; j< X.M(); j++){
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
      DenseMatrix<TPrecision> X1(Y.N(), X.M()+1);
      DenseMatrix<TPrecision> Y1(Y.N(), Y.M());
      DenseVector<TPrecision> W(Y.N() );
      
      /*
      DenseVector<int> knn =  DenseVector<int>(knnX+1);
      DenseVector<TPrecision> knnD = DenseVector<TPrecision>(knnX+1);
      Geometry<TPrecision>::computeKNN(fY, x, knn, knnD, sl2metric);
*/

      //Setup linear system
      for(unsigned int i2=0; i2 < X.N(); i2++){
        int i = i2;//knn(i2);
        //W(i2) = kernelX.f( knnD(i2) ); 
        W(i2) = kernelX.f(x, X, i );

        X1(i2, 0) = 1;
        for(unsigned int j=0; j< X.M(); j++){
          X1(i2, j+1) = X(j, i);
        }

        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i2, m) = Y(m, i);
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
      DenseMatrix<TPrecision> X1(Y.N(), X.M()+1);
      DenseMatrix<TPrecision> Y1(Y.N(), Y.M());
      DenseVector<TPrecision> W(Y.N());
      
      DenseVector<TPrecision> x = Linalg<TPrecision>::ExtractColumn(fY, n);
      //Setup linear system
      for(unsigned int i2=0; i2 < X.N(); i2++){
        int i = i2;//KNNX(i2, n);
        W(i2) = kernelX.f(X, n, X, i);
        X1(i2, 0) = 1;
        for(unsigned int j=0; j< X.M(); j++){
          X1(i2, j+1) = fY(j, i);
        }

        for(unsigned int m = 0; m<Y.M(); m++){
          Y1(i2, m) = Y(m, i);
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

      DenseMatrix<TPrecision> sol( X.M()+1, Y.M() );
      for(unsigned int k=0; k< X.M(); k++){
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






}; 


#endif

