#ifndef EPANECHNIKOVKERNEL_H
#define EPANECHNIKOVKERNEL_H

#include "Kernel.h"
#include "EuclideanMetric.h"

template <typename TPrecision>
class EpanechnikovKernel : public Kernel<TPrecision, TPrecision>{

  private:
    EuclideanMetric<TPrecision> metric;
    TPrecision support;
    TPrecision n;
    unsigned int dim;


  public:
 
    EpanechnikovKernel(unsigned int dimension = 1){
      dim = dimension;
      support = 1;
      n = pow(3.0/4.0, dim);

    } 

    EpanechnikovKernel(TPrecision suprt, unsigned int dimension = 1){
      dim = dimension;
      support = suprt*suprt;
      n = pow(3.0/4.0, dim);
    }
    

  TPrecision f(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    TPrecision val = metric.distanceSquared(x1, x2);
    return f(val);
  };

  TPrecision f(Vector<TPrecision> &x1, Matrix<TPrecision> &X2, int i2){
    TPrecision val =metric.distanceSquared(X2, i2, x1 );
    return f(val);

  };
  
  TPrecision f(Matrix<TPrecision> &X1, int i1, Matrix<TPrecision> &X2, int i2){
    TPrecision val = metric.distanceSquared(X1, i1, X2, i2 );
    return f(val);
  };


  TPrecision f(TPrecision dSquared){
    if(dSquared > support){
      return 0;
    }
    dSquared = (1 - dSquared / support);
    return n * pow(dSquared, dim);

  };




  void grad(Vector<TPrecision> &x1, Vector<TPrecision> &x2, Vector<TPrecision> &g){
    gradf(x1, x2, g); 
  };



  TPrecision gradf(Vector<TPrecision> &x1, Vector<TPrecision> &x2,
      Vector<TPrecision> &g){
   
    Linalg<TPrecision>::Subtract(x1, x2, g);
    TPrecision d = 0;
    for(unsigned int i=0; i<g.N(); i++){
      d += g(i)*g(i);
    } 
    if(d > support){
      Linalg<TPrecision>::Set(g, 0);
      return 0;
    }
    TPrecision tmp = 1-d/support;
    TPrecision tmpP = n*pow(tmp, dim-1);
    TPrecision val = -2*tmpP*dim/support;
    Linalg<TPrecision>::Scale(g, val, g);
    return tmpP*tmp;
  };    
  
  
  
  TPrecision gradf(Matrix<TPrecision> &x1, int i1, Matrix<TPrecision> &x2, int
        i2, Vector<TPrecision> &g){
  
    Linalg<TPrecision>::Subtract(x1, i1, x2, i2, g);
    TPrecision d = 0;
    for(unsigned int i=0; i<g.N(); i++){
      d += g(i)*g(i);
    } 
    if(d > support){
      Linalg<TPrecision>::Set(g, 0);
      return 0;
    }
    TPrecision tmp = 1-d/support;
    TPrecision tmpP = n*pow(tmp, dim-1);
    TPrecision val = -2*tmpP*dim/support;
    Linalg<TPrecision>::Scale(g, val, g);
    return tmpP*tmp;
  };


  
  
  TPrecision gradKernelParam(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    TPrecision val = metric.distanceSquared(x1,x2);
    if(val > support){
      return 0;
    }

    return n * dim *pow(1 - val/support, dim-1) * 2 * val/(support*sqrt(support)); 
  };

  void setKernelParam(TPrecision &param){
    support = param*param;

  };

  TPrecision getKernelParam(){
    return sqrt(support);
  };

};

#endif
