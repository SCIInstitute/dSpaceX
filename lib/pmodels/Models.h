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
  
  void setModel(Eigen::MatrixXf _W, Eigen::MatrixXf _w0, Eigen::MatrixXf _Z)
  {
    W  = _W;
    w0 = _w0;
    Z  = _Z; // latent space coords for samples used to generate this model
  }

  // void addSample(unsigned n)
  // {
  //   sample_indices.push_back(n);
  // }

  // const unsigned numSamples() const
  // {
  //   return sample_indices.size();
  // }

  // const std::vector<ValueIndexPair>& getSampleIndices() const
  // {
  //   return fieldvalues_and_indices;
  // }

  const Eigen::VectorXf getZCoord(unsigned local_idx) const
  {
    // // requires a z_coords for all samples in the dataset, but some models (PCA) only have the coords for their own samples
    // // so... todo: create a map from global idx to local idx to access a z_coord for model regeneration/verification
    // if (global_idx >= Z.size())
    //   throw std::runtime_error("cannot return " + std::to_string(global_idx) + "th z_coord because there are only " + std::to_string(Z.size()) + " samples in the dataset.");
    
    // todo: map nth global_idx of this crystal's samples, which corresponds to nth row of Z

    // z_coords for this model are ordered by the global order of samples used to construct it
    return Z.row(local_idx);
  }

  // const Eigen::RowVectorXf& getFieldValues()
  // {
  //   if (!fieldvalues)  // have the fieldvalues been retrieved and cached?
  //   {
  //     // fill fieldvalues from modelset
  //     for (auto idx : crystal->getSampleIndices())
  //       fieldvalues.push_back(crystal->getSample(idx));
  //     std::sort(fieldvalue); // necessary? <ctc> try not doing it, purposely rearrange, think about math
  //   }
  //   return fieldvalues;
  // }
    
  /*
  void setFieldValues(Eigen::Map<Eigen::VectorXf> values)
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

    ...but even then there would be tons of duplicated data ( n * npersistences). tgere is only one set of fields. this should go in modelset-- no, even simply dataset, then be read by model when computing new samples
       . oh. hmm.... maybe that;s what is being done
                                -> How about this idea: each model knows which dataset to which it belongs. No more dancing around
    std::sort(fieldvalues_and_indices.begin(), fieldvalues_and_indices.end(), ValueIndexPair::compare);

    // // TODO: odd place to put this... the reason is that the sample_indices aren't there when the model is set above
    // z_coords.resize(sample_indices.size(), Z.cols());
    // {
    //   unsigned i = 0;
    //   for (auto idx : sample_indices)
    //   {
    //     // ShapeOdds store a latent space coord per sample in the entire dataset
    //     //z_coords.row(i++) = Z.row(idx); // ...so not reading them like this breaks ShapeOdds (fixme)

    //     // PCA models only have a z_coord for shapes used to construct them, so this is simpler:
    //     z_coords.row(i) = Z.row(i);
    //     i++;
    //   }
    // }
  }
  */
  
  // todo: this one is still needed (by fetchNImagesForCrystal), but fieldvaues for the model are kinda stepping back 
  float minFieldValue() const
  {
    return fieldvalues->minCoeff();
  }

  float maxFieldValue() const
  {
    return fieldvalues->maxCoeff();
  }

  /// computes new z_coord at this field value
  const Eigen::RowVectorXf getNewLatentSpaceValue(const Eigen::RowVectorXf& fieldvalues, float new_fieldval, float sigma = 0.25) const
  {
    //debug: hardcode new fieldval
    //new_fieldval = 0.62341;
    //std::cout << "num_samples: " << sample_indices.size() << std::endl;
    //std::cout << "z-size: " << z_coords.cols() << std::endl;
    
    // gaussian kernel regression to generate a new LSV
    using namespace Eigen;

    // calculate difference
    RowVectorXf fieldvals(fieldvalues); // <ctc> do this: convert this to a static function and just pass in z_coords (or fieldvalues?) (for evaluation of multiple model types (PCA or ShapeOdds) for DARPA June meeting)
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

private:
  Type type{None};

  // Shapeodds model 
  std::vector<unsigned> sample_indices;     // indices of images used to construct this model
  Eigen::MatrixXf z_coords;                 // latent space coordinates of samples used to learn this model
  Eigen::MatrixXf Z;                        // ALL latent space coordinates of the dataset (todo: don't store these in the model)
  Eigen::MatrixXf W;
  Eigen::MatrixXf w0;

  //std::string fieldname;
  std::unique_ptr<Eigen::RowVectorXf> fieldvalues;
  std::vector<ValueIndexPair> fieldvalues_and_indices;  //TODO: this feels pretty hokey...

  friend class ShapeOdds;
};


///////////////////////////////////////////////////////////////////////////////
// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds // : public Model (TODO)
{
public:
  static Eigen::MatrixXf evaluateModel(const Model &model, const Eigen::VectorXf &z_coord, const bool writeToDisk = false,
                                       const std::string outpath = "", unsigned w = 0, unsigned h = 0);
  
  static float testEvaluateModel(const Model &model, const Eigen::Matrix<float, 1, Eigen::Dynamic> &z_coord,
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

  Eigen::MatrixXf my_V_matrix;
  Eigen::MatrixXi my_F_matrix;

};

std::ostream& operator<<(std::ostream &os, const Model::Type& type);

} // dspacex
