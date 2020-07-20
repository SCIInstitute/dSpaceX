#include "MorseSmale.h"
#include "DatasetLoader.h"

namespace dspacex {

bool MSModelSet::hasModel(unsigned persistence, unsigned crystal) const
{
  return !(persistence >= persistence_levels.size() ||
           crystal >= persistence_levels[persistence].numCrystals());
}

ModelPair MSModelSet::getModel(unsigned persistence, unsigned crystal)
{
  if (!hasModel(persistence, crystal))
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  return ModelPair(modelName(persistence, crystal),
                   persistence_levels[persistence].getCrystal(crystal).model);
}

std::vector<ModelPair> MSModelSet::getAllModels()
{
  unsigned persistence_padding = paddedStringWidth(persistence_levels.size());

  std::vector<ModelPair> models;  
  for (unsigned p = 0; p < persistence_levels.size(); p++)
  {
    unsigned crystals_padding = persistence_levels[p].numCrystals();
    for (unsigned c = 0; c < persistence_levels[p].numCrystals(); c++)
    {
      models.push_back(ModelPair(modelName(p,c,persistence_padding,crystals_padding),
                                 persistence_levels[p].getCrystal(c).model));
    }
  }
  return models;
}

} // dspacex
