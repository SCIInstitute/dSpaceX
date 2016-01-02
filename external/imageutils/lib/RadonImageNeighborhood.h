#ifndef RADONIMAGENEIGHBORHOOD_H
#define RADONIMAGENEIGHBORHOOD_H

#include "DenseVector.h"
#include "Matrix.h"

#include "ImageIO.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

#include "itkLinearInterpolateImageFunction.h"


template<typename TImage>
class RadonImageNeighborhood{

  public:
    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::ConstPointer ImageConstPointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename Image::PixelType Precision;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;
    typedef typename Image::SpacingType ImageSpacing;
 
    
    typedef typename itk::ImageRegionIterator<Image> ImageRegionIterator;
    typedef typename itk::ImageRegionConstIterator<Image> ImageRegionConstIterator;
    typedef typename itk::ImageRegionIteratorWithIndex<Image> ImageRegionIteratorWithIndex;



    RadonImageNeighborhood(ImagePointer maskImage){
      mask = maskImage;
      spacing = mask->GetSpacing();

      bounds = findBoundingBox(mask);
      ImageRegionIterator maskIt(mask, bounds);
      D = 0;
      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt){
        Precision pixel = maskIt.Get();
        if(pixel != 0){
          D++;
        }    
      }
    };

    ImagePointer getMask(){
      return mask;
    };


    ImageRegion getBounds(){
      return bounds;
    };


    int getD(){
      return D;
    };



    DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, int radius,
        int nPhi, int nSamples, int maskIndex = 1){
      ImageRegion region = image->GetLargestPossibleRegion();
      typename ImageRegion::SizeType size = region.GetSize();
      typename ImageRegion::IndexType index = region.GetIndex();
      for(int i=0; i<size.GetSizeDimension(); i++){
        size[i] -= 2*radius;
        index[i] += radius;
      }
      region.SetSize(size);
      region.SetIndex(index);
      return extractNeighborhoods(image, radius, region, nPhi, nSamples,
          maskIndex);
    };



    DenseMatrix<Precision> extractAllNeighborhoods(ImagePointer image, int 
        radius, int nPhi, int nSamples){
      ImageRegion region = image->GetLargestPossibleRegion();
      typename ImageRegion::SizeType size = region.GetSize();
      typename ImageRegion::IndexType index = region.GetIndex();
      for(int i=0; i<size.GetSizeDimension(); i++){
        size[i] -= 2*radius;
        index[i] += radius;
      }
      region.SetSize(size);
      region.SetIndex(index);
      return extractAllNeighborhoods(image, radius, region, nPhi, nSamples);
    };


      
    DenseMatrix<Precision> extractAllNeighborhoods(ImagePointer image, int r,
        ImageRegion region, int nPhi, int nSamples){
      
      ImageRegionIterator it(image, region);
      
      typename ImageRegion::SizeType size = region.GetSize();
      int npixels = 1;
      for(int i=0; i < size.GetSizeDimension(); i++){
        npixels *= size[i];
      }

      DenseMatrix<Precision> n(nPhi, npixels);

      LinearInterpolatePointer lip = LinearInterpolate::New();
      lip->SetInputImage(image);
       
      int index = 0;
      for( ; !it.IsAtEnd(); ++it ){        
         ImageIndex center = it.GetIndex();
         radon(lip, center, r, nPhi, nSamples, n, index);
         index++;
      }

      return n;
    };



    DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, int r,
        ImageRegion region, int nPhi, int nSamples, int maskIndex = 1){
      

      ImageRegionIterator it(image, region);
      
      ImageRegionIterator maskIt(mask, region); 
      maskIt.GoToBegin();

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          npixels++;
        }
      }
      maskIt.GoToBegin();


      DenseMatrix<Precision> n(nPhi, npixels);
      
      LinearInterpolatePointer lip = LinearInterpolate::New();
      lip->SetInputImage(image);

      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get() == maskIndex;          
        if(usePixel != 0 ){
          ImageIndex center = it.GetIndex();
          radon(lip, center, r, nPhi, nSamples, n, index);
          index++;
        }
      }

      return n;
    };


    ImageRegion findBoundingBox(ImagePointer image){
      ImageIndex min;
      min.Fill(100000);
      ImageIndex max;
      max.Fill(0);
  
      ImageRegionIteratorWithIndex it(image, image->GetLargestPossibleRegion());
      for(it.GoToBegin(); !it.IsAtEnd(); ++it){
        Precision pixel = it.Get();
        if(pixel != 0){
          ImageIndex index = it.GetIndex();
          for(long i=0; i<index.GetIndexDimension(); i++){
            if(max[i] < index[i]){
              max[i] = index[i];
            }
            if(min[i] > index[i]){
              min[i] = index[i];
            }
          }
        }    
      }
  
      ImageSize size;
      for(long i=0; i < ImageSize::GetSizeDimension(); i++){
        size[i] = max[i] - min[i] + 1;
      }  

      ImageRegion bound(min, size);
      return bound;
   };




private:
     int D;
     ImagePointer mask;
     ImageSpacing spacing;
     ImageRegion bounds;

     typedef typename itk::LinearInterpolateImageFunction<Image, double>
       LinearInterpolate;
     typedef typename LinearInterpolate::Pointer LinearInterpolatePointer;
     typedef typename LinearInterpolate::ContinuousIndexType ImageContinuousIndex;   

     void radon(LinearInterpolatePointer ip, ImageIndex center, int radius, int
         nphi, int nsamples, DenseMatrix<double> m, int index){
        
       double cVal = ip->EvaluateAtIndex(center);

       for(int i = 0; i < nphi; i++){
         double alpha = M_PI * i/(double)nphi;
         double sum = cVal;
         for(int j = 0; j < nsamples; j++){
           ImageContinuousIndex ii1 = center;
           ImageContinuousIndex ii2 = center;
           itk::Vector<double, 2 > off;
           double d = radius * (j+1) / (double)nsamples;
           off[0] = d * cos(alpha);
           off[1] = d * sin(alpha);
           ii1 += off;
           ii2 -= off;
           sum += ip->EvaluateAtContinuousIndex(ii1);
           sum += ip->EvaluateAtContinuousIndex(ii2);
         }
         m(i, index) = sum;
       }
     
     };


};


#endif
