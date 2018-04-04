#ifndef ANISTROPICGAUSSIANKERNEL_H
#define ANISTROPICGAUSSIANKERNEL_H

#include "Kernel.h"
#include "Linalg.h"
#include "SymmetricEigensystem.h"
#include "SampleStatistics.h"

template <typename TPrecision>
class AnistropicGaussianKernel : public Kernel<TPrecision, DenseMatrix<TPrecision> >{

  private:
    DenseMatrix<TPrecision> C;
    DenseMatrix<TPrecision> CInv;
    DenseVector<TPrecision> tmp1;
    DenseVector<TPrecision> tmp2;
    TPrecision nc;


  public:
  
  AnistropicGaussianKernel(){
    nc= 0;
  };

  AnistropicGaussianKernel(DenseMatrix<TPrecision> cov){
     setKernelParam(cov);
  };
  

  TPrecision f(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    Linalg<TPrecision>::Subtract(x1, x2, tmp1);
    Linalg<TPrecision>::Multiply(CInv, tmp1, tmp2);
    TPrecision d = Linalg<TPrecision>::Dot(tmp1, tmp2);
    //std::cout << d << std::endl; 
    return exp( -d );
  };



  TPrecision f(Vector<TPrecision> &x1, Matrix<TPrecision> &X2, int i2){
    Linalg<TPrecision>::ExtractColumn(X2, i2, tmp2);
    return f(x1, tmp2);
  };
  


  TPrecision f(Matrix<TPrecision> &X1, int i1, Matrix<TPrecision> &X2, int i2){
    Linalg<TPrecision>::ExtractColumn(X1, i1, tmp1);
    Linalg<TPrecision>::ExtractColumn(X2, i2, tmp2);

    return f(tmp1, tmp2);
  };




  void grad(Vector<TPrecision> &x1, Vector<TPrecision> &x2, Vector<TPrecision> &g){
    gradf(x1, x2, g); 
  };



  TPrecision gradf(Vector<TPrecision> &x1, Vector<TPrecision> &x2,
      Vector<TPrecision> &g){
    
    Linalg<TPrecision>::Subtract(x1, x2, g);
    TPrecision val = f(x1, x2);
    for(int i=0; i<g.N(); i++){
      g(i) *= -val;
    }
    return val;
  };
  



  DenseMatrix<TPrecision> gradKernelParam(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    throw "Method not implemented";
    return C;         
  };
    



  void setKernelParam(DenseMatrix<TPrecision> &cov){ 
    tmp1.deallocate();
    tmp2.deallocate();
    tmp1 = DenseVector<TPrecision>(cov.M());
    tmp2 = DenseVector<TPrecision>(cov.M()); 
    
    C.deallocate();
    C = cov;
    SymmetricEigensystem<TPrecision> se(C);
    for(int i=0; i< C.N(); i++){
      if(se.ew(i) > 0.001 ){
        se.ew(i) = 1.0 / sqrt(se.ew(i));
      }
      else{
        se.ew(i) = 1.0 / sqrt(0.001);
      }
      Linalg<TPrecision>::ScaleColumn(se.ev, i, se.ew(i) );
    }
    
    CInv.deallocate();
    CInv = Linalg<TPrecision>::Multiply(se.ev, se.ev, false, true);

    se.ew.deallocate();
    se.ev.deallocate();

  };


   

};

#endif
