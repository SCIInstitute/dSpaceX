#include "Modelset.h"
#include "DatasetLoader.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
using Clock = std::chrono::steady_clock;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::literals::chrono_literals;

namespace dspacex {

bool MSModelset::hasModel(int p, int c) const
{
  return p < persistence_levels.size() && p >= 0 && c < persistence_levels[p].crystals.size() && c >= 0;
}

/* 
 * Returns Model of crystal c in persistence level p, creating/reading it if necessary.
 *
 * TODO: Use persistence range for this MSModelset's M-S complex to allow the global
 *       persistence to be passed to this function rather than make the caller precalculate
 *       this since it forces them to know too much about the modelset.
 *
 *       i.e., when only a subset of N of the P total M-S persistences is
 *       used, desired p is in range [0, P-1], and pidx = p - N.
 */
std::shared_ptr<Model> MSModelset::getModel(int p, int c)
{
  if (!hasModel(p, c))
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  std::shared_ptr<Model>& model(persistence_levels[p].crystals[c].model);
  if (!model) {
    DatasetLoader::parseModel(persistence_levels[p].crystals[c].modelPath,
                              *(model = Model::create(modeltype, modelname)),
                              persistence_levels[p].crystals[c].samples);

    // set fieldvalue bounds of model
    std::vector<float> bounds(getCrystalFieldvals(p, c));
    auto minmax = std::minmax_element(bounds.begin(), bounds.end());
    model->setBounds(std::pair<float, float>(*minmax.first, *minmax.second));
  }
  return model;
}

/* 
 * returns all models, creating/reading it if necessary (warning: expensive)
 */
std::vector<std::shared_ptr<Model>> MSModelset::getAllModels()
{
  std::vector<std::shared_ptr<Model>> models;
  for (auto p = 0; p < persistence_levels.size(); p++)
    for (auto c = 0; c < persistence_levels[p].crystals.size(); c++)
      models.push_back(getModel(p, c));

  return models;
}

/* 
 * returns fieldvals for the set of samples associated with this crystal
 */
const std::vector<Precision>& MSModelset::getCrystalFieldvals(int p, int c) {
  auto& crystal(persistence_levels[p].crystals[c]);
  if (!crystal.fieldvals) {
    crystal.fieldvals = std::make_unique<std::vector<Precision>>();

    // set fieldvalue for each sample
    unsigned i{0};
    for (auto& sample : crystal.samples) {
      sample.val = fieldvals[sample.idx];
      sample.local_idx = i++;
    }

    // sort samples by increasing fieldvalue
    std::sort(crystal.samples.begin(), crystal.samples.end(), ValueIndexPair::compare);
    
    // create cache of fieldvals (used for generating z_coords)
    for (auto sample : crystal.samples) {
      crystal.fieldvals->push_back(sample.val);
    }
  }
  return *crystal.fieldvals;
}

/* 
 * returns sigma for evaluating the model associated with this crystal
 */
Precision MSModelset::getCrystalSigma(int p, int c) {
  auto& crystal(persistence_levels[p].crystals[c]);
  if (crystal.sigma == -1) {
    crystal.sigma = dspacex::computeSigma(getCrystalFieldvals(p, c));
  }

  return crystal.sigma;
}

py::object& MSModelset::getCustomRenderer() {

  if (!python_renderer) {
    initializeCustomRenderer();
  }

  return python_renderer;
}

/*
 * Initialize custom Python shape renderer for this model.
 * MUST be called from main thread if renderer uses OpenGL (ex: vtk, pyvista, pyrender, itk)
 */
void MSModelset::initializeCustomRenderer() {
  using namespace pybind11::literals;

  // get the Python module containing the renderer
  auto modname(custom_renderer[0]);
  auto module = MSModelset::getPythonModule(modname);

  time_point<Clock> start = Clock::now();

  // instantiate renderer
  std::string default_file = custom_renderer.size() > 2 ? custom_renderer[2] : "";
  float scale = custom_renderer.size() > 3 ? std::stof(custom_renderer[3]) : 1.25;
  python_renderer = module.attr(custom_renderer[1].c_str())("default"_a = default_file, "scale"_a = scale);

  std::cout << "Custom renderer created in " << duration_cast<milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
}

/*
 * Retrieves the requested Python module.
 */
py::object& MSModelset::getPythonModule(const std::string& modname) {

  if (MSModelset::python_modules.find(modname) == MSModelset::python_modules.end()) {
    time_point<Clock> start = Clock::now();

    // import the requested module
    try {
      MSModelset::python_modules[modname] = py::module::import(modname.c_str());
    } catch(std::exception e) {
      std::cerr << "Could not find custom Python thumbnail renderer. Set `--scriptspath` on server start.";
    }
    std::cout << modname <<" loaded in " << duration_cast<milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
  }

  return MSModelset::python_modules.at(modname);
}
std::map<std::string, py::object> MSModelset::python_modules;

} // dspacex
