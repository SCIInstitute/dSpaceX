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

  const Eigen::VectorXd getNewLatentSpaceValue(double fieldval)
  {
    return Z.row(0); // just to make sure everything around the function is working

    // do the proper gaussian kernel regression to generate a new LSV
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
