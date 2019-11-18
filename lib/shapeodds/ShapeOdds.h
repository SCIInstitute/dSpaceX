#pragma once

#include <Eigen/Core>
#include <iostream>
#include <vector>
#include <set>
#include "precision/Precision.h"
#include "utils/StringUtils.h"

namespace Shapeodds {  //<ctc> What is a more generic name for ShapeOdds, InfShapeOdds, GP, SharedGP, etc?

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
    std::cout << "Shapeodds::Model copy ctor (&m = " << &m << ")." << std::endl;
  }
  Model operator=(const Model &m)
  {
    std::cout << "Shapeodds::Model assignment operator (&m = " << &m << ")." << std::endl;
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

  unsigned numSamples() const
  {
    return sample_indices.size();
  }

  const std::set<unsigned>& getSampleIndices() const
  {
    return sample_indices;
  }


  //private:         //<ctc> TODO: uncomment me, "friend" DatasetLoader::parseMSModels needs access for testing
  // Shapeodds model 
  std::set<unsigned> sample_indices;        // indices of images used to construct this model
  Eigen::MatrixXd Z;  
  Eigen::MatrixXd W;
  Eigen::MatrixXd w0;

  friend class ShapeOdds;
};


// class ShapeOddsModel : public Model
// TODO
//  - in order to provide common interface for SharedGP models


//
// A set of Shapeodds models are computed for elements of a Morse-Smale complex, with one model
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

class Crystal
{
public:
  Crystal()
  {}
  ~Crystal()
  {}

  Crystal(const Crystal &m) : model(m.model)
  {
    std::cout << "Shapeodds::Crystal copy ctor (&m = " << &m << ")." << std::endl;
  }
  Crystal operator=(const Crystal &m)
  {
    std::cout << "Shapeodds::Crystal assignment operator (&m = " << &m << ")." << std::endl;
    return Crystal(m);
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

class PersistenceLevel
{
public:
  PersistenceLevel()
  {}
  ~PersistenceLevel()
  {}

  PersistenceLevel(const PersistenceLevel &m) : crystals(m.crystals), global_embeddings(m.global_embeddings)
  {
    std::cout << "Shapeodds::PersistenceLevel copy ctor (&m = " << &m << ")." << std::endl;
  }
  PersistenceLevel operator=(const PersistenceLevel &m)
  {
    std::cout << "Shapeodds::PersistenceLevel assignment operator (&m = " << &m << ")." << std::endl;
    return PersistenceLevel(m);
  }
  
  void setNumCrystals(unsigned nCrystals)
  {
    crystals.resize(nCrystals);
  }

  unsigned numCrystals() const
  {
    return crystals.size();
  }

  Crystal& getCrystal(unsigned i)
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

    for (unsigned n = 0; n < crystal_ids.cols(); n++)
    {
      crystals[crystal_ids(n)].addSample(n);
    }
  }

private:
  std::vector<Crystal> crystals;
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
    std::cout << "Shapeodds::MSModelContainer copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSModelContainer operator=(const MSModelContainer &m)
  {
    std::cout << "Shapeodds::MSModelContainer assignment operator (&model = " << &m << ")." << std::endl;
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

  PersistenceLevel& getPersistenceLevel(unsigned i)
  {
    return persistence_levels[i];
  }

  ModelPair getModel(unsigned p, unsigned c);
  std::vector<ModelPair> getAllModels();
  
private:
  static std::string modelName(unsigned p, unsigned c, unsigned persistence_padding = 2, unsigned crystals_padding = 2)
  {
    return std::string("p"+paddedIndexString(p, persistence_padding)+"c"+paddedIndexString(c, crystals_padding));
  }

  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
  std::vector<PersistenceLevel> persistence_levels;
};




// operates on Shapeodds models, e.g., to produce a new image from a latent space coordinate z.
class ShapeOdds
{
public:
  ShapeOdds();
  ~ShapeOdds();

  static bool evaluateModel(Model &model, Eigen::MatrixXd &z_coord);

  int doSomething(int x=42);
  
private:
  int do_something_quietly(int y);

  Eigen::MatrixXd my_V_matrix;
  Eigen::MatrixXi my_F_matrix;

  std::vector<Model> models;
};

}
