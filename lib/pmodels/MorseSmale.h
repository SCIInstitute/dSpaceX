#pragma once

#include "dspacex/Precision.h"
#include "utils/StringUtils.h"
#include "Models.h"

namespace dspacex {

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
  void addSample(unsigned n)
  {
    model.addSample(n);
  }

  unsigned numSamples() const
  {
    return model.numSamples();
  }

  const std::vector<Model::ValueIndexPair>& getSampleIndices() const
  {
    return model.getSampleIndices();
  }

  Model& getModel() //todo: return by model type since a crystal can have more than one model. Maybe have to return a pointer in case model doesn't exist. Could a crystal have more than one of the same type of model? Seems possible since differently-constructed models may want to be compared.
  {
    return model;  //todo: which model?
  }
  
  // void addModel(Model &m)
  // {
  //   models.append(m)
  // }

private:
  // todo: there can be more than one model per crystal (say, one per qoi and one per design param and one per image/dt
  // todo: the crystal probably shouldn't own the model(s) since they're technically independent of the crystal
  Model model;
};

class MSPersistenceLevel
{
public:
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


// Associates a model with it's name (pXXcYY) // todo: this is an issue because there could be more than one model with the same name (in another MSComplex)
typedef std::pair<std::string, Model&> ModelPair;

// Morse-Smale model container, a M-S complex for a given field and the models learned for each of its crystals.
class MSComplex
{
public:
  MSComplex(std::string &field, unsigned nSamples, unsigned nPersistences)
    : fieldname(field), num_samples(nSamples), persistence_levels(nPersistences)
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

  MSPersistenceLevel& getPersistenceLevel(unsigned idx)
  {
    return persistence_levels[idx];
  }

  bool hasModel(unsigned p, unsigned c) const;
  ModelPair getModel(unsigned p, unsigned c);
  std::vector<ModelPair> getAllModels();
  
private:
  static std::string modelName(unsigned p, unsigned c, unsigned persistence_padding = 2, unsigned crystals_padding = 2)
  {
    return std::string("p"+paddedIndexString(p, persistence_padding)+"c"+paddedIndexString(c, crystals_padding));
  }

  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
  std::vector<MSPersistenceLevel> persistence_levels;
};



} // dspacex

