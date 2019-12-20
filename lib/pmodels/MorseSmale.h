#pragma once

#include "precision/Precision.h"
#include "utils/StringUtils.h"
#include "Models.h"

namespace PModels {  // Probabilistic models such as ShapeOdds, InfShapeOdds, GP, SharedGP, etc

//
// A set of probabilistic models are learned using the samples associated with the crystals at
// each persistenve level of a Morse-Smale complex. While a Model can
// exist independently, these structures help organize the group of models for a given M-S:
//
//  MSModelContainer knows the total number of samples for this M-S, contains its set of Persistence levels
//   - Persistences contains set of Crystals and their global embeddings (common 2d latent space for all crystals at that level)
//     - Crystal contains its model, which contains indices of its samples and its local embedding (latent space), along with W and w0.
//
// Looking forward...
//      There could be different models associated with a given crystal (e.g., ShapeOdds or SharedGP of various types),
//      so we could either provide access to a vector of Models or allow them to be queried by type. The latter might
//      not be easy since there could be multiple versions of a single type (ex: using different parameters for learning).
//
//      At least for ShapeOdds, persistences also contain a global embedding (latent space) for evaulation in a common
//      space for all of its crystals. 
//      
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


// Associates a model with it's name (pXXcYY)
typedef std::pair<std::string, Model&> ModelPair;

// Morse-Smale model container, a M-S complex for a given field and the models learned for each of its crystals.
class MSComplex
{
public:
  MSComplex(std::string &field, unsigned nSamples, unsigned nPersistences)
    : fieldname(field), num_samples(nSamples), persistence_levels(nPersistences)
  {}
  ~MSComplex()
  {}

  MSComplex(const MSComplex &m) : fieldname(m.fieldname), num_samples(m.num_samples), persistence_levels(m.persistence_levels)
  {
    //std::cout << "PModels::MSComplex copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSComplex operator=(const MSComplex &m)
  {
    //std::cout << "PModels::MSComplex assignment operator (&model = " << &m << ")." << std::endl;
    return MSComplex(m);
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
    // <ctc> TODO: the number of persistence levels should be used to adjust the requested plvl from the PModels::MSComplex
    //             sort of a chicken-and-egg problem since the shapeodds models don't realize there are more plvls than are computed
    unsigned persistence_idx = i;// - 14; // <ctc> hack since persistence levels are numbered 0-19 in shapeodds output for CantileverBeam
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



} // end namespace PModels

