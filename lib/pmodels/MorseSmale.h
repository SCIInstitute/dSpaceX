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
  {
    std::cout << "PModels::MSCrystal ctor." << std::endl;
  }
  ~MSCrystal()
  {
    std::cout << "PModels::MSCrystal dtor." << std::endl;
  }

  MSCrystal(const MSCrystal &m) : model(m.model)
  {
    std::cout << "PModels::MSCrystal copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSCrystal operator=(const MSCrystal &m)
  {
    std::cout << "PModels::MSCrystal assignment operator (&m = " << &m << ")." << std::endl;
    return MSCrystal(m);
  }
  
  MSCrystal(MSCrystal &&c) = default;
  MSCrystal& operator=(MSCrystal &&c) = default;

  void addSample(unsigned n)
  {
    model.addSample(n);
  }

  unsigned numSamples() const
  {
    return model.numSamples();
  }

  const std::vector<unsigned>& getSampleIndices() const
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
  {
    std::cout << "PModels::MSPersistenceLevel ctor." << std::endl;
  }
  ~MSPersistenceLevel()
  {
    std::cout << "PModels::MSPersistenceLevel dtor." << std::endl;
  }

  MSPersistenceLevel(const MSPersistenceLevel &m) : crystals(m.crystals), global_embeddings(m.global_embeddings)
  {
    std::cout << "PModels::MSPersistenceLevel copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSPersistenceLevel operator=(const MSPersistenceLevel &m)
  {
    std::cout << "PModels::MSPersistenceLevel assignment operator (&m = " << &m << ")." << std::endl;
    return MSPersistenceLevel(m);
  }
  
  MSPersistenceLevel(MSPersistenceLevel &&c) = default;
  MSPersistenceLevel& operator=(MSPersistenceLevel &&c) = default;

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


// Associates a model with it's name (pXXcYY) // <ctc> an issue? there could be more than one model with the same name (in another MSComplex)
typedef std::pair<std::string, Model&> ModelPair;

// Morse-Smale model container, a M-S complex for a given field and the models learned for each of its crystals.
class MSComplex
{
public:
  MSComplex()
  {
    std::cout << "PModels::MSComplex ctor." << std::endl;
  }
  MSComplex(std::string &field, unsigned nSamples, unsigned nPersistences)
    : fieldname(field), num_samples(nSamples), persistence_levels(nPersistences)
  {
    std::cout << "PModels::MSComplex ctor with initialization vars." << std::endl;
  }
  ~MSComplex()
  {
    std::cout << "PModels::MSComplex dtor." << std::endl;
  }
  MSComplex(const MSComplex &m) : fieldname(m.fieldname), num_samples(m.num_samples), persistence_levels(m.persistence_levels)
  {
    std::cout << "PModels::MSComplex copy ctor (&m = " << &m << ")." << std::endl;
  }
  MSComplex operator=(const MSComplex &m)
  {
    std::cout << "PModels::MSComplex assignment operator (&model = " << &m << ")." << std::endl;
    return MSComplex(m);
  }
  MSComplex(MSComplex &&c) = default;
  MSComplex& operator=(MSComplex &&c) = default;

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
//    unsigned actual_persistence_level = p + 14; // <ctc> hack since persistence levels are numbered 0-19 in shapeodds output for CantileverBeam
// ugh: outside in the Controller we've already computed the correct index for this MS, but internally we don't know. Maybe just keep it that way? Except the model name will be wrong from the outside. ugh. ugh. ugh. Why do they need these names anyway? It's noted above how that could be a conflict with more than one field.
    return std::string("p"+paddedIndexString(p, persistence_padding)+"c"+paddedIndexString(c, crystals_padding));
  }

  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
  std::vector<MSPersistenceLevel> persistence_levels;
};



} // end namespace PModels

