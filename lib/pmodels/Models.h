#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "imageutils/Image.h"

namespace dspacex {

// Probabilistic (predictive) models such as ShapeOdds, InfShapeOdds, GP, SharedGP, etc
//
// There is a model for each crystal at each persistence level for a given M-S topology. It is
// constructed from a given number of input images (samples), and consists of L necessary
// values that make the model able to compute new images for a given point, z, in the latent
// space.
//
// TODO: provide common interface for ShapeOdds, InfiniteShapeOdds, and SharedGP models
// TODO: a model and its associated crystal should know to which fieldname it belongs, right?
// todo: rename to PredictiveModel
class Model
{
public:

  // fieldvalue and the index of its sample in the full set of samples for this dataset
  struct ValueIndexPair
  {
    float val;
    unsigned idx;

    static bool compare(const ValueIndexPair &p, const ValueIndexPair &q) { return p.val < q.val; }
  };


  // todo: this belongs with ShapeOdds
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

  const std::vector<ValueIndexPair>& getSampleIndices() const
  {
    return fieldvalues_and_indices;
  }

  const Eigen::VectorXd getZCoord(const unsigned global_idx) const
  {
    if (global_idx >= Z.size())
      throw std::runtime_error("cannot return " + std::to_string(global_idx) + "th z_coord because there are only " + std::to_string(Z.size()) + " samples in the dataset.");
    
    return Z.row(global_idx);
  }

  // note: fieldname is part of the mscomplex in which this crystal lives, so maybe "getparent" would be better...
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
    fieldvalues_and_indices.resize(sample_indices.size());
    fieldvalues.resize(sample_indices.size());
    {
      unsigned i = 0;
      for (auto idx : sample_indices)
      {
        fieldvalues_and_indices[i].idx = idx;
        fieldvalues_and_indices[i].val = values(idx);
        fieldvalues(i++) = values(idx);
      }
    }

    // sort by increasing fieldvalue... TODO: maybe addSample should add its corresponding field value rather than this awkward fcn
    std::sort(fieldvalues_and_indices.begin(), fieldvalues_and_indices.end(), ValueIndexPair::compare);

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

  double minFieldValue() const
  {
    return fieldvalues.minCoeff();
  }

  double maxFieldValue() const
  {
    return fieldvalues.maxCoeff();
  }

  const Eigen::RowVectorXd getNewLatentSpaceValue(double new_fieldval, double sigma = 0.25) const
  {
    //debug: hardcode new fieldval
    //new_fieldval = 0.62341;
    //std::cout << "num_samples: " << sample_indices.size() << std::endl;
    //std::cout << "z-size: " << z_coords.cols() << std::endl;
    
    // gaussian kernel regression to generate a new LSV
    using namespace Eigen;

    // calculate difference
    RowVectorXd fieldvals(fieldvalues);
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
    double denom = sqrt(2.0 * M_PI * sigma);
    //std::cout << "denom (sqrt(2*pi*sigma):\n" << denom << std::endl;
    fieldvals /= denom;
    //std::cout << "Gaussian matrix of difference:\n" << fieldvals << std::endl;

    // calculate weight and normalization for regression
    double summation = fieldvals.sum();
    //std::cout << "sum of Gaussian vector of difference:\n" << summation << std::endl;

    MatrixXd output = fieldvals * z_coords;
    //std::cout << "output before division:\n" << output << std::endl;
    output /= summation;
    //RowVectorXd output = (fieldvals * z_coords) / summation;
    //std::cout << "new z_coord:\n" << output << std::endl;
    //std::cout << "for comparison, here's the first z_coord from the training data:\n" << z_coords.row(0) << std::endl;

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
  std::vector<ValueIndexPair> fieldvalues_and_indices;  //TODO: this feels pretty hokey...

  friend class ShapeOdds;
};


///////////////////////////////////////////////////////////////////////////////
// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds // : public Model (TODO)
{
public:
  static Eigen::MatrixXd evaluateModel(const Model &model, const Eigen::VectorXd &z_coord, const bool writeToDisk = false,
                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0);
  
  static float testEvaluateModel(const Model &model, const Eigen::Matrix<double, 1, Eigen::Dynamic> &z_coord,
                                 const unsigned p, const unsigned c, const unsigned z_idx, const Image &sampleImage,
                                 const bool writeToDisk = false, const std::string path = "");

private:
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

} // dspacex
