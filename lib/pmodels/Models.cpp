#include "Models.h"
#include "lodepng.h"

namespace dspacex {

Model::Type Model::strToType(const std::string& type) {
  if (type == "pca")          return Model::PCA;
  if (type == "shapeodds")    return Model::ShapeOdds;
  if (type == "infshapeodds") return Model::InfShapeOdds;
  if (type == "sharedgp")     return Model::SharedGP;
  if (type == "custom")       return Model::Custom;
  return None;
}

std::string Model::typeToStr(const Model::Type& type) {
  if (type == Model::PCA)          return "PCA";
  if (type == Model::ShapeOdds)    return "ShapeOdds";
  if (type == Model::InfShapeOdds) return "InfShapeOdds";
  if (type == Model::SharedGP)     return "SharedGP";
  if (type == Model::Custom)       return "Custom";
  return "<Unknown Model Type>";
}

std::ostream& operator<<(std::ostream &os, const Model::Type& type) {
  return os << Model::typeToStr(type);
}

/// returns 
std::unique_ptr<Model> Model::create(Type t, const std::string& name) {
  switch (t) {
    case PCA:
      return std::make_unique<PCAModel>();
    case ShapeOdds:
      return std::make_unique<ShapeOddsModel>();
    case InfShapeOdds:
      return nullptr;
    case SharedGP:
      return nullptr;
    case Custom:
      return std::make_unique<CustomModel>(name);
    default:
      return nullptr;
  }
}

// TODO if helpful; e.g., if we want to write [8|16|32|64]-bit greyscale this fcn could take care of resampling appropriately)
// void writeImage(const Eigen::MatrixXf &column_order_image, std::string path, unsigned w, unsigned h)
// {
// }

const Eigen::RowVectorXf Model::getNewLatentSpaceValue(const Eigen::RowVectorXf& fieldvalues, const Eigen::MatrixXf& z_coords, float new_fieldval, float sigma)
{
  //debug: hardcode new fieldval
  //new_fieldval = 0.62341;
  //std::cout << "num_samples: " << sample_indices.size() << std::endl;
  //std::cout << "z-size: " << z_coords.cols() << std::endl;
    
  // gaussian kernel regression to generate a new LSV
  using namespace Eigen;

  // calculate difference
  RowVectorXf fieldvals(fieldvalues); // todo: convert this to a static function by passing in z_coords since it's used for evaluation of multiple model types (PCA or ShapeOdds)
  fieldvals *= -1.0;
  fieldvals.array() += new_fieldval;
  //std::cout << "difference between new field value and training field values:\n" << fieldvals << std::endl;

  // apply Gaussian to difference
  fieldvals = fieldvals.array().square();
  //std::cout << "squared difference:\n" << fieldvals << std::endl;
  fieldvals /= (-2.0 * sigma * sigma);
  //std::cout << "difference / -2sigma^2:\n" << fieldvals << std::endl;
  fieldvals = fieldvals.array().exp();
  //std::cout << "e^(difference / -2sigma^2):\n" << fieldvals << std::endl;
  float denom = sqrt(2.0 * M_PI * sigma);
  //std::cout << "denom (sqrt(2*pi*sigma):\n" << denom << std::endl;
  fieldvals /= denom;
  //std::cout << "Gaussian matrix of difference:\n" << fieldvals << std::endl;

  // calculate weight and normalization for regression
  float summation = fieldvals.sum();
  //std::cout << "sum of Gaussian vector of difference:\n" << summation << std::endl;

  MatrixXf output = fieldvals * z_coords;
  //std::cout << "output before division:\n" << output << std::endl;
  output /= summation;
  //RowVectorXf output = (fieldvals * z_coords) / summation;
  //std::cout << "new z_coord:\n" << output << std::endl;
  //std::cout << "for comparison, here's the first z_coord from the training data:\n" << z_coords.row(0) << std::endl;

  return output;
}

// verify evaluated model and how closely it corresponds to sample at that idx
// return measured difference between generated sample and original, and ...
float Model::testEvaluateModel(std::shared_ptr<Model> model, const Eigen::Matrix<float, 1, Eigen::Dynamic> &z_coord,
                               const Image &sampleImage, const bool writeToDisk, const std::string basePath)
{
  std::string outpath(basePath + "/p_idx_"/* + std::to_string(p)*/ + "-c_idx_"/* + std::to_string(c)*/ +"-z_idx_-"/* + std::to_string(z_idx)*/ + ".png");
  unsigned w = sampleImage.getWidth(), h = sampleImage.getHeight();
  
  Eigen::MatrixXf I(model->evaluate(z_coord, writeToDisk, outpath, w, h));
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

// creates a new sample (an image) from the given model at the specified latent space coordinate
// write evaluated model to disk if requested (if w * h != I.size(), then write I.rows() x I.cols() image)
Eigen::MatrixXf ShapeOddsModel::evaluate(const Eigen::VectorXf &z_coord,
                                         const bool writeToDisk, const std::string outpath, unsigned w, unsigned h) const
{
  // I = f(z):
  //  phi = W * z + w0
  //  I = 1 / ( 1 + e^(-phi) )

  // the z_coord should just be one row
  //std::cout << "latent space coord (z):\n" << z_coord << std::endl;
  Eigen::MatrixXf phi = W * z_coord + w0;
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

  return I;
}

// creates a new sample (an image) from the given model at the specified latent space coordinate
// write evaluated model to disk if requested (if w * h != I.size(), then write I.rows() x I.cols() image)
Eigen::MatrixXf PCAModel::evaluate(const Eigen::VectorXf &z_coord,
                                   const bool writeToDisk, const std::string outpath, unsigned w, unsigned h) const
{
  //evaluate this as a PCA model:
  // z = (x - w0)W^t  // computed using Model::getNewLatentSpaceValue (and not like this says)
  // x = zW + w0
  // where z is the latent space (the passed in z_coord)
  // and x is the data space (the new image)

  Eigen::MatrixXf Wt(W);
  Wt.transposeInPlace();
  Eigen::MatrixXf I((Wt * z_coord) + w0);
  
  // Ross said to get rid of anything below 0 and scale normalize the rest 2020.06.07
  for (unsigned i = 0; i < I.size(); i++) {
    I(i) = std::max(0.0f, I(i));
  }
  
  // scale normalize so that all values are in range [0,1]. For each member X: X = (X - min) / (max - min).
  auto minval(I.minCoeff());
  // to be consistent, we probably want to scale all images together so we have proper min/max (TODO)
  auto maxval(I.maxCoeff());
  I.array() -= minval;
  I.array() /= (maxval - minval);

  return I;
}

Eigen::MatrixXf Model::fetchInterpolation(int idx, int interpolationSet) const
{
  // TODO
  return Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>();
}

} // dspacex
