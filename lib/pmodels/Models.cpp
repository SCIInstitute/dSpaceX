#include "Models.h"

namespace dspacex {

Model::Type Model::strToType(const std::string& type) {
  if (type == "pca")          return Model::PCA;
  if (type == "shapeodds")    return Model::ShapeOdds;
  if (type == "infshapeodds") return Model::InfShapeOdds;
  if (type == "sharedgp")     return Model::SharedGP;
  return None;
}

std::string Model::typeToStr(const Model::Type& type) {
  if (type == Model::PCA)          return "PCA";
  if (type == Model::ShapeOdds)    return "ShapeOdds";
  if (type == Model::InfShapeOdds) return "InfShapeOdds";
  if (type == Model::SharedGP)     return "SharedGP";
  return "<Unknown Model Type>";
}

std::ostream& operator<<(std::ostream &os, const Model::Type& type) {
  return os << Model::typeToStr(type);
}

// TODO if helpful; e.g., if we want to write [8|16|32|64]-bit greyscale this fcn could take care of resampling appropriately)
// void writeImage(const Eigen::MatrixXf &column_order_image, std::string path, unsigned w, unsigned h)
// {
// }

// creates a new sample (an image) from the given model at the specified latent space coordinate
// write evaluated model to disk if requested (if w * h != I.size(), then write I.rows() x I.cols() image)
Eigen::MatrixXf ShapeOdds::evaluateModel(const Model &model, const Eigen::VectorXf &z_coord,
                                         const bool writeToDisk, const std::string outpath, unsigned w, unsigned h)
{
#if 0 //<ctc> super duper hack to test evaluation of PCA model without updated dataset loading (which has been started but is a wip)
  // I = f(z):
  //  phi = W * z + w0
  //  I = 1 / ( 1 + e^(-phi) )

  // the z_coord should just be one row
  //std::cout << "latent space coord (z):\n" << z_coord << std::endl;
  Eigen::MatrixXf phi = model.W * z_coord + model.w0;
  //std::cout << "phi = W * z + w0:\n" << phi << std::endl;

  //I = 1.0 / (1 + exp(-phi));
  phi.array() *= -1.0;
  phi = phi.array().exp();
  phi.array() += 1.0;
  Eigen::MatrixXf I(phi.array().inverse());
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
#else
  //evaluate this as a PCA model:
  // z = (x - w0)W^t  // computed using Model::getNewLatentSpaceValue (and not like this says)
  // x = zW + w0
  // where z is the latent space (the passed in z_coord)
  // and x is the data space (the new image)

  Eigen::MatrixXf Wt(model.W);
  Wt.transposeInPlace();
  Eigen::MatrixXf I((Wt * z_coord) + model.w0);
  
  // test pca model by returning w0 directly
  //I = model.w0;// /255.0; (the scale normalization below does the same thing)

  // test pca model by returning Eigen vectors in W directly (they're image-like)
//  static int i=0;
//  if (i>=50) i=0;
//  I = Wt.col(i++);

  //I /= 255.0;

  // Ross said to get rid of anything below 0 and scale normalize the rest 2020.06.07
  for (unsigned i = 0; i < I.size(); i++) {
    I(i) = std::max(0.0f, I(i));
  }
  
  // scale normalize so that all values are in range [0,1]. For each member X: X = (X - min) / (max - min).
  auto minval(I.minCoeff());
  //auto minval(0.0);  
  // to be consistent, we probably want to scale all images together so we have proper min/max (TODO)
  auto maxval(I.maxCoeff());
  I.array() -= minval;
  I.array() /= (maxval - minval);
#endif

  return I;
}

// verify evaluated model and how closely it corresponds to sample at that idx
// return measured difference between generated sample and original, and ...
float ShapeOdds::testEvaluateModel(const Model &model, const Eigen::Matrix<float, 1, Eigen::Dynamic> &z_coord,
                                   /*unsigned p, unsigned c,*/ /*unsigned z_idx,*/ const Image &sampleImage,
                                   const bool writeToDisk, const std::string basePath)
{
  std::string outpath(basePath + "/p_idx_"/* + std::to_string(p)*/ + "-c_idx_"/* + std::to_string(c)*/ +"-z_idx_-"/* + std::to_string(z_idx)*/ + ".png");
  unsigned w = sampleImage.getWidth(), h = sampleImage.getHeight();
  
  Eigen::MatrixXf I(evaluateModel(model, z_coord, writeToDisk, outpath, w, h));
  //std::cout << "evaluated model: " << I << std::endl;

  float quality = 1.0;

  // Compare sampleImage (presumably image corresponding to z_idx) to the one just generated by the model.
  Eigen::MatrixXf sampleImageMatrix(w * h, 1);
  
  //<ctc> look at convertToImage for a more elegant way to do this. Also, this function is going away anyway to be replaced by compareImages
  // copy sample image into column-order Eigen::MatrixXf and rescale to [0.0, 1.0]
  const unsigned char* imgdata = sampleImage.getConstData();
  for (unsigned r = 0; r < h; r++)    // for every row...
    for (unsigned c = 0; c < w; c++)  // read all the cols
      sampleImageMatrix(r * w + c) = (float)imgdata[r * w + c] / 255.0;

  //std::cout << "converted sample image data: " << sampleImageMatrix << std::endl;
  
  // compare image with I
  // take average of euclidean norm of each pixel's difference: (A-B).norm() / (A-B).size()
  {
    Eigen::MatrixXf diff(sampleImageMatrix - I);
    std::cout << "diff: rng: [" << diff.minCoeff() << ", " << diff.maxCoeff() << "], avg: " << diff.mean() << ", norm: " << diff.norm() << std::endl; 
    quality = diff.norm() / diff.size();
  }
  
  // produce a pixel-by-pixel differnce (A-B)
  bool writeDiffImage = true; 
  if (writeDiffImage)
  {
    Eigen::MatrixXf diff(sampleImageMatrix - I);
    //<ctc> now we have a reason to have an independent writeImage function (see above)
    //todo
    // - shall we map negative values to blue, positive to red?
  }
  
  return quality;
}

} // dspacex
