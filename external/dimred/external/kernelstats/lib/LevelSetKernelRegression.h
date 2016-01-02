#ifndef LEVELSETKERNELREGRESSION_H
#define LEVELSETKERNELREGRESSION_H

#include <math.h>

#include "FirstOrderLevelSetMetric.h"
#include "Geometry.h"
#include "Kernel.h"
#include "ImageIO.h"
#include "DenseVector.h"
#include "DenseMatrix.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "GaussianKernel.h"

#define KERNEL_CUTOFF 0.00001

template<typename TPrecision, typename TImage>
class LevelSetKernelRegression{

  private:

    typedef typename TImage::Pointer TImagePointer;

    typedef typename itk::GradientMagnitudeImageFilter<TImage, TImage>
      GradientMagnitudeFilter;
    typedef typename GradientMagnitudeFilter::Pointer
      GradientMagnitudeFilterPointer;

    typedef typename itk::ImageRegionConstIterator<TImage> ImageRegionConstIterator;
    typedef typename itk::ImageRegionIterator<TImage> ImageRegionIterator;

      
  public:
    LevelSetKernelRegression(){};

    LevelSetKernelRegression(DenseVector<TImagePointer> &y, DenseMatrix<TPrecision>
        &x, GaussianKernel<TPrecision> &k):X(x), Y(y), kernel(k){
        Yit = DenseVector<ImageRegionIterator>(Y.N());  
        Yg = DenseVector<TImagePointer>(Y.N());
	      for(unsigned int i=0; i<Y.N(); i++){
	        Yit(i) = ImageRegionIterator(Y(i), Y(i)->GetLargestPossibleRegion());        
 	        GradientMagnitudeFilterPointer gm = GradientMagnitudeFilter::New();
          gm->SetInput(Y(i));
          gm->Update();
          Yg(i) = gm->GetOutput();
	      }
        nIter = 100;
        step = 1;
        stopping = 1;
    };


    TImagePointer evaluate(Vector<TPrecision> &x, int lo = -1){
      TImagePointer out = ImageIO<TImage>::copyImage(Y(0));
      evaluate(x, out);
      return out;
    };
  
 
    void evaluate(Vector<TPrecision> &x, TImagePointer &out,  int lo =
        -1){
      TPrecision wsum = 0;
      DenseVector<TPrecision> w(X.N());
      TPrecision dist = 0;
      TPrecision cutoff = 2*kernel.getKernelParam();
      cutoff = cutoff * cutoff;
      for(int i=0; i < X.N(); i++){
        dist = l2s.distance(X, i, x);
        if(dist < cutoff && i != lo){
          w(i) = kernel.f(dist);
      	}
        else{
          w(i) = 0;
        }
        wsum += w(i);
      }
      for(unsigned int i=0; i < X.N(); i++){
        w(i) /= wsum;
      }


      //Initalize
      ImageRegionIterator it(out, out->GetLargestPossibleRegion());
      resetIterators();
      for(; !it.IsAtEnd(); ++it){
        TPrecision tmp = 0;
        for(unsigned int i=0; i<X.N(); i++){
	        if(w(i) > 0){
            tmp += w(i) * Yit(i).Get();
            ++Yit(i);  
	        }
	      }
        it.Set(tmp);
      }

      //grad descent
      TPrecision change = stopping+1;
      unsigned int n = 0; 
      TImagePointer dtmp = ImageIO<TImage>::copyImage(out);
      while(fabs(change) > stopping && n < nIter){
        //Gradient magnitude for out/moving image
        GradientMagnitudeFilterPointer gm = GradientMagnitudeFilter::New();
        gm->SetInput(out);
        gm->Update();
        ImagePointer gout = gm->GetOutput();

        change = 0;
        for(unsigned int i=0; i<X.N(); i++){
   	      if(w(i) > 0){
            metric.derivativeX2(Y(i), Yg(i), out, gout, dtmp);
	          ImageRegionIterator dIt(dtmp, dtmp->GetLargestPossibleRegion());
            it.GoToBegin();
            for(; !dIt.IsAtEnd(); ++it, ++dIt){
              TPrecision tmp =  step * w(i) * dIt.Get();
  	          change += tmp;
	            it.Set(it.Get() - tmp);
	          }
          }
        }
        n++;
        //std::cout << change << std::endl;
      }

    };

    void setStep(TPrecision s){
      step =s;
    };

    void setNIter(unsigned int n){
      nIter = n;
    };

    void setStopping(TPrecision stop){
      stopping = stop;
    };

    void setEpsilon(TPrecision e){
      metric.setEpsilon(e);
    };    
    
    void setLevelSetMetric(FirstOrderLevelSetMetric<TPrecision, TImage> m){
      metric = m;
    };

    void setKernelSigma(TPrecision sigma){
      kernel.setKernelParam(sigma);
    };

    FirstOrderLevelSetMetric<TPrecision, TImage> getLevelSetMetric(){
      return metric;
    };

  private:
    FirstOrderLevelSetMetric<TPrecision, TImage> metric;
    SquaredEuclideanMetric<TPrecision> l2s;
 
    DenseMatrix<TPrecision> X;
    DenseVector<TImagePointer> Y;
    DenseVector<TImagePointer> Yg;
    DenseVector< ImageRegionIterator > Yit;


    GaussianKernel<TPrecision> kernel;

    unsigned int nIter;
    TPrecision step;
    TPrecision stopping;

    void resetIterators(){
      for(unsigned int i=0; i<Y.N(); i++){
	      Yit(i).GoToBegin();
      }
    };

    void incrementIterators(){
      for(int i=0; i<Y.N(); i++){
	      ++Yit(i);
      }
    };
};


#endif
