#ifndef IMAGENEIGHBORHOOD_H
#define IMAGENEIGHBORHOOD_H

#include "DenseVector.h"
#include "Matrix.h"
#include "Linalg.h"

#include "ImageIO.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"
#include "itkPeriodicBoundaryCondition.h"
#include <itkZeroFluxNeumannBoundaryCondition.h>

template<typename TImage>
class ImageNeighborhood{

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
    typedef typename itk::NeighborhoodIterator<Image,
            itk::ZeroFluxNeumannBoundaryCondition<Image> > NeighborhoodIterator;
   typedef typename NeighborhoodIterator::RadiusType Radius;



    ImageNeighborhood(ImagePointer maskImage){
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



    //Extract all patches of size (2r+1) x (2r+1) for which the mask image is
    //not zero
    FortranLinalg::DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, Radius radius,
        int maskIndex = 1){
      return extractNeighborhoods(image, radius,
          image->GetLargestPossibleRegion(), maskIndex);
    };

    FortranLinalg::DenseMatrix<Precision> extractNeighborhoods(ImagePointer image, Radius r,
        ImageRegion region, int maskIndex = 1){
      using namespace FortranLinalg;

      int nd = 1;
      for(unsigned int i=0; i<Image::GetImageDimension(); i++){
        nd *= (2 * r[i] + 1);
      }

      NeighborhoodIterator it(r, image, region);
      
      ImageRegionIterator maskIt(mask, region); 
      maskIt.GoToBegin();

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          npixels++;
        }
      }
      maskIt.GoToBegin();


      DenseMatrix<Precision> n(nd, npixels);

      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        Precision usePixel = maskIt.Get() == maskIndex;
        if(usePixel != 0 ){
          for(int j=0; j<nd; j++){
            n(j, index) = it.GetPixel(j);
          }

          index++;
        }
      }

      return n;
    };



    //Extract all pacthes within radius rRegion from x
    FortranLinalg::DenseMatrix<Precision> extractRadial(ImagePointer image, ImageIndex x, Precision rRegion, Radius rPatch){
      using namespace FortranLinalg;

      std::list< DenseVector<Precision> > patches;      

      int nd = 1;
      for(unsigned int i=0; i<Image::GetImageDimension(); i++){
        nd *= (2 * rPatch[i] + 1);
      }

      ImageSize size;
      for(int i=0; i<ImageIndex::GetIndexDimension(); i++){
        x[i] -= rRegion;
        size[i] = 2*rRegion+1;
      }
       
      
      ImageRegion region(x, size);
      NeighborhoodIterator it(rPatch, image, region);
      for( ; !it.IsAtEnd(); ++it){
        ImageIndex y = it.GetIndex();
        typename ImageIndex::OffsetType d = x-y;
        Precision r = 0;
        for(int i=0;i<ImageIndex::GetIndexDimension(); i++){
           r += d[i]*d[i];
        }
        r = sqrt(r);
        if(r < rRegion){
          DenseVector<Precision> patch(nd);
          for(int j=0; j<nd; j++){
            patch(j) = it.GetPixel(j);
          }
          patches.push_back(patch);
        }
      }
      DenseMatrix<Precision> n = Linalg<Precision>::ToMatrix(patches);

      typename std::list< DenseVector<Precision> >::iterator pIt = patches.begin(); 
      for(; pIt != patches.end(); ++pIt){
        (*pIt).deallocate();
      }    

      return n;
    };



    //Extract all image indices for which the mask imeg is not zero
    ImageIndex *extractIndicies(int maskIndex = 1){
      ImageRegionIteratorWithIndex maskIt(mask, mask->GetLargestPossibleRegion()); 
      maskIt.GoToBegin();

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          npixels++;
        }
      }
      ImageIndex *ind =new ImageIndex[npixels];
      maskIt.GoToBegin();
      int i = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          ind[i] = maskIt.GetIndex();
          ++i;
        }
      }
      return ind;

    };



    //Extract all neighborhoods from the image
    FortranLinalg::DenseMatrix<Precision> extractAllNeighborhoods(ImagePointer image, Radius
        radius){
      return extractAllNeighborhoods(image, radius,
          image->GetLargestPossibleRegion());
    };


    FortranLinalg::DenseMatrix<Precision> extractAllNeighborhoods(ImagePointer image, Radius r,
        ImageRegion region){
      using namespace FortranLinalg;
      
      int nd = 1;
      for(unsigned int i=0; i<Image::GetImageDimension(); i++){
        nd *= (2 * r[i] + 1);
      }

      NeighborhoodIterator it(r, image, region);
      
      typename ImageRegion::SizeType size = region.GetSize();
      int npixels = 1;
      for(int i=0; i < size.GetSizeDimension(); i++){
        npixels *= size[i];
      }

      DenseMatrix<Precision> n(nd, npixels);

      int index = 0;
      for( ; !it.IsAtEnd(); ++it ){

          for(int j=0; j<nd; j++){
              n(j, index) = it.GetPixel(j);
          }

          index++;
      }

      return n;
    }; 
   




    

    //Put the whole image into a single vector
    FortranLinalg::DenseVector<Precision> extractAll(ImagePointer image){
      return extractAll(image, image->GetLargestPossibleRegion());
    };


    //Put the image region into a single vector
    FortranLinalg::DenseVector<Precision> extractAll(ImagePointer image, ImageRegion region){
      using namespace FortranLinalg;
      
      ImageRegionIterator it(image, region);
      
      ImageRegionIterator maskIt(mask, region); 

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        npixels++;
      }


      maskIt.GoToBegin();

      DenseVector<Precision> n(npixels);

      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
          n(index) = it.Get();
          index++;
      }

      return n;
    };



    //Extract image inot a single vector put only locations where mask != 0
    FortranLinalg::DenseVector<Precision> extract(ImagePointer image, int maskIndex = 1){
      return extract(image, image->GetLargestPossibleRegion(), maskIndex);
    };


    FortranLinalg::DenseVector<Precision> extract(ImagePointer image,
        ImageRegion region, int maskIndex = 1){
      using namespace FortranLinalg;
      
      ImageRegionIterator it(image, region);
      
      ImageRegionIterator maskIt(mask, region); 

      unsigned int npixels = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt){
        if(maskIt.Get() == maskIndex ){
          npixels++;
        }
      }


      maskIt.GoToBegin();

      DenseVector<Precision> n(npixels);

      int index = 0;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it ){
        if(maskIt.Get() == maskIndex ){
          n(index) = it.Get();
          index++;
        }
      }

      return n;
    };




    //Create a new image of the same size as the mask image
    ImagePointer createImage(){
      return createImage(mask->GetLargestPossibleRegion());
    }



    //Create a new image of size region
    ImagePointer createImage(ImageRegion region ){
      ImagePointer image = Image::New();
      image->SetRegions(region);
      image->SetSpacing(spacing);
      image->SetOrigin(mask->GetOrigin());
      image->Allocate();
      return image; 
    };

    






    //Fill the image based from the neighborhood patches in n
    void fillImage(FortranLinalg::DenseMatrix<Precision> &n, ImagePointer image, Radius r,  Precision outsideValue = 0){
      using namespace FortranLinalg;
      
     
      ImagePointer counter = ImageIO<Image>::copyImage(image);
      
      NeighborhoodIterator cit(r, counter, counter->GetLargestPossibleRegion());       
      NeighborhoodIterator it(r, image, image->GetLargestPossibleRegion());       
      ImageRegionIterator maskIt(mask, image->GetLargestPossibleRegion()); 

      for( ;!cit.IsAtEnd(); ++cit, ++it){
        cit.SetCenterPixel(0);
        it.SetCenterPixel(0);
      }

      
      it.GoToBegin();
      maskIt.GoToBegin();
      cit.GoToBegin();
      int index = 0;
      bool ignore = false;
      for( ; !maskIt.IsAtEnd(); ++maskIt, ++it, ++cit){
        Precision usePixel = maskIt.Get();
        if(usePixel != 0 ){ 
          for(int j=0; j<n.M(); j++){
            cit.SetPixel(j, cit.GetPixel(j)+1, ignore);
            it.SetPixel(j, it.GetPixel(j) + n(j, index), ignore);
          }
          ++index;
        }else{
          it.SetCenterPixel(outsideValue);
          cit.SetCenterPixel(1);          
        }
      }
      it.GoToBegin();
      cit.GoToBegin();
      for( ; !it.IsAtEnd(); ++it, ++cit){
        if(cit.GetCenterPixel() > 0 ){ 
          it.SetCenterPixel(it.GetCenterPixel()/cit.GetCenterPixel()); 
        }
      }


    };
    



    //Create Image and fill by patch information in n
    ImagePointer createImage(FortranLinalg::DenseMatrix<Precision> &n, Radius r, Precision outsideValue = 0){
      return createImage(n, mask->GetLargestPossibleRegion(), r, outsideValue);
    };




    ImagePointer createImage(FortranLinalg::DenseMatrix<Precision> &n, ImageRegion region,
        Radius r, Precision outsideValue = 0){
      ImagePointer image = createImage(region);
      fillImage(n, image, r, outsideValue);
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

};


#endif
