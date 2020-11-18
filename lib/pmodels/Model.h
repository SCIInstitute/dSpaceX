#pragma once

#include <set>
#include <vector>
#include <iostream>
#include <Eigen/Core>
#include "imageutils/Image.h"
#include "dataset/Fieldtype.h"

namespace dspacex {

/// Probabilistic interpolation models such as ShapeOdds, InfShapeOdds, SharedGP, and PCA.
/// They are learned from a set of input designs (images/samples, parameters, and qois), and
/// consist of L necessary values that enable the model to compute new samples for a given
/// point, z, in its latent space.
class Model
{
public:
  enum Type { PCA, ShapeOdds, InfShapeOdds, SharedGP, Custom, None = 0 };
  static std::string typeToStr(const Type& type);
  static Type strToType(const std::string& type);

  /// factory function
  static std::unique_ptr<Model> create(Type t, const std::string& name = std::string());

  Model(Type t = None) : type(t) {}
  virtual ~Model() = default;
  
  void setModel(Eigen::MatrixXf _W, Eigen::MatrixXf _w0, Eigen::MatrixXf _Z) {
    W  = _W;
    w0 = _w0;
    Z  = _Z; // latent space coords for samples used to generate this model
  }

  void setBounds(std::pair<float, float> minmax) {
    minval = minmax.first;
    maxval = minmax.second;
  }

  const Eigen::MatrixXf& getZCoords() const { return Z; }
  const Eigen::VectorXf getZCoord(unsigned local_idx) const { return Z.row(local_idx); }

  float minFieldValue() const { return minval; }
  float maxFieldValue() const { return maxval; }

  /// return a precomputed interpolation from the specified set (todo)
  Eigen::MatrixXf fetchInterpolation(int idx, int interpolationSet = 0) const;

  /// computes new z_coord at this field value
  static const Eigen::RowVectorXf getNewLatentSpaceValue(const Eigen::RowVectorXf& fieldvalues, const Eigen::MatrixXf& z_coords,
                                                         float new_fieldval, float sigma = 0.25);

  virtual std::shared_ptr<Eigen::MatrixXf> evaluate(const Eigen::VectorXf &z_coord) const = 0;

  // TODO: create version of evaluate that produces new images for an array of z_coords rather than one at a time
  // virtual std::vector<Eigen::MatrixXf> evaluate(const Eigen::MatrixXf &z_coords, const bool writeToDisk = false,
  //                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0) const = 0;

  Type getType() const { return type; }

protected:
  Eigen::MatrixXf Z; // latent space coordinates of samples used to learn this model, ordered by the global sample idx
  Eigen::MatrixXf W;
  Eigen::MatrixXf w0;
  float minval{0.0f}, maxval{0.0f};  // min amd max fieldvalue of samples used to learn this model

private:
  Type type{None};
};


/* 
 * static functions to evaluate a ShapeOdds model
 */
class ShapeOddsModel : public Model
{
public:
  ShapeOddsModel() : Model{ShapeOdds} {}
  ~ShapeOddsModel() = default;
  std::shared_ptr<Eigen::MatrixXf> evaluate(const Eigen::VectorXf &z_coord) const override;
};

/* 
 * static functions to evaluate a PCA model
 */
class PCAModel : public Model
{
public:
  PCAModel() : Model{PCA} {}
  ~PCAModel() = default;
  std::shared_ptr<Eigen::MatrixXf> evaluate(const Eigen::VectorXf &z_coord) const override;
};

/* 
 * A custom model does not have an evaluate function, but still has associated sets of pre-computed interpolations.
 */
class CustomModel : public Model
{
public:
  CustomModel(std::string _name = std::string()) : Model{Custom}, name(std::move(_name)) {}
  ~CustomModel() = default;
  std::shared_ptr<Eigen::MatrixXf> evaluate(const Eigen::VectorXf &z_coord) const override
  { return std::shared_ptr<Eigen::MatrixXf>(nullptr); }
  
  const std::string name;
};

std::ostream& operator<<(std::ostream &os, const Model::Type& type);

} // dspacex
