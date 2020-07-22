#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "imageutils/Image.h"
#include "dspacex/Fieldtype.h"

namespace dspacex {

class MSCrystal;

/// Probabilistic interpolation models such as ShapeOdds, InfShapeOdds, SharedGP, and PCA.
/// They are learned from a set of input designs (images/samples, parameters, and qois), and
/// consist of L necessary values that enable the model to compute new samples for a given
/// point, z, in its latent space.

/// Common interface to evaluate a probablistic model given a new field value or z coordinate.
class Model
{
public:
  enum Type { PCA, ShapeOdds, InfShapeOdds, SharedGP, None = 0 };
  static std::string typeToStr(const Type& type);
  static Type strToType(const std::string& type);

  Model(Type t) : type(t) {}
  void setModel(Eigen::MatrixXf _W, Eigen::MatrixXf _w0, Eigen::MatrixXf _Z)
  {
    W  = _W;
    w0 = _w0;
    Z  = _Z; // latent space coords for samples used to generate this model
  }

  void setBounds(std::pair<float, float> minmax) {
    minval = minmax.first;
    maxval = minmax.second;
  }

  const Eigen::VectorXf& getZCoord(unsigned local_idx) const
  {
    // z_coords for this model are ordered by the global order of samples used to construct it
    return Z.row(local_idx);
  }

  float minFieldValue() const { return minval; }
  float maxFieldValue() const { return maxval; }

  /// computes new z_coord at this field value
  /* static */ const Eigen::RowVectorXf getNewLatentSpaceValue(const Eigen::RowVectorXf& fieldvalues, float new_fieldval, float sigma = 0.25) const
  {
    //debug: hardcode new fieldval
    //new_fieldval = 0.62341;
    //std::cout << "num_samples: " << sample_indices.size() << std::endl;
    //std::cout << "z-size: " << Z.cols() << std::endl;
    
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

    MatrixXf output = fieldvals * Z;
    //std::cout << "output before division:\n" << output << std::endl;
    output /= summation;
    //RowVectorXf output = (fieldvals * Z) / summation;
    //std::cout << "new z_coord:\n" << output << std::endl;
    //std::cout << "for comparison, here's the first z_coord from the training data:\n" << Z.row(0) << std::endl;

    return output;
  }

  Type getType() const { return type; }

private:
  Type type{None};

  Eigen::MatrixXf Z;                 // latent space coordinates of samples used to learn this model
  Eigen::MatrixXf W;
  Eigen::MatrixXf w0;
  float minval{0.0f}, maxval{0.0f};  // min amd max fieldvalue of samples used to learn this model

  friend class ShapeOdds;
  friend class PCA;
};


/* 
 * static functions to evaluate a ShapeOdds model
 */
class ShapeOdds
{
public:
  static Eigen::MatrixXf evaluateModel(const Model &model, const Eigen::VectorXf &z_coord, const bool writeToDisk = false,
                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0);
  
  static float testEvaluateModel(const Model &model, const Eigen::Matrix<float, 1, Eigen::Dynamic> &z_coord,
                                 /*const unsigned p, const unsigned c, const unsigned z_idx,*/ const Image &sampleImage,
                                 const bool writeToDisk = false, const std::string path = "");
};

/* 
 * static functions to evaluate a PCA model
 */
class PCA
{
public:
  static Eigen::MatrixXf evaluateModel(const Model &model, const Eigen::VectorXf &z_coord, const bool writeToDisk = false,
                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0);
  
  static float testEvaluateModel(const Model &model, const Eigen::Matrix<float, 1, Eigen::Dynamic> &z_coord,
                                 /*const unsigned p, const unsigned c, const unsigned z_idx,*/ const Image &sampleImage,
                                 const bool writeToDisk = false, const std::string path = "");
};

std::ostream& operator<<(std::ostream &os, const Model::Type& type);

} // dspacex
