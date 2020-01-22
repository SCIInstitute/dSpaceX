#include "Models.h"
#include "imageutils/ImageLoader.h"
#include "lodepng.h"

namespace PModels {

void testEigen()
{
#if 0 // was just learning how to use the Eigen library
  using namespace Eigen;
  
  // Inline mesh of a cube
  my_V_matrix = (MatrixXd(8,3)<<
                 0.0,0.0,0.0,
                 0.0,0.0,1.0,
                 0.0,1.0,0.0,
                 0.0,1.0,1.0,
                 1.0,0.0,0.0,
                 1.0,0.0,1.0,
                 1.0,1.0,0.0,
                 1.0,1.0,1.0).finished();
  my_F_matrix = (MatrixXi(12,3)<<
                 1,7,5,
                 1,3,7,
                 1,4,3,
                 1,2,4,
                 3,8,7,
                 3,4,8,
                 5,7,8,
                 5,8,6,
                 1,5,6,
                 1,6,2,
                 2,6,8,
                 2,8,4).finished().array()-1;

  std::cout << "my_V_matrix: \n" << my_V_matrix << std::endl;
  std::cout << "my_F_matrix: \n" << my_F_matrix << std::endl;

  int idata[] = {1,2,4,8};
  float fdata[] = {1.234,2.468,4.816,8.1624};
  const std::string sidata("1,2,4,8");
  const std::string sfdata("1.234,2.468,4.816,8.1624");
  
  // same as: Map<Matrix2i> imat(idata);
  Map<MatrixXi> imat(idata,2,2);
  std::cout << "imat (map of idata):\n" << imat << std::endl;
  std::cout << "imat.row(0):\n" << imat.row(0) << std::endl;
  std::cout << "imat.col(0):\n" << imat.col(0) << std::endl;
  std::cout << "imat.minCoeff():\n" << imat.minCoeff() << std::endl;
  std::cout << "imat.colwise().minCoeff():\n" << imat.colwise().minCoeff() << std::endl;
  std::cout << "imat.maxCoeff():\n" << imat.maxCoeff() << std::endl;
  std::cout << "imat.colwise().maxCoeff():\n" << imat.colwise().maxCoeff() << std::endl;
  
  Map<MatrixXf> fmat(fdata,2,2);
  std::cout << "fmat (map of fdata):\n" << fmat << std::endl;
  fmat.array() *= -1.0;
  std::cout << "fmat * -1.0:\n" << fmat << std::endl;
  fmat = fmat.array().exp();
  std::cout << "e^fmat:\n" << fmat << std::endl;
  fmat.array() += 1.0;
  std::cout << "fmat + 1.0:\n" << fmat << std::endl;
  fmat = fmat.array().inverse();
  std::cout << "1.0/fmat:\n" << fmat << std::endl;

  Map<RowVectorXf> fvec_of_fmat(fmat.data(), fmat.size());
  std::cout << "vec of fmat:\n" << fvec_of_fmat << std::endl;
  Matrix<float, Dynamic, Dynamic, RowMajor> fmat_rm(fmat);
  Map<RowVectorXf> fvec_of_fmat_rm(fmat_rm.data(), fmat_rm.size());
  std::cout << "row major vec of fmat:\n" << fvec_of_fmat_rm << std::endl;
  
  //colmajor column to rowmajor image (just like I below)
  int imgdata[] = {1,2,4,8,16,32};
  Map<MatrixXi> imat_61(imgdata, 6, 1);
  std::cout << "col major 6 x 1 imat_61:\n" << imat_61 << std::endl;
  typedef Matrix<int,Dynamic,Dynamic,RowMajor> rowmaj_image;
  Map<rowmaj_image> imat_23_rowmaj(imat_61.data(), 2, 3);
  std::cout << "row major 2 x 3 image imat_61:\n" << imat_23_rowmaj << std::endl;
  typedef Matrix<int,Dynamic,Dynamic,ColMajor> colmaj_image;
  Map<colmaj_image> imat_32_colmaj(imat_61.data(), 3, 2);
  std::cout << "col major 3 x 2 image imat_61:\n" << imat_32_colmaj << std::endl;

  MatrixXf M1(2,6);    // Column-major storage
  M1 << 1, 2, 3,  4,  5,  6,
        7, 8, 9, 10, 11, 12;
  std::cout << "M1:" << std::endl << M1 << std::endl;
  Map<MatrixXf> M2(M1.data(), 6,2);
  std::cout << "M2:" << std::endl << M2 << std::endl;

  // try casting from double to char
  MatrixXf MD(2,6);
  MD << 1.00012, 253.56, 39.99, 40.01, 159.499, 65.5, 97, 98, 9, 10, 11, 128;
  Matrix<char, Dynamic, Dynamic> Mc = MD.cast<char>();
  std::cout << "MD:" << std::endl << MD << std::endl;
  std::cout << "...cast to char is:\n";
  std::cout << "Mc:" << std::endl << Mc << std::endl;
#endif
}

// TODO if helpful; e.g., if we want to write [8|16|32|64]-bit greyscale this fcn could take care of resampling appropriately)
// void writeImage(const Eigen::MatrixXd &column_order_image, std::string path, unsigned w, unsigned h)
// {
// }

// creates a new sample (an image) from the given model at the specified latent space coordinate
// write evaluated model to disk if requested (if w * h != I.size(), then write I.rows() x I.cols() image)
Eigen::MatrixXd ShapeOdds::evaluateModel(const Model &model, const Eigen::VectorXd &z_coord,
                                         const bool writeToDisk, const std::string outpath, unsigned w, unsigned h)
{
  // I = f(z):
  //  phi = W * z + w0
  //  I = 1 / ( 1 + e^(-phi) )

  // the z_coord should just be one row
  //std::cout << "latent space coord (z):\n" << z_coord << std::endl;
  Eigen::MatrixXd phi = model.W * z_coord + model.w0;
  //std::cout << "phi = W * z + w0:\n" << phi << std::endl;

  //I = 1.0 / (1 + exp(-phi));
  phi.array() *= -1.0;
  phi = phi.array().exp();
  phi.array() += 1.0;
  Eigen::MatrixXd I(phi.array().inverse());
  //std::cout << "I = 1 / (1 + e^(-phi)):\n" << I << std::endl;
  
  if (writeToDisk)
  {
    Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> image = (I.array() * 255.0).cast<unsigned char>();

    if (image.size() != w * h)
    { 
      std::cout << "Warning: w * h (" << w << " * " << h << ") != computed image size (" << image.size() << ")\n";
      h = image.rows();
      w = image.cols();
    }
    image.resize(h, w);
    image.transposeInPlace();

    unsigned error = lodepng::encode(outpath, image.data(), w, h, LCT_GREY, 8);
    if (error) {
      throw std::runtime_error("encoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
    } 
  }

  return I;
}

// verify evaluated model and how closely it corresponds to sample at that idx
// return measured difference between generated sample and original, and ...
float ShapeOdds::testEvaluateModel(const Model &model, const Eigen::Matrix<double, 1, Eigen::Dynamic> &z_coord,
                                   unsigned p, unsigned c, unsigned z_idx, const Image &sampleImage,
                                   const bool writeToDisk, const std::string basePath)
{
  std::string outpath(basePath + "/p" + std::to_string(p) + "-c" + std::to_string(c) +"-z" + std::to_string(z_idx) + ".png");
  unsigned w = sampleImage.getWidth(), h = sampleImage.getHeight();
  
  Eigen::MatrixXd I(evaluateModel(model, z_coord, writeToDisk, outpath, w, h));
  //std::cout << "evaluated model: " << I << std::endl;

  float quality = 1.0;

  // Compare sampleImage (presumably image corresponding to z_idx) to the one just generated by the model.
  Eigen::MatrixXd sampleImageMatrix(w * h, 1);
  
  //<ctc> look at convertToImage for a more elegant way to do this. Also, this function is going away anyway to be replaced by compareImages
  // copy sample image into column-order Eigen::MatrixXd and rescale to [0.0, 1.0]
  const unsigned char* imgdata = sampleImage.getConstData();
  for (unsigned r = 0; r < h; r++)    // for every row...
    for (unsigned c = 0; c < w; c++)  // read all the cols
      sampleImageMatrix(r * w + c) = (double)imgdata[r * w + c] / 255.0;

  //std::cout << "converted sample image data: " << sampleImageMatrix << std::endl;
  
  // compare image with I
  // take average of euclidean norm of each pixel's difference: (A-B).norm() / (A-B).size()
  {
    Eigen::MatrixXd diff(sampleImageMatrix - I);
    std::cout << "diff: rng: [" << diff.minCoeff() << ", " << diff.maxCoeff() << "], avg: " << diff.mean() << ", norm: " << diff.norm() << std::endl; 
    quality = diff.norm() / diff.size();
  }
  
  // produce a pixel-by-pixel differnce (A-B)
  bool writeDiffImage = true; 
  if (writeDiffImage)
  {
    Eigen::MatrixXd diff(sampleImageMatrix - I);
    //<ctc> now we have a reason to have an independent writeImage function (see above)
    //todo
    // - shall we map negative values to blue, positive to red?
  }
  
  return quality;
}

} // end namespace PModels
