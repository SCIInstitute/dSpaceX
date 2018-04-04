#ifndef DEFORMATIONKERNELREGRESSION_H
#define DEFORMATIONKERNELREGRESSION_H

#include <math.h>
#include "Linalg.h"
#include "Geometry.h"
#include "SquaredEuclideanMetric.h"
#include "DenseMatrix.h"
#include "DenseVector.h"
#include "ElasticWarp.h"
#include "ImageVectorConverter.h"


template<typename TPrecision, typename TImage>
class DeformationKernelRegression{

  public:

    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;

    DeformationKernelRegression( FortranLinalg::DenseMatrix<TPrecision> &data,
        FortranLinalg::DenseMatrix<TPrecision> &values, int nn, ImagePointer m, int
        nIterations, TPrecision step, TPrecision alpha, TPrecision eps,
        TPrecision lambda, TPrecision lambdaInc, TPrecision
        lambdaIncT, TPrecision resSigma, int nres, bool lout = false) 
      : X(data), y(values), knn(nn), nIter(nIterations), mask(m)  {

      warp.setAlpha(alpha);
      warp.setMaximumIterations(1);
      warp.setMaximumMotion(step);
      warp.setEpsilon(eps);
      warp.setLambda(lambda);
      warp.setLambdaIncrease(lambdaInc);
      warp.setLambdaIncreaseThreshold(lambdaIncT);

      sigma = resSigma;
      nRes = nres;
      leaveout = lout;

    };






