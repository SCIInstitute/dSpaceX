#pragma once

#include "dspacex/Precision.h"
#include "utils/StringUtils.h"
#include "Models.h"

namespace dspacex {

/// When a Morse-Smale complex is used to decompose/partition a dataset, the samples associated
/// with each crystal at each persistence level can be used to learn a probabilistic
/// interpolation model (e.g., ShapeOdds, PCA, SharedGP). These classes provide access to such
/// sets of models.

/// Crystal contains the models constructed from this set of samples
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


/// Persistence contains the set of Crystals and their global embeddings (common 2d latent
/// space for all crystals at that level)
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

/// MSComplex is a model container that inclues the parameters used to compute the M-S using
/// NNMSComplex, as well as sub-containers of persistince levels and their crystals, which
/// ultimately store the models of this complex.
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

  // TODO: add parameters used by NNMSComplex to create this so the complex can be predictively recomputed
  //void addParams(...); //e.g., fieldname, persistenceLevel, knn, sigma, smooth, noise, numPersistences, numSamples, normalize
  //Params getParams() const;
  // TODO: store persistence range for this complex since not all levels have to be computed

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

