#ifndef COLORIMAGENEIGHBORHOOD_H
#define COLORIMAGENEIGHBORHOOD_H

#include "DenseVector.h"
#include "Matrix.h"

#include "ImageIO.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"
#include "itkPeriodicBoundaryCondition.h"
#include <itkZeroFluxNeumannBoundaryCondition.h>

template<typename TImage, typename TMaskImage>
class ColorImageNeighborhood{

  public:
    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::ConstPointer ImageConstPointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;
    typedef typename Image::SpacingType ImageSpacing;
    typedef typename Image::PixelType ImagePixel;
 
    
    typedef typename itk::ImageRegionIterator<Image> ImageRegionIterator;
    typedef typename itk::ImageRegionConstIterator<Image> ImageRegionConstIterator;
    typedef typename itk::ImageRegionIteratorWithIndex<Image> ImageRegionIteratorWithIndex;
    typedef typename itk::NeighborhoodIterator<Image,
            itk::ZeroFluxNeumannBoundaryCondition<Image> > NeighborhoodIterator;
   typedef typename NeighborhoodIterator::RadiusType Radius;
    
   
   typedef TMaskImage MaskImage;
    typedef typename MaskImage::Pointer MaskImagePointer;
    typedef typename MaskImage::ConstPointer MaskImageConstPointer;
    typedef typename MaskImage::IndexType MaskImageIndex;
    typedef typename MaskImage::RegionType MaskImageRegion;
    typedef typename MaskImageRegion::SizeType MaskImageSize;
    typedef typename MaskImageSize::SizeValueType MaskImageSizeValue;
    typedef typename MaskImage::SpacingType MaskImageSpacing;
 
    
    typedef typename itk::ImageRegionIterator<MaskImage> MaskImageRegionIterator;
    typedef typename itk::ImageRegionConstIterator<MaskImage> MaskImageRegionConstIterator;
    typedef typename itk::ImageRegionIteratorWithIndex<MaskImage> MaskImageRegionIteratorWithIndex;
    typedef typename itk::NeighborhoodIterator<MaskImage,
            itk::ZeroFluxNeumannBoundaryCondition<MaskImage> > MaskNeighborhoodIterator;


    ColorImageNeighborhood(MaskImagePointer maskImage){
      mask = maskImage;
      spacing = mask->GetSpacing();

      bounds = findBoundingBox(mask);
      MaskImageRegionIterator maskIt(mask, bounds);
      D = 0;
      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt){
        Precision pixel = maskIt.Get();
        if(pixel != 0){
          D++;
        }    
      }
    };

    MaskImagePointer getMask(){
      return mask;
    };


    MaskImageRegion getBounds(){
      return bounds;
    };


    int getD(){
      return D;
    };


    DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, Radius radius,
        int maskIndex = 1){
      return extractNeighborhoods(image, radius,
          image->GetLargestPossibleRegion(), maskIndex);
    };




    DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, Radius r,
        ImageRegion region, int maskIndex = 1){
      
      int nd = 1;
      for(unsigned int i=0; i<Image::GetImageDimension(); i++){
        nd *= (2 * r[i] + 1);
      }
      NeighborhoodIterator it(r, image, region);
      
      MaskImageRegionIterator maskIt(mask, region); 
      maskIt.GoToBegin();

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          npixels++;
        }
      }
      maskIt.GoToBegin();


      DenseMatrix<Precision> n(nd*Image::PixelType::Dimension, npixels);

      int PD = Image::PixelType::Dimension;
      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get() == maskIndex;
        if(usePixel != 0 ){
          for(int j=0; j<nd; j++){
            ImagePixel p= it.GetPixel(j);
            for(int k=0; k< PD; k++){
              n(j*PD+k, index) = p[k];
            }
          }
          index++;
        }
      }

      return n;
    };





    DenseVector<Precision> extract(ImagePointer image, int maskIndex = 1){
      return extract(image, image->GetLargestPossibleRegion(), maskIndex);
    };




    ImagePointer createImage(){
      return createImage(mask->GetLargestPossibleRegion());
    }



    ImagePointer createImage(ImageRegion region ){
      ImagePointer image = Image::New();
      image->SetRegions(region);
      image->SetSpacing(spacing);
      image->Allocate();
      return image; 
    };

    
    ImagePointer createImage(DenseMatrix<Precision> &n, Radius r,Precision outsideValue = 0){
     ImagePointer im = createImage(mask->GetLargestPossibleRegion());
     fillImage(n, im, r, outsideValue);
     return im;

    }



 
    void fillImage(DenseMatrix<Precision> &n, ImagePointer image, Radius r, 
                   Precision outsideValue = 0){
      
      
      int nd = 1;
      for(unsigned int i=0; i<Image::GetImageDimension(); i++){
        nd *= (2 * r[i] + 1);
      }
      
     
      MaskImagePointer counter = ImageIO<MaskImage>::copyImage(mask);
      
      MaskNeighborhoodIterator cit(r, counter, counter->GetLargestPossibleRegion());       
      NeighborhoodIterator it(r, image, image->GetLargestPossibleRegion());       
      MaskImageRegionIterator maskIt(mask, image->GetLargestPossibleRegion()); 

      for( ;!cit.IsAtEnd(); ++cit, ++it){
        cit.SetCenterPixel(0);
        ImagePixel p;
        p.Fill(0);
        it.SetCenterPixel(p);
      }

      
      it.GoToBegin();
      maskIt.GoToBegin();
      cit.GoToBegin();
      int index = 0;
      bool ignore = false;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it, ++cit){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0 ){ 
                
          
          int nindex = 0;
          for(int j=0; j<nd; j++){
            cit.SetPixel(j, cit.GetPixel(j)+1, ignore);
            ImagePixel  p;
            for(int k=0; k < ImagePixel::Dimension; k++){
              p[k] = n(nindex, index);
              nindex++;
            }
            it.SetPixel(j, it.GetPixel(j) + p, ignore);
          }
          ++index;
        }else{
          ImagePixel p;
          p.Fill(outsideValue);
          it.SetCenterPixel(p);
          cit.SetCenterPixel(1);          
        }
      }

      it.GoToBegin();
      cit.GoToBegin();
      for( ; !it.IsAtEnd(); ++it, ++cit){
        if(cit.GetCenterPixel() > 0 ){ 
          ImagePixel p = it.GetCenterPixel();
          Precision s = cit.GetCenterPixel();
          for(int k=0; k < ImagePixel::Dimension; k++){
              p[k] /= s;
          }
          it.SetCenterPixel(p); 
        }
      }

    };
    






    ImageRegion findBoundingBox(MaskImagePointer image){
      ImageIndex min;
      min.Fill(100000);
      ImageIndex max;
      max.Fill(0);
  
      MaskImageRegionIteratorWithIndex it(image, image->GetLargestPossibleRegion());
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
     MaskImagePointer mask;
     MaskImageSpacing spacing;
     MaskImageRegion bounds;

};


#endif
