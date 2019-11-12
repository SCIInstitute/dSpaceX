#pragma once

#include <Eigen/Core>
#include <iostream>
#include <vector>
#include <set>
#include "flinalg/Linalg.h"
#include "precision/Precision.h"

//std::pair<std::string, Shapeodds::Model&> ModelPair;  // moving foward this could be a ShapeOdds model or a SharedGP model

//<ctc> TODO: replace 'unsigned' indices with 'size_type'

namespace Shapeodds {

// There is a model for each crystal at each persistence level for a given M-S topology. It is
// constructed from a given number of input images (samples), and consists of L necessary
// values that make the model able to compute new images for a given point, z, in the latent
// space.
class Model
{
public:
  Model(FortranLinalg::DenseMatrix<Precision> _Z, FortranLinalg::DenseMatrix<Precision> _W, FortranLinalg::DenseMatrix<Precision> _w0)
    : Z(_Z), W(_W), w0(_w0)
  {}
  ~Model()
  {}
  
  // since models may get large, keep track of when they're copyied so this step can be optimized (probably passing vectors around by value is copying their contents). C++11 should use move semantics (see https://mbevin.wordpress.com/2012/11/20/move-semantics/), so this is just to verify that's being done.
  Model(const Model &model) : Z(model.Z), W(model.W), w0(model.w0)
  {
    std::cout << "Shapeodds::Model copy ctor (&model = " << &model << ")." << std::endl;
  }
  Model operator=(const Model &m)
  {
    std::cout << "Shapeodds::Model assignment operator (&model = " << &m << ")." << std::endl;
    return Model(m);
  }
  
  //private:
  // Shapeodds model 
  FortranLinalg::DenseMatrix<Precision> Z;
  FortranLinalg::DenseMatrix<Precision> W;
  FortranLinalg::DenseMatrix<Precision> w0;

  friend class ShapeOdds;
};

//
// A set of Shapeodds models are computed for elements of a Morse-Smale complex, with one model
// per Crystal of each Persistence level. These persistences also contain a global embedding
// (latent space) for evaulation in a common space for all of its crystals.  While a Model can
// exist independently, these structures help organize the group of models for a given M-S:
//
//  MSModelContainer knows the total number of samples for this M-S, contains set of Persistence levels
//   - Persistences contains set of Crystals and their global embeddings (latent space)
//     - Crystal contains indices of its samples and local embedding (latent space)
//
// Note a vector of ModelPairs for all models of the MSModelContainer can be retrieved using
// its getAllModels function.
//

class Crystal
{
public:
  Crystal(Model m) : model(m)
  {}
  ~Crystal()
  {}

  void addSample(unsigned n)
  {
    samples.insert(n);
  }

  unsigned numSamples() const
  {
    return samples.size();
  }

  Model& getModel()
  {
    return model;
  }

private:
  std::set<unsigned> samples;
  Model model;
};

class PersistenceLevel
{
public:
  PersistenceLevel()
  {}
  ~PersistenceLevel()
  {}

  unsigned numCrystals() const
  {
    return crystals.size();
  }

  void addCrystal(Crystal c)
  {
    crystals.push_back(c);
  }

  Crystal& getCrystal(unsigned i)
  {
    return crystals[i];
  }
  
  void setGlobalEmbeddings(FortranLinalg::DenseMatrix<Precision> embeddings)
  {
    global_embeddings = embeddings;
  }

  // each crystal is composed of a non-intersecting set of samples, read from this vector
  void setCrystalSamples(FortranLinalg::DenseVector<Precision> crystal_ids)
  {
    // NOTE: all crytals must have been added or these crystal ids will be out of range
    for (unsigned n = 0; n < crystal_ids.N(); n++)
    {
      crystals[crystal_ids(n)].addSample(n);
    }
  }

private:
  std::vector<Crystal> crystals;
  FortranLinalg::DenseMatrix<Precision> global_embeddings;
};

// Morse-Smale model container
class MSModelContainer
{
public:
  MSModelContainer(std::string &field, unsigned nSamples) : fieldname(field), num_samples(nSamples)
  {}
  ~MSModelContainer()
  {}

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

  void addPersistenceLevel(PersistenceLevel &l)
  {
    persistence_levels.push_back(l);
  }

  void addPersistenceLevel()
  {
    PersistenceLevel plvl;
    persistence_levels.push_back(plvl);
  }

  PersistenceLevel& getPersistenceLevel(unsigned i)
  {
    return persistence_levels[i];
  }

  //<ctc> maybe add a helper to get the model from a specific crystal of a specific persistence level all in one query
  //      then we can check for errors at the top level, since it'll be tedious to make users of this class do it every time.


  typedef std::pair<std::string, Model&> ModelPair;
  std::vector<ModelPair> getAllModels()
  {
    std::vector<ModelPair> models;  
    for (unsigned p = 0; p < persistence_levels.size(); p++)
    {
      for (unsigned c = 0; c < persistence_levels[p].numCrystals(); c++)
      {
        std::string modelName("p"+std::to_string(p)+"-c"+std::to_string(c));
        models.push_back(ModelPair(modelName, persistence_levels[p].getCrystal(c).getModel()));
      }
    }
    return models;
  }
  
private:
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

  static bool evaluateModel(Model &model, FortranLinalg::DenseMatrix<Precision> &z_coord);

  int doSomething(int x=42);
  
private:
  int do_something_quietly(int y);

  Eigen::MatrixXd my_V_matrix;
  Eigen::MatrixXi my_F_matrix;

  std::vector<Model> models;

};

}
