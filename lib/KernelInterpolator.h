#ifndef KERNELINTERPOLATOR_H
#define KERNELINTERPOLATOR_H

#include <math.h>

#include "SquaredEuclideanMetric.h"
#include "Geometry.h"
#include "DenseVector.h"
#include "DenseMatrix.h"


template<typename TPrecision>
class KernelInterpolator{
      
  public:
    //x needs to be ordered
    KernelInterpolator(FortranLinalg::DenseVector<TPrecision> &xin, int dim):x(xin){
        tmp = FortranLinalg::DenseVector<TPrecision>(dim);
        delta = FortranLinalg::DenseVector<TPrecision>(dim);
    };


    KernelInterpolator(FortranLinalg::DenseVector<TPrecision> &xin, FortranLinalg::DenseMatrix<TPrecision>
        &Yin):x(xin), Y(Yin){
        tmp = FortranLinalg::DenseVector<TPrecision>(Y.M());
        delta = FortranLinalg::DenseVector<TPrecision>(Y.M());
    };

    


    void evaluate(TPrecision &xe, FortranLinalg::DenseVector<TPrecision> &out){
      FortranLinalg::Linalg<TPrecision>::Zero(out);

      
      TPrecision d1 = 0;
      TPrecision d2 = 0;
      TPrecision wsum = 0;
      TPrecision w = 0;
      FortranLinalg::Linalg<TPrecision>::Zero(out);
      for(unsigned int i=1; i < x.N(); i++){
          d1 = x(i-1) - xe;
          d1 = sqrt(d1*d1);
          d2 = x(i) - xe;
          d2 = sqrt(d2*d2);
          w = 1.0/(1-expf(d1*d2));
          
          if(std::numeric_limits<TPrecision>::infinity() == w){
            if(d1 < d2){
              FortranLinalg::Linalg<TPrecision>::ExtractColumn(Y, i-1, out);
            }
            else{
              FortranLinalg::Linalg<TPrecision>::ExtractColumn(Y, i, out);
            }
            return;
          }
          wsum += w; 
          FortranLinalg::Linalg<TPrecision>::Subtract(Y, i, Y, i-1, delta);
          FortranLinalg::Linalg<TPrecision>::AddScale(Y, i-1, xe - x(i-1), delta, tmp);  
          FortranLinalg::Linalg<TPrecision>::AddScale(out, w, tmp, out);
      }

      FortranLinalg::Linalg<TPrecision>::Scale(out, 1.0/wsum, out);
    };

    
    
    void setData(FortranLinalg::DenseMatrix<TPrecision> Yin){ 
      Y = Yin;
    };


  private:
    SquaredEuclideanMetric<Precision> metric;
 
    FortranLinalg::DenseMatrix<TPrecision> Y;
    FortranLinalg::DenseVector<TPrecision> x;
    FortranLinalg::DenseVector<TPrecision> tmp;
    FortranLinalg::DenseVector<TPrecision> delta;

};


#endif
