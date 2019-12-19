#pragma once

#include <Eigen/Core>
#include <iostream>
#include <vector>
#include <set>
#include "precision/Precision.h"
#include "utils/StringUtils.h"
#include "imageutils/Image.h"

namespace PModels {  // Probabilistic models such as ShapeOdds, InfShapeOdds, GP, SharedGP, etc

class Model;
typedef std::pair<std::string, Model&> ModelPair;  // moving foward this could be a ShapeOdds model or a SharedGP model

// There is a model for each crystal at each persistence level for a given M-S topology. It is
// constructed from a given number of input images (samples), and consists of L necessary
// values that make the model able to compute new images for a given point, z, in the latent
// space.
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


// class ShapeOddsModel : public Model
// TODO
//  - in order to provide common interface for ShapeOdds, InfiniteShapeOdds, and SharedGP models


//
// A set of PModels models are computed for elements of a Morse-Smale complex, with one model
// per Crystal of each Persistence level. These persistences also contain a global embedding
// (latent space) for evaulation in a common space for all of its crystals.  While a Model can
// exist independently, these structures help organize the group of models for a given M-S:
//
//  MSModelContainer knows the total number of samples for this M-S, contains its set of Persistence levels
//   - Persistences contains set of Crystals and their global embeddings (common 2d latent space for all crystals at that level)
//     - Crystal contains its model, which contains indices of its samples and its local embedding (latent space), along with W and w0.
//
// Note a vector of ModelPairs for all models of the MSModelContainer can be retrieved using
// its getAllModels function.
//

class MSCrystal
{
public:
  MSCrystal()
  {}
  ~MSCrystal()
  {}

  MSCrystal(const MSCrystal &m) : model(m.model)
  {
    //std::cout << "PModels::MSCrystal copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSCrystal operator=(const MSCrystal &m)
  {
    //std::cout << "PModels::MSCrystal assignment operator (&m = " << &m << ")." << std::endl;
    return MSCrystal(m);
  }
  
  void addSample(unsigned n)
  {
    model.addSample(n);
  }

  unsigned numSamples() const
  {
    return model.numSamples();
  }

  const std::set<unsigned>& getSampleIndices() const
  {
    return model.getSampleIndices();
  }

  Model& getModel()
  {
    return model;
  }

private:
  Model model;
};

class MSPersistenceLevel
{
public:
  MSPersistenceLevel()
  {}
  ~MSPersistenceLevel()
  {}

  MSPersistenceLevel(const MSPersistenceLevel &m) : crystals(m.crystals), global_embeddings(m.global_embeddings)
  {
    //std::cout << "PModels::MSPersistenceLevel copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSPersistenceLevel operator=(const MSPersistenceLevel &m)
  {
    //std::cout << "PModels::MSPersistenceLevel assignment operator (&m = " << &m << ")." << std::endl;
    return MSPersistenceLevel(m);
  }
  
  void setNumCrystals(unsigned nCrystals)
  {
    crystals.resize(nCrystals);
  }

  unsigned numCrystals() const
  {
    return crystals.size();
  }

  MSCrystal& getCrystal(unsigned i)
  {
    return crystals[i];
  }
  
  void setGlobalEmbeddings(const Eigen::MatrixXd &embeddings)
  {
    global_embeddings = embeddings;
  }

  // each crystal is composed of a non-intersecting set of samples, read from this vector
  void setCrystalSamples(Eigen::MatrixXi &crystal_ids)
  {
    // NOTE: all crystals must have been added or these crystal ids will be out of range      

    for (unsigned n = 0; n < crystal_ids.rows(); n++)
    {
      crystals[crystal_ids(n)].addSample(n);
    }
  }

private:
  std::vector<MSCrystal> crystals;
  Eigen::MatrixXd global_embeddings;
};

// Morse-Smale model container
class MSModelContainer
{
public:
  MSModelContainer(std::string &field, unsigned nSamples, unsigned nPersistences)
    : fieldname(field), num_samples(nSamples), persistence_levels(nPersistences)
  {}
  ~MSModelContainer()
  {}

  MSModelContainer(const MSModelContainer &m) : fieldname(m.fieldname), num_samples(m.num_samples), persistence_levels(m.persistence_levels)
  {
    //std::cout << "PModels::MSModelContainer copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSModelContainer operator=(const MSModelContainer &m)
  {
    //std::cout << "PModels::MSModelContainer assignment operator (&model = " << &m << ")." << std::endl;
    return MSModelContainer(m);
  }
  
  std::string getFieldname() const
  {
    return fieldname;
  }

  unsigned numSamples() const
  {
    return num_samples;
  }

  unsigned numPersistenceLevels() const
  {
    return persistence_levels.size();
  }

  MSPersistenceLevel& getPersistenceLevel(unsigned i)
  {
    unsigned persistence_idx = i - 14; // <ctc> hack since persistence levels are numbered 0-19 in shapeodds output for CantileverBeam
    return persistence_levels[persistence_idx];
  }

  ModelPair getModel(unsigned p, unsigned c);
  std::vector<ModelPair> getAllModels();
  
private:
  static std::string modelName(unsigned p, unsigned c, unsigned persistence_padding = 2, unsigned crystals_padding = 2)
  {
    unsigned actual_persistence_level = p + 14; // <ctc> hack since persistence levels are numbered 0-19 in shapeodds output for CantileverBeam
    return std::string("p"+paddedIndexString(actual_persistence_level, persistence_padding)+"c"+paddedIndexString(c, crystals_padding));
  }

  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
  std::vector<MSPersistenceLevel> persistence_levels;
};




// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds
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

} // end namespace PModels

