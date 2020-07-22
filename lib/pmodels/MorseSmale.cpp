#include "MorseSmale.h"
#include "DatasetLoader.h"

#include <algorithm>

namespace dspacex {

bool MSModelset::hasModel(int p, int c) const
{
  return p < persistence_levels.size() && p >= 0 && c < persistence_levels[p].crystals.size() && c >= 0;
}

/* 
 * returns model crystal of persistence level, creating/reading it if necessary
 * TODO: use persistence range for this complex to facilitae global persistence passed to getModel function
 */
std::shared_ptr<Model> MSModelset::getModel(int p, int c)
{
  if (!hasModel(p, c))
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  std::shared_ptr<Model>& model(persistence_levels[p].crystals[c].model);
  if (!model) {
    DatasetLoader::parseModel(persistence_levels[p].crystals[c].modelPath,
                              *(model = std::make_shared<Model>(modeltype)),
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


} // dspacex
