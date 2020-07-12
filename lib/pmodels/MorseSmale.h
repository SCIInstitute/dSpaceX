#pragma once

#include "dspacex/Precision.h"
#include "utils/StringUtils.h"
#include "Models.h"

#include <map>

namespace dspacex {

class MSModelSet;
using ModelMap = std::map<std::string, std::vector<std::shared_ptr<MSModelSet>>>;

/// When a Morse-Smale complex is used to decompose/partition a dataset, the samples associated
/// with each crystal at each persistence level can be used to learn a probabilistic
/// interpolation model (e.g., ShapeOdds, PCA, SharedGP). These classes provide access to such
/// sets of models.

/// Crystal contains the models constructed from this set of samples
class MSCrystal
{
public:
  void addSample(unsigned n) { model->addSample(n); }          // TODO: fail gracefully if model doesn't exist
  unsigned numSamples() const { return model->numSamples(); }

  const std::vector<Model::ValueIndexPair>& getSampleIndices() const
  {
    return model->getSampleIndices();
  }

  std::shared_ptr<Model> getModel() { return model; }
  
private:
  std::shared_ptr<Model> model;
};


/// Persistence contains the set of Crystals and their global embeddings (common 2d latent
/// space for all crystals at that level)
class MSPersistenceLevel
{
public:
  void setNumCrystals(unsigned nCrystals) { crystals.resize(nCrystals); }
  unsigned numCrystals() const { return crystals.size(); }
  MSCrystal& getCrystal(unsigned i) { return crystals[i]; }
  
  void setGlobalEmbeddings(const Eigen::MatrixXd &embeddings) { global_embeddings = embeddings; }

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


// Associates a model with it's name (pXXcYY) // todo: this is an issue because there could be more than one model with the same name (in another MSModelSet)
typedef std::pair<std::string, std::shared_ptr<Model>> ModelPair;

/// MSModelSet is a model container that stores one set of models for a given field and inclues
/// the parameters used to compute the M-S using NNMSComplex, as well as sub-containers of
/// persistince levels and their crystals, which ultimately store the models of this
/// complex.
class MSModelSet
{
public:
  MSModelSet(Model::Type mtype, const std::string& mname, const std::string& field, unsigned nSamples, unsigned nPersistences)
    : modeltype(mtype), modelname(mname), fieldname(field), num_samples(nSamples), persistence_levels(nPersistences)
  {}

  auto modelType() const { return modeltype; }
  auto modelName() const { return modelname; }
  auto getFieldname() const { return fieldname; }
  auto numSamples() const { return num_samples; }
  auto numPersistenceLevels() const { return persistence_levels.size(); }
  
  MSPersistenceLevel& getPersistenceLevel(unsigned idx) { return persistence_levels[idx]; }

  /// indicates the existence of a model at the specified crystal of the specified persistence level
  bool hasModel(unsigned p, unsigned c) const;

  /// returns the model at the specified crystal of the specified persistence level
  ModelPair getModel(unsigned p, unsigned c);

  /// returns all models
  std::vector<ModelPair> getAllModels();

  // TODO: add parameters used by NNMSComplex to create this so the complex can be predictively recomputed
  //       this is especially important for the new preprocessed data pipeline
  //void addParams(...); //e.g., fieldname, persistenceLevel, knn, sigma, smooth, noise, numPersistences, numSamples, normalize
  //Params getParams() const;
  // TODO: store persistence range for this complex since not all levels have to be computed

private:
  static std::string modelName(unsigned p, unsigned c, unsigned persistence_padding = 2, unsigned crystals_padding = 2)
  {
    return std::string("p"+paddedIndexString(p, persistence_padding)+"c"+paddedIndexString(c, crystals_padding));
  }

  Model::Type modeltype;            // type of models in this complex (pca, shapeodds, sharedgp, etc)
  std::string modelname;            // name of models in this complex
  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
        // TODO: add the other parameters used by NNMSComplex to create this so the complex can be recomputed
  std::vector<MSPersistenceLevel> persistence_levels;
};



} // dspacex

