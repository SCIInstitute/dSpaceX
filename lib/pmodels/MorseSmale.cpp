#include "MorseSmale.h"

namespace dspacex {

bool MSComplex::hasModel(unsigned persistence, unsigned crystal) const
{
  return !(persistence >= persistence_levels.size() ||
           crystal >= persistence_levels[persistence].numCrystals());
}

ModelPair MSComplex::getModel(unsigned persistence, unsigned crystal)
{
  if (!hasModel(persistence, crystal))
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  return ModelPair(modelName(persistence, crystal),
                   persistence_levels[persistence].getCrystal(crystal).getModel());
}

std::vector<ModelPair> MSComplex::getAllModels()
{
  unsigned persistence_padding = paddedStringWidth(persistence_levels.size());

  std::vector<ModelPair> models;  
  for (unsigned p = 0; p < persistence_levels.size(); p++)
  {
    unsigned crystals_padding = persistence_levels[p].numCrystals();
    for (unsigned c = 0; c < persistence_levels[p].numCrystals(); c++)
    {
      models.push_back(ModelPair(modelName(p,c,persistence_padding,crystals_padding),
                                 persistence_levels[p].getCrystal(c).getModel()));
    }
  }
  return models;
}

} // dspacex
