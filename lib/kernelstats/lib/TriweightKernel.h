#ifndef TRIWEIGHTKERNEL_H
#define TRIWEIGHTKERNEL_H

#include "Kernel.h"
#include "EuclideanMetric.h"

template <typename TPrecision>
class TriweightKernel : public Kernel<TPrecision, TPrecision>{

  private:
    EuclideanMetric<TPrecision> metric;
    TPrecision support;


  public:
  

    TriweightKernel(TPrecision suprt){
      support = suprt*suprt;
    }
    
  
  TPrecision f(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    TPrecision val = metric.distance(x1, x2);
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


  TPrecision f(TPrecision distance){
    if(distance > support){
      return 0;
    }
    distance = (support - distance) / support;
    return distance*distance*distance;

  };

  void grad(Vector<TPrecision> &x1, Vector<TPrecision> &x2, Vector<TPrecision> &g){
    gradf(x1, x2, g); 
  };

  TPrecision gradf(Vector<TPrecision> &x1, Vector<TPrecision> &x2,
      Vector<TPrecision> &g){
   
    TPrecision d = metric.distanceSquared(x1, x2);
    if(d > support){
      Linalg<TPrecision>::Set(g, 0);
      return 0;
    }
    TPrecision val = (support - d)/support;

    for(int i=0; i<g.N(); i++){
      g(i) =  -6 * val * val * ( x1(i) - x2(i) ) / support;
     }
    return val*val*val;

  };
  
  
  TPrecision gradKernelParam(Vector<TPrecision> &x1, Vector<TPrecision> &x2){
    exit(-3);
    TPrecision val = metric.distanceSquared(x1,x2);
    if(val > support){
      return 0;
    };

    return -3 * val / (support * sqrt(support) ); 
  };

  void setKernelParam(TPrecision param){
    support = param*param;
  };

};

#endif
