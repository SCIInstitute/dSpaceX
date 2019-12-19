#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "imageutils/Image.h"

namespace PModels {

class Model;
typedef std::pair<std::string, Model&> ModelPair;

// There is a model for each crystal at each persistence level for a given M-S topology. It is
// constructed from a given number of input images (samples), and consists of L necessary
// values that make the model able to compute new images for a given point, z, in the latent
// space.
//
// TODO: provide common interface for ShapeOdds, InfiniteShapeOdds, and SharedGP models
class Model
{
public:
  Model()
  {}
  ~Model()
  {}

  // since models may get large, keep track of when they're copyied so this step can be optimized (probably passing vectors around by value is copying their contents). C++11 should use move semantics (see https://mbevin.wordpress.com/2012/11/20/move-semantics/), so this is just to verify that's being done.
  Model(const Model &m) : Z(m.Z), W(m.W), w0(m.w0), sample_indices(m.sample_indices)
  {
    //std::cout << "PModels::Model copy ctor (&m = " << &m << ")." << std::endl;
  }
  Model operator=(const Model &m)
  {
    //std::cout << "PModels::Model assignment operator (&m = " << &m << ")." << std::endl;
    return Model(m);
  }
  
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

private:
  // Shapeodds model 
  std::set<unsigned> sample_indices;        // indices of images used to construct this model
  Eigen::MatrixXd Z;  
  Eigen::MatrixXd W;
  Eigen::MatrixXd w0;

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

  int doSomething(int x=42);
  
private:
  int do_something_quietly(int y);

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
