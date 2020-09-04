/// When a Morse-Smale complex is used to decompose/partition a dataset, the samples associated
/// with each crystal at each persistence level can be used to learn a probabilistic
/// interpolation model (e.g., ShapeOdds, PCA, SharedGP). These classes provide access to such
/// sets of models.

#pragma once

#include "dspacex/Precision.h"
#include "utils/StringUtils.h"
#include "Models.h"

#include <map>

namespace dspacex {

class MSModelset;

/// All model sets for a given field
using ModelMap = std::map<std::string, std::vector<std::shared_ptr<MSModelset>>>;

/*
 * MSModelset is a model container that stores the set of models computed for a given field
 * using the samples associated with each crystal of a M-S complex. Inclues the parameters
 * used to compute the M-S using NNMSComplex.
 */
class MSModelset
{
  /*
   * Crystal contains the models constructed from this set of samples
   */
  struct Crystal
  {
    void addSampleIndex(unsigned n) { sample_indices.push_back(n); }
    unsigned numSamples() const { sample_indices.size(); }
    const std::vector<unsigned>& getSampleIndices() const { return sample_indices; }
    void setModelPath(const std::string& path) { modelPath = path; }
  
  private:
    std::string modelPath;
    std::vector<unsigned> sample_indices;
    std::shared_ptr<Model> model;
    std::unique_ptr<std::vector<float>> samples;  // cache samples since they're needed to evaluate model each time

    friend class MSModelset;
  };


  /*
   * Persistence contains the set of Crystals and their global embeddings (common 2d latent
   * space for all crystals at that level)
   */
  struct PersistenceLevel
  {
    void setNumCrystals(unsigned nCrystals) { crystals.resize(nCrystals); }

    // each crystal is composed of a non-intersecting set of samples, read from this vector
    void setCrystalSampleIds(Eigen::RowVectorXi crystal_ids) {
      // WARNING: all crystals must have been added or these crystal ids will be out of range      
      for (unsigned n = 0; n < crystal_ids.cols(); n++) { crystals[crystal_ids(n)].addSampleIndex(n); }
    }

    std::vector<Crystal> crystals;
  };


public:
  MSModelset(Model::Type mtype, const std::string& field, unsigned nSamples, unsigned nPersistences, bool rotate = false)
    : modeltype(mtype), modelname(Model::typeToStr(mtype)), fieldname(field),
      num_samples(nSamples), samples(nSamples), _rotate(rotate)
  { persistence_levels.resize(nPersistences); }

  auto modelType() const { return modeltype; }
  auto modelName() const { return modelname; }
  void setModelName(const std::string& name) { modelname = name; }
  auto rotate() const { return _rotate; }
  auto fieldName() const { return fieldname; }
  auto numSamples() const { return num_samples; }
  auto numPersistenceLevels() const { return persistence_levels.size(); }
  void setSamples(Eigen::VectorXf values) { samples = values; }
  
  PersistenceLevel& getPersistenceLevel(unsigned idx) { return persistence_levels[idx]; }

  /// indicates the existence of a model at the specified crystal of the specified persistence level
  bool hasModel(int persistence, int crystal) const;

  /// returns the model at the specified crystal of the specified persistence level
  std::shared_ptr<Model> getModel(int persistence, int crystal);

  /// returns set of samples associated with a crystal
  const std::vector<float>& getCrystalSamples(int persistence, int crystalid);

  /// returns all models (*unused, and costly since it will read every model in this set)
  std::vector<std::shared_ptr<Model>> getAllModels();

  /// parameters used by NNMSComplex to create this (so the complex can be recomputed)
  struct MSParams {
    int knn;
    double sigma;
    double smooth;
    int curvepoints;
    int depth;
    bool noise;
    bool normalize;
  };
  MSParams getParams() const { return params; }
  void setParams(int knn, double sigma, double smooth, int curvepoints, int depth, bool noise, bool normalize) {
    params.knn = knn;
    params.sigma = sigma;
    params.smooth = smooth;
    params.curvepoints = curvepoints;
    params.depth = depth;
    params.noise = noise;
    params.normalize = normalize;
  }

private:
  MSModelset();

  Model::Type modeltype;            // type of models in this complex (pca, shapeodds, sharedgp, etc)
  std::string modelname;            // name of models in this complex
  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S (redundant if we have the samples themselves)
  bool _rotate{false};               // whether the models results need to be rotated 90 degrees clockwise (e.g., old ShapeOdds models)
  Eigen::VectorXf samples;          // the samples of the dataset for this field (a copy of... fixme)
  MSParams params;
  std::vector<PersistenceLevel> persistence_levels;
};



} // dspacex

