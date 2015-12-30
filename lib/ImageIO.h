#ifndef IMAGEIO_H
#define IMAGEIO_H


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include "IO.h"


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkCastImageFilter.h"


#include "DenseMatrix.h"

#include "ImageVectorConverter.h"

#include <list>

template <typename TImage>
class ImageIO{


  public:

    typedef TImage Image;
    typedef typename Image::Pointer ImagePointer;
    typedef typename Image::ConstPointer ImageConstPointer;
    typedef typename Image::IndexType ImageIndex;
    typedef typename Image::RegionType ImageRegion;
    typedef typename ImageRegion::SizeType ImageSize;
    typedef typename ImageSize::SizeValueType ImageSizeValue;
    typedef typename Image::SpacingType ImageSpacing;
    typedef typename Image::PixelType Precision;

    typedef typename itk::ImageFileReader<Image> ImageReader;
    typedef typename ImageReader::Pointer ImageReaderPointer;

    typedef typename itk::ImageSeriesReader<Image> ImageSeriesReader;
    typedef typename ImageSeriesReader::Pointer ImageSeriesReaderPointer;

    typedef typename itk::ImageFileWriter<Image> ImageWriter;
    typedef typename ImageWriter::Pointer ImageWriterPointer;

    typedef typename itk::ImageRegionIteratorWithIndex<Image> ImageRegionIteratorWithIndex;
    typedef typename itk::ImageRegionIterator<Image> ImageRegionIterator;

    typedef typename itk::CastImageFilter<Image, Image> CastFilter;
    typedef typename CastFilter::Pointer CastFilterPointer;

    typedef ImageVectorConverter<Image> Converter;


  //Save an image from a vector
  static void saveImage(ImagePointer image, const std::string &filename){
    ImageWriterPointer writer = ImageWriter::New();
    writer->SetFileName(filename);
    writer->SetInput(image);
    writer->Update();
  };


  static void saveImage(FortranLinalg::Vector<Precision> &imVals, const std::string filename, Converter &converter){
    ImagePointer image = converter.createImage(imVals);
    saveImage(image, filename.c_str()); 
  };



  static ImagePointer readImage(const std::string &filename){
    ImagePointer input = NULL;
    ImageReaderPointer imageReader = ImageReader::New();
    imageReader->SetFileName( filename );
    imageReader->Update();
    input = imageReader->GetOutput();
    return input;
  };


  static ImagePointer copyImage(ImagePointer image){
    CastFilterPointer cast = CastFilter::New();
    cast->SetInput(image);
    cast->Update();
    return cast->GetOutput();  
  };

  /*static ImagePointer createImage(ImagePointer tmplate){
    ImagePointer image = Image::New();
    image->SetRegions( tmplate->GetLargestPossibleRegion() );
    image->SetOrigin( tmplate->GetOrigin());
    image->SetSpacing( tmplate->GetSpacing() );
    image->SetDirection( tmplate->GetDirection());
    image->Allocate();
    return image; 
  };*/

  //read data matrix from images
  static FortranLinalg::DenseMatrix<Precision> readDataMatrix(ImageVectorConverter<Image> &converter, 
                                    std::string imageFileList) {
      std::list<std::string> files =
        IO<Precision>::readStringList(imageFileList);
      return readDataMatrix(converter, files);
  };


  //read data matrix from images
  static FortranLinalg::DenseMatrix<Precision> readDataMatrix(ImageVectorConverter<Image> &converter, 
                                    std::list<std::string> &imageFiles) {

    int D = converter.getD();
    FortranLinalg::DenseMatrix<Precision> data( D, imageFiles.size() );

    long column = 0;
    for(std::list<std::string>::iterator fileIt = imageFiles.begin(); fileIt !=
      imageFiles.end(); ++fileIt){
    
      ImageReaderPointer imageReader = ImageReader::New();
      imageReader->SetFileName( *fileIt );
    
      ImagePointer input = readImage( (*fileIt).c_str());

      converter.fillColumn(input, column, data);
      ++column;
    }
    return data;
  };

};


#endif
