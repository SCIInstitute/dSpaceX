/// When a Morse-Smale complex is used to decompose/partition a dataset, the samples associated
/// with each crystal at each persistence level can be used to learn a probabilistic
/// interpolation model (e.g., ShapeOdds, PCA, SharedGP). These classes provide access to such
/// sets of models.

#pragma once

#include "dataset/Precision.h"
#include "dataset/ValueIndexPair.h"
#include "utils/StringUtils.h"
#include "Model.h"

#include <map>
#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

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
    void addSampleIndex(unsigned n) { samples.push_back(ValueIndexPair{n}); }
    unsigned numSamples() const { samples.size(); }
    const std::vector<ValueIndexPair>& getSamples() const { return samples; }
    void setModelPath(const std::string& path) { modelPath = path; }
  
  private:
    std::string modelPath;
    std::vector<ValueIndexPair> samples;
    std::shared_ptr<Model> model;
    std::unique_ptr<std::vector<Precision>> fieldvals;  // cache fieldvals needed to evaluate model each time

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
  MSModelset(Model::Type mtype, const std::string& field, unsigned nSamples, unsigned nPersistences,
             bool rotateResult = false)
    : modeltype(mtype), modelname(Model::typeToStr(mtype)), fieldname(field),
      num_samples(nSamples), fieldvals(nSamples), _rotate(rotateResult)
  { persistence_levels.resize(nPersistences); }

  auto modelType() const { return modeltype; }
  auto modelName() const { return modelname; }
  void setModelName(const std::string& name) { modelname = name; }
  auto rotate() const { return _rotate; }
  auto fieldName() const { return fieldname; }
  auto numSamples() const { return num_samples; }
  auto numPersistenceLevels() const { return persistence_levels.size(); }
  void setFieldvals(Eigen::VectorXf values) { fieldvals = values; }
  void setCustomEvaluator(std::vector<std::string> evaluator) { custom_evaluator = evaluator; }
  void setCustomRenderer(std::vector<std::string> renderer) { custom_renderer = renderer; }
  auto hasCustomRenderer() const { return !custom_renderer.empty(); }
  auto hasCustomEvaluator() const { return !custom_evaluator.empty(); }
  
  PersistenceLevel& getPersistenceLevel(unsigned idx) { return persistence_levels[idx]; }

  /// indicates the existence of a model at the specified crystal of the specified persistence level
  bool hasModel(int persistence, int crystal) const;

  /// returns the model at the specified crystal of the specified persistence level
  std::shared_ptr<Model> getModel(int persistence, int crystal);

  /// returns set of fieldvals associated with a crystal
  const std::vector<Precision>& getCrystalFieldvals(int persistence, int crystalid);

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
  
  /// returns custom Python renderer, instantiating it if necessary
  py::object& getCustomRenderer();
  
  /// returns custom Python evaluator, instantiating it if necessary
  py::object& getCustomEvaluator();

private:
  MSModelset();

  Model::Type modeltype;            // type of models in this complex (pca, shapeodds, sharedgp, etc)
  std::string modelname;            // name of models in this complex
  std::string fieldname;            // name of field for which this M-S complex was computed
  unsigned num_samples;             // how many samples were used to compute this M-S
  bool _rotate{false};              // whether model's results need to be rotated 90 degrees clockwise (e.g., old ShapeOdds models)
  Eigen::VectorXf fieldvals;        // the fieldvals of the samples for this field
  MSParams params;
  std::vector<PersistenceLevel> persistence_levels;

  // Custom Python modules for evaluation and renderering (if provided)
  std::vector<std::string> custom_evaluator;  // name, module, args
  std::vector<std::string> custom_renderer;
  py::object python_evaluator;      // custom model evaluation class instance
  py::object python_renderer;       // custom thumbnail renderer class instance

  /// creates an instance of the custom Python renderer
  void initializeCustomRenderer();

  /// creates an instance of the custom Python evaluator (TODO: use manual_test example)
  void initializeCustomEvaluator();

  /// returns requested python module, loading it if necessary
  static py::object& getPythonModule(const std::string& name); 
  static std::map<std::string, py::object> python_modules;
};

} // dspacex