   FortranLinalg::DenseVector<TPrecision> evaluate(FortranLinalg::Vector<TPrecision> &yValue){
      using namespace FortranLinalg;
      //start index for knn if leaveoneout
      int start = 0;
      if(leaveout){
        start = 1;
      }

      //original images
      std::vector<ImagePointer> images(knn+start); 

      //knn and weights
      DenseVector<TPrecision> weights(knn + start);
      DenseVector<int> knns(knn + start);
      DenseVector<TPrecision> knnDists(knn + start); 
      TPrecision wsum = 0;
      Geometry<TPrecision>::computeKNN(y, yValue, knns, knnDists, labelMetric);

      //compute variance
      TPrecision var = 0;
      for(int i=start; i<knn+start; i++){
        var+=knnDists(i);
      }
      var /= knn;
      if(var == 0) var =1;

      //Warp fields
      VImage *warpFields[knn+start];
      std::cout << "ImageIndex: ";
      for(int i=0; i < knn + start; i++){
        warpFields[i] = NULL;
      }


      ImagePointer *pyramid[knn];
      ImagePointer *pyramidMask =  warp.downsample(mask, nRes, sigma);


      //images in use and weights    
      ImageVectorConverter<TImage> converter(mask);
      for(int i = start; i < knn+start; i++){
         weights(i) = exp(- knnDists(i) / var );
         wsum += weights(i);
         images[i] = converter.createImage(X, knns(i));
         std::cout << knns(i)<<":"<<weights(i) << " "; 
      }
      std::cout << "sum of weights: " << wsum << std::endl;

      for(int i = start; i < knn+start; i++){ 
        pyramid[i] = warp.downsample(images[i], nRes, sigma);
      }

      //normalize weights and compute inital target
      converter = ImageVectorConverter<TImage>(pyramidMask[nRes-1]);

      DenseVector<TPrecision> v( converter.getD() );
      DenseVector<TPrecision> out(v.N());
      Linalg<TPrecision>::Zero(out);

      for(int i = start; i < knn+start; i++){
        weights(i) /= wsum;
        converter.extractVector(pyramid[i][nRes-1], v);
        Linalg<TPrecision>::AddScale(out, weights(i), v, out);
      }
      std::cout << std::endl;

      ImagePointer target = converter.createImage(out);
      ImagePointer tmp1 = converter.createImage();
      
      //debug
      //ImageIO<Image>::saveImage(target, "init.mhd");
 
      
      TPrecision lambda = warp.getLambda();
      TPrecision lambdaIncThreshold = warp.getLambdaIncreaseThreshold();
      TPrecision epsilon = warp.getEpsilon();

      //Do warping fro all scales      
      for(int s=1; s <= nRes; s++){
	      warp.setLambda(lambda * pow(10, nRes-s+1));
        warp.setLambdaIncreaseThreshold( lambdaIncThreshold / pow(10, nRes-s+1));
        //Warp to target and update target
        TPrecision rmsePrev = std::numeric_limits<TPrecision>::max();
        for(int n=0; n < nIter; n++){
        //warp.setMaximumIterations(nIter);
        
          //reset new target
          Linalg<TPrecision>::Zero(out);

            
          //transform to target image and update new target
          TPrecision rmse = 0;
          for(int i=start; i < knn+start; i++){
            VImage *tmpWarp = warp.warp(pyramid[i][nRes-s], target, converter.getMask(), warpFields[i] ); 
            delete warpFields[i];
            warpFields[i] = tmpWarp;
           
            rmse +=  weights(i) * warp.getRMSE();
           
            //add warped to new target
            ImageTransform::Transform(tmp1, pyramid[i][nRes-s], warpFields[i]);
            converter.extractVector(tmp1, v);
            Linalg<TPrecision>::AddScale(out, weights(i), v, out); 
          }


          


          

          //update target
          converter.fillImage(out, target); 

          std::cout << "--- Average rmse: " << rmse << " ---" << std::endl;

          if(rmse < epsilon){
            break;
          }

          if(rmsePrev - rmse < warp.getLambdaIncreaseThreshold()){
            if(warp.getLambdaIncrease() == 0){
              break;
            }
            warp.setLambda(warp.getLambda() + warp.getLambdaIncrease());
          }
          rmsePrev = rmse;
        }
        //Warping for one scale done


         if(s < nRes){          
           
          typedef typename VImage::ITKVectorImagePointer ITKVectorImagePointer;
          typedef typename VImage::ITKVectorImage ITKVectorImage;
          ITKVectorImagePointer wtmp = warpFields[start]->toITK();
          std::stringstream ss;
          ss << "w_" << s <<"before.mhd";
          ImageIO< ITKVectorImage >::saveImage(wtmp, ss.str());
          //upsample warps
          for(int i=start; i < knn+start; i++){
           warp.upsample(warpFields[i]);
          }          
          wtmp = warpFields[start]->toITK();
          std::stringstream ss2;
          ss2 << "w_" << s <<"after.mhd";
          ImageIO< ITKVectorImage >::saveImage(wtmp, ss2.str());


        
         //compute new target for this resolution from current warps
         converter = ImageVectorConverter<TImage>(pyramidMask[nRes-s-1]);

    
          tmp1 = converter.createImage();
          out.deallocate();
          out = DenseVector<TPrecision>(converter.getD());
          v.deallocate();
          v = DenseVector<TPrecision>(converter.getD());
          Linalg<TPrecision>::Zero(out);
          for(int i = start; i < knn+start; i++){
            ImageTransform::Transform(tmp1, pyramid[i][nRes-s-1], warpFields[i]);
            converter.extractVector(tmp1, v);
            Linalg<TPrecision>::AddScale(out, weights(i), v, out);
          }
          target = converter.createImage(out);
        }

      }

      warp.setLambda(lambda);
      warp.setEpsilon(epsilon);


      //cleanup
      for(int i=start; i<knn+start; i++){
        delete[] pyramid[i];
        delete warpFields[i];
      }
      delete[] pyramidMask;
      
      return out;
    };



     FortranLinalg::DenseMatrix<TPrecision> getX(){
       return X;
     }
     FortranLinalg::DenseMatrix<TPrecision> getY(){
       return y;
     }

  private:

    
    typedef SimpleWarp<Image> Warp;
    typedef typename Warp::VImage VImage;
    typedef typename Warp::ImageTransform ImageTransform;

    FortranLinalg::DenseMatrix<TPrecision> X;
    FortranLinalg::DenseMatrix<TPrecision> y;

    int knn;
    bool leaveout;
    TPrecision sigma;
    int nRes;

    SquaredEuclideanMetric<TPrecision> labelMetric;


    Warp warp;
    int nIter; 

    ImagePointer mask;
};


#endif
