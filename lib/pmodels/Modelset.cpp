#include "Modelset.h"
#include "DatasetLoader.h"

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
 * returns model crystal of persistence level, creating/reading it if necessary
 * TODO: use persistence range for this complex to facilitae global persistence passed to getModel function
 *       i.e., when only a subset of N of the P total M-S persistences is used, desired p is in range [0,P-1], and pidx = p - N.
 *             Don't make the called precalculate this since it forces them to know too much about the modelset.
 */
std::shared_ptr<Model> MSModelset::getModel(int p, int c)
{
  if (!hasModel(p, c))
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  std::shared_ptr<Model>& model(persistence_levels[p].crystals[c].model);
  if (!model) {
    DatasetLoader::parseModel(persistence_levels[p].crystals[c].modelPath,
                              *(model = Model::create(modeltype, modelname)),
                              persistence_levels[p].crystals[c].sample_indices);

    // set fieldvalue bounds of model
    std::vector<float> bounds(getCrystalSamples(p, c));
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
 * returns set of samples associated with this crystal
 */
const std::vector<float>& MSModelset::getCrystalSamples(int p, int c) {
  auto& crystal(persistence_levels[p].crystals[c]);
  if (!crystal.samples) {
    crystal.samples = std::make_unique<std::vector<float>>();
    for (auto idx : crystal.sample_indices)
      crystal.samples->push_back(samples[idx]);
  }
  return *crystal.samples;
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
  python_renderer = module.attr(custom_renderer[1].c_str())("default"_a = default_file, "scale"_a = 1.0);

  std::cout << "Custom renderer created in " << duration_cast<milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
}

/*
 * Retrieves the requested Python module.
 */
py::object& MSModelset::getPythonModule(const std::string& modname) {

  if (MSModelset::python_modules.find(modname) == MSModelset::python_modules.end()) {
    time_point<Clock> start = Clock::now();

    // import the requested module
    MSModelset::python_modules[modname] = py::module::import(modname.c_str());

    std::cout << modname <<" loaded in " << duration_cast<milliseconds>(Clock::now() - start).count() << " ms" << std::endl;
  }

  return MSModelset::python_modules[modname];
}
std::map<std::string, py::object> MSModelset::python_modules;

} // dspacex
