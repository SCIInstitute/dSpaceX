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
  Model()
  {
    std::cout << "PModels::Model ctor." << std::endl;
  }
  ~Model()
  {
    std::cout << "PModels::Model dtor." << std::endl;
  }
  Model(const Model &m) : Z(m.Z), W(m.W), w0(m.w0), sample_indices(m.sample_indices)
  {
    std::cout << "PModels::Model copy ctor (&m = " << &m << ")." << std::endl;
  }
  Model operator=(const Model &m)
  {
    std::cout << "PModels::Model assignment operator (&m = " << &m << ")." << std::endl;
    return Model(m);
  }
  Model(Model &&c) = default;
  Model& operator=(Model &&c) = default;
  
  void setModel(Eigen::MatrixXd _W, Eigen::MatrixXd _w0, Eigen::MatrixXd _Z)
  {
    W  = _W;
    w0 = _w0;
    Z  = _Z;  // todo: maybe better *not* to store this at all since the model used a subset and indices could be a reverse reference to sample
  }

  void addSample(unsigned n)
  {
    sample_indices.push_back(n);
  }

  const unsigned numSamples() const
  {
    return sample_indices.size();
  }

  const std::vector<unsigned>& getSampleIndices() const
  {
    return sample_indices;
  }
  
  const Eigen::VectorXd getZCoord(const unsigned idx) const
  {
    if (idx >= numSamples())
      throw std::runtime_error("cannot return " + std::to_string(idx) + "th z_coord because there are only " + std::to_string(numSamples()) + " samples in this model.");
    
    return Z.row(sample_indices[idx]);
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
    fieldvalues.resize(sample_indices.size());
    {
      unsigned i = 0;
      for (auto idx : sample_indices)
      {
        fieldvalues(i++) = values(idx);
      }
    }

    // TODO: odd place to put this... the reason is that the sample_indices aren't there when the model is set above
    z_coords.resize(sample_indices.size(), Z.cols());
    {
      unsigned i = 0;
      for (auto idx : sample_indices)
      {
        z_coords.row(i++) = Z.row(idx);
      }
    }
  }

  double minFieldValue()
  {
    return fieldvalues.minCoeff();
  }

  double maxFieldValue()
  {
    return fieldvalues.maxCoeff();
  }

  const Eigen::RowVectorXd getNewLatentSpaceValue(double new_fieldval, double sigma = 0.25)
  {
    //debug: hardcode new fieldval
    //new_fieldval = 0.62341;
    std::cout << "num_samples: " << sample_indices.size() << std::endl;
    std::cout << "z-size: " << z_coords.cols() << std::endl;
    
    // gaussian kernel regression to generate a new LSV
    using namespace Eigen;

    // calculate difference
    RowVectorXd fieldvals(fieldvalues);
    fieldvals *= -1.0;
    fieldvals.array() += new_fieldval;
    std::cout << "difference between new field value and training field values:\n" << fieldvals << std::endl;

    // apply Gaussian to difference
    fieldvals = fieldvals.array().square();
    std::cout << "squared difference:\n" << fieldvals << std::endl;
    fieldvals /= (-2.0 * sigma * sigma);
    std::cout << "difference / -2sigma^2:\n" << fieldvals << std::endl;
    fieldvals = fieldvals.array().exp();
    std::cout << "e^(difference / -2sigma^2):\n" << fieldvals << std::endl;
    double denom = sqrt(2.0 * M_PI * sigma);
    std::cout << "denom (sqrt(2*pi*sigma):\n" << denom << std::endl;
    fieldvals /= denom;
    std::cout << "Gaussian matrix of difference:\n" << fieldvals << std::endl;

    // calculate weight and normalization for regression
    double summation = fieldvals.sum();
    std::cout << "sum of Gaussian vector of difference:\n" << summation << std::endl;

    //gaussian_vector.transposeInPlace();
    // MatrixXd output = gaussian_vector * z_coords;
    // std::cout << "Gaussian matrix * z_coords (latent space coords generated during model's training):\n" << output << std::endl;
    //VectorXd newZ = output.rowwise().sum();
    //std::cout << "New z_coord from rowwise summation of G * z_coords:\n" << newZ << std::endl;
    //VectorXd newZ = output.transpose();

    MatrixXd output = fieldvals * z_coords;
    std::cout << "output before division:\n" << output << std::endl;
    output /= summation;
    //RowVectorXd output = (fieldvals * z_coords) / summation;
    std::cout << "output after division:\n" << output << std::endl;
    std::cout << "for comparison, here's the first z_coord from the training data:\n" << z_coords.row(0) << std::endl;

    return output;
  }

private:
  // Shapeodds model 
  std::vector<unsigned> sample_indices;        // indices of images used to construct this model
  Eigen::MatrixXd z_coords;                 // latent space coordinates of samples used to learn this model
  Eigen::MatrixXd Z;                        // ALL latent space coordinates of the dataset (todo: don't store these in the model)
  Eigen::MatrixXd W;
  Eigen::MatrixXd w0;

  std::string fieldname;
  Eigen::RowVectorXd fieldvalues;  

  friend class ShapeOdds;
};


///////////////////////////////////////////////////////////////////////////////
// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds // : public Model
{
public:
  ShapeOdds();
  ~ShapeOdds()
  {
    std::cout << "PModels::ShapeOdds dtor." << std::endl;
  }
  ShapeOdds(const ShapeOdds &m) : my_V_matrix(m.my_V_matrix), my_F_matrix(m.my_F_matrix), models(m.models)
  {
    std::cout << "PModels::ShapeOdds copy ctor (&m = " << &m << ")." << std::endl;
  }
  ShapeOdds operator=(const ShapeOdds &m)
  {
    std::cout << "PModels::ShapeOdds assignment operator (&m = " << &m << ")." << std::endl;
    return ShapeOdds(m);
  }
  ShapeOdds(ShapeOdds &&c) = default;
  ShapeOdds& operator=(ShapeOdds &&c) = default;

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
