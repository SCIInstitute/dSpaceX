#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "imageutils/Image.h"

namespace PModels {

// There is a model for each crystal at each persistence level for a given M-S topology. It is
// constructed from a given number of input images (samples), and consists of L necessary
// values that make the model able to compute new images for a given point, z, in the latent
// space.
//
// TODO: provide common interface for ShapeOdds, InfiniteShapeOdds, and SharedGP models
// TODO: a model and its associated crystal should know to which fieldname it belongs, right?
class Model
{
public:
  Model() : fieldvalues(NULL, 0)
  {}

  void setModel(Eigen::MatrixXd _W, Eigen::MatrixXd _w0, Eigen::MatrixXd _Z)
  {
    W  = _W;
    w0 = _w0;
    Z  = _Z;
  }

  void addSample(unsigned n)
  {
    sample_indices.insert(n);
  }

  const unsigned numSamples() const
  {
    return sample_indices.size();
  }

  const std::set<unsigned>& getSampleIndices() const
  {
    return sample_indices;
  }
  
  const Eigen::Matrix<double, 1, Eigen::Dynamic> getZCoord(const unsigned idx) const
  {
    if (idx >= numSamples())
      throw std::runtime_error("cannot return " + std::to_string(idx) + "th z_coord because there are only " + std::to_string(numSamples()) + " samples in this model.");
    
    return Z.row(idx);
  }

  // note: fieldname is part of the mscomplex in which this crystal lives, so maybe "getparent" would be better... some problem with copying that I ran into ([yet another] TODO)
  void setFieldname(const std::string name)
  {
    fieldname = name;
  }

  const std::string& getFieldname() const
  {
    return fieldname;
  }

  void setFieldValues(Eigen::Map<Eigen::VectorXd> values)
  {
    new (&fieldvalues) Eigen::Map<Eigen::VectorXd>(values.data(), values.size());
    min_fieldval = fieldvalues.minCoeff();
    max_fieldval = fieldvalues.maxCoeff();
  }

  double minFieldValue()
  {
    return min_fieldval;
  }

  double maxFieldValue()
  {
    return max_fieldval;
  }

  const Eigen::VectorXd getNewLatentSpaceValue(double fieldval, double sigma = 0.25)
  {
    // gaussian kernel regression to generate a new LSV
    using namespace Eigen;

    // calculate difference
    VectorXd difference = fieldvalues * -1.0;
    difference.array() += fieldval;
    //VectorXd difference = fieldval - fieldvalues;
    std::cout << "difference between new field value and training field values:\n" << difference << std::endl;
    difference.array().square();
    std::cout << "squared difference:\n" << difference << std::endl;

    // apply Gaussian to difference
    VectorXd exponent = difference / -2.0 * sigma * sigma;
    std::cout << "difference / -2sigma^2:\n" << exponent << std::endl;
    exponent.array().exp();
    std::cout << "e^(difference / -2sigma^2):\n" << exponent << std::endl;
    double denom = sqrt(2.0 * M_PI * sigma);
    std::cout << "denom (sqrt(2*pi*sigma):\n" << denom << std::endl;
    MatrixXd gaussian_matrix = exponent / denom;
    std::cout << "Gaussian matrix of difference:\n" << gaussian_matrix << std::endl;

    // calculate weight and normalization for regression
    double summation = gaussian_matrix.sum();
    std::cout << "sum of Gaussian matrix of difference:\n" << summation << std::endl;
    // note: may need to transpose gaussian here...
    gaussian_matrix.transposeInPlace();
    MatrixXd output = gaussian_matrix * Z;
    std::cout << "Gaussian matrix * Z (latent space coords generated during model's training):\n" << output << std::endl;
    //VectorXd newZ = output.rowwise().sum();
    //std::cout << "New z_coord from rowwise summation of G * Z:\n" << newZ << std::endl;
    VectorXd newZ = output.transpose();
    std::cout << "for comparison, here's the first z_coord from the training data:\n" << Z.row(0) << std::endl;


    return newZ;
    //return Z.row(0); // just to make sure everything around the function is working
  }

private:
  // Shapeodds model 
  std::set<unsigned> sample_indices;        // indices of images used to construct this model
  Eigen::MatrixXd Z;  
  Eigen::MatrixXd W;
  Eigen::MatrixXd w0;

  std::string fieldname;
  Eigen::Map<Eigen::VectorXd> fieldvalues;  
  double min_fieldval, max_fieldval;

  friend class ShapeOdds;
};


///////////////////////////////////////////////////////////////////////////////
// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds // : public Model
{
public:
  ShapeOdds();
  ~ShapeOdds();

  static Eigen::MatrixXd evaluateModel(const Model &model, const Eigen::VectorXd &z_coord, const bool writeToDisk = false,
                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0);
  
  static float testEvaluateModel(const Model &model, const Eigen::Matrix<double, 1, Eigen::Dynamic> &z_coord,
                                 const unsigned p, const unsigned c, const unsigned z_idx, const Image &sampleImage,
                                 const bool writeToDisk = false, const std::string path = "");
  
private:

  Eigen::MatrixXd my_V_matrix;
  Eigen::MatrixXi my_F_matrix;

  std::vector<Model> models;
};


///////////////////////////////////////////////////////////////////////////////
class SharedGP // : public Model
{
public:
  SharedGP();
  ~SharedGP();

  int doSomething(int x=42);
  
private:
  int do_something_quietly(int y);

  Eigen::MatrixXd my_V_matrix;
  Eigen::MatrixXi my_F_matrix;

};

} // end namespace PModels
