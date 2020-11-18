#include "Model.h"
#include "lodepng.h"

#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::literals::chrono_literals;

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

// z' = f(x', X, Z)
//
// new z_coord z' = Gaussian kernel regression computed from new field_val x' and original
// field_vals X along with their associated z_coords Z.
//
// The contribution of the nearby original z_coords is larger than those farther away using a
// Gaussian curve centered at the new sample, and sigma determine the width of this curve.
//
const Eigen::RowVectorXf Model::getNewLatentSpaceValue(const Eigen::RowVectorXf& fieldvalues, const Eigen::MatrixXf& z_coords, float new_fieldval, float sigma)
{
  // 1 / sqrt(2*pi*sigma) * e^0.5*((x-mu)/sigma)^2
  // x-mu is the differences between the field values
  // 
  // sigma should be used to only include a subset of nearby fieldvalues to average its value
  // is not independent of the field range, but perhaps a percentage of the data range is a
  // reasonable choice (todo: ask Shireen what she wants to do)
  
  // gaussian kernel regression to generate a new LSV
  using namespace Eigen;

  // calculate difference
  RowVectorXf fieldvals(fieldvalues);
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

// creates a new sample (an image) from the given model at the specified latent space coordinate
std::shared_ptr<Eigen::MatrixXf> ShapeOddsModel::evaluate(const Eigen::VectorXf &z_coord) const
{
  time_point<Clock> start = Clock::now();

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
  std::shared_ptr<Eigen::MatrixXf> I(new Eigen::MatrixXf(phi.array().inverse()));
  //std::cout << "I = 1 / (1 + e^(-phi)):\n" << I << std::endl;
  
  time_point<Clock> end = Clock::now();
  std::cout << "ShapeOdds model evaluated in " << duration_cast<milliseconds>(end - start).count() << " ms" << std::endl;

  return I;
}

// creates a new sample (an image) from the given model at the specified latent space coordinate
std::shared_ptr<Eigen::MatrixXf> PCAModel::evaluate(const Eigen::VectorXf &z_coord) const
{
  time_point<Clock> start = Clock::now();

  //evaluate this as a PCA model:
  // z = (x - w0)W^t  // computed using Model::getNewLatentSpaceValue (and not like this says)
  // x = zW + w0
  // where z is the latent space (the passed in z_coord)
  // and x is the data space (the new image)

  //std::cout << "PCAModel::evaluate: z_coord = " << z_coord << std::endl;

  Eigen::MatrixXf Wt(W);
  Wt.transposeInPlace();
  std::shared_ptr<Eigen::MatrixXf> Ip(new Eigen::MatrixXf((Wt * z_coord) + w0));
  Eigen::MatrixXf& I(*Ip);
  
#if 0 // this eliminates vertices when evaluating a corresponding-mesh model (that produces new vertices)
  // Ross said to get rid of anything below 0 and scale normalize the rest 2020.06.07
  for (unsigned i = 0; i < I.size(); i++) {
    I(i) = std::max(0.0f, I(i));
  }
#endif
  
  // scale normalize so that all values are in range [0,1]. For each member X: X = (X - min) / (max - min).
  auto minval(I.minCoeff());
  // to be consistent, we probably want to scale all images together so we have proper min/max (TODO)
  auto maxval(I.maxCoeff());
  I.array() -= minval;
  I.array() /= (maxval - minval);

  std::cout << "PCA model evaluated in " << duration_cast<milliseconds>(Clock::now() - start).count() << " ms" << std::endl;

  return Ip;
}

Eigen::MatrixXf Model::fetchInterpolation(int idx, int interpolationSet) const
{
  // TODO
  return Eigen::Matrix<Precision, Eigen::Dynamic, Eigen::Dynamic>();
}

} // dspacex
