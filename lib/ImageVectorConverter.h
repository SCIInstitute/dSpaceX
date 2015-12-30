#ifndef IMAGEVECTORCONVERTER_H
#define IMAGEVECTORCONVERTER_H

#include "DenseVector.h"
#include "Matrix.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkCastImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"

template<typename TImage>
class ImageVectorConverter{

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
   

    ImageVectorConverter(ImagePointer maskImage){
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


    FortranLinalg::DenseVector<Precision> extractVector(ImageConstPointer image){
      FortranLinalg::DenseVector<Precision> vec(D);
      extractVector(image, vec);
      return vec;
    };


    //TODO
    FortranLinalg::DenseVector<Precision> extractVector(ImagePointer image){
      FortranLinalg::DenseVector<Precision> vec(D);
      extractVector(image, vec);
      return vec;
    };


    void extractVector(ImageConstPointer image, FortranLinalg::Vector<Precision> &vector){
      ImageRegionIterator maskIt(mask, bounds);
      ImageRegionConstIterator it(image, bounds);
      int row = 0;
      for(it.GoToBegin(), maskIt.GoToBegin(); !it.IsAtEnd(); ++it, ++maskIt){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0){
          vector[row] =  it.Get();
          ++row;
        }
      }
    };


    //TODO
    void extractVector(ImagePointer image, FortranLinalg::Vector<Precision> &vector){
      ImageRegionIterator maskIt(mask, bounds);
      ImageRegionConstIterator it(image, bounds);
      int row = 0;
      for(it.GoToBegin(), maskIt.GoToBegin(); !it.IsAtEnd(); ++it, ++maskIt){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0){
          vector(row) =  it.Get();
          ++row;
        }
      }

    };


    void fillColumn(ImagePointer image, int column, FortranLinalg::Matrix<Precision> &matrix){
      ImageRegionIterator maskIt(mask, bounds);
      ImageRegionIterator it(image, bounds);
      int row = 0;
      for(it.GoToBegin(), maskIt.GoToBegin(); !it.IsAtEnd(); ++it, ++maskIt){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0){
          matrix(row, column) =  it.Get();
          ++row;
        }
      }
    };


    ImagePointer createImage(){
      CastFilterPointer cast = CastFilter::New();
      cast->SetInput(mask);
      cast->Update();
      return cast->GetOutput();
    };

    
    void fillImage(FortranLinalg::Vector<Precision> &vec, ImagePointer image, Precision outsideValue = 0){
      ImageRegionIterator it(image, image->GetLargestPossibleRegion()); 
      it.GoToBegin();
      ImageRegionIterator maskIt(mask, mask->GetLargestPossibleRegion()); 
      maskIt.GoToBegin();
      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0 ){
          it.Set(vec(index));
          ++index;
        }else{
          it.Set(outsideValue); 	        
        }
      }
    };
    
    void fillImage(FortranLinalg::Matrix<Precision> &matrix, int column, ImagePointer image, Precision outsideValue = 0){
      ImageRegionIterator it(image, image->GetLargestPossibleRegion()); 
      it.GoToBegin();
      ImageRegionIterator maskIt(mask, mask->GetLargestPossibleRegion()); 
      maskIt.GoToBegin();
      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0 ){
          it.Set(matrix(index, column));
          ++index;
        }else{
          it.Set(outsideValue); 	        
        }
      }
    };



    ImagePointer createImage(FortranLinalg::Vector<Precision> &vec, Precision outsideValue = 0){
      ImagePointer image = createImage();
      fillImage(vec, image, outsideValue);
      return image;
    };


    ImagePointer createImage(FortranLinalg::Matrix<Precision> &matrix, int column, Precision
        outsideValue = 0){
      ImagePointer image = createImage();
      ImageRegionIterator it(image, image->GetLargestPossibleRegion()); 
      it.GoToBegin();
      ImageRegionIterator maskIt(mask, mask->GetLargestPossibleRegion()); 
      maskIt.GoToBegin();
      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0 ){
          it.Set(matrix(index, column));
          ++index;
        }else{
          it.Set(outsideValue); 	        
        }
      }

      return image;

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
          for(unsigned int i=0; i<index.GetIndexDimension(); i++){
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
      for(unsigned int i=0; i < ImageSize::GetSizeDimension(); i++){
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
     typedef typename itk::CastImageFilter<Image, Image> CastFilter;
     typedef typename CastFilter::Pointer CastFilterPointer;


};


#endif
