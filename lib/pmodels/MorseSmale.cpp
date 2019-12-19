#include "MorseSmale.h"

namespace PModels {

ModelPair MSModelContainer::getModel(unsigned p, unsigned c)
{
  unsigned persistence_idx = p - 14; // <ctc> hack since persistence levels are numbered 0-19 in shapeodds output for CantileverBeam
  if (persistence_idx >= persistence_levels.size() || c >= persistence_levels[persistence_idx].numCrystals())
    throw std::runtime_error("Requested model persistence / crystal index is out of range");
      
  return ModelPair(modelName(p, c), persistence_levels[persistence_idx].getCrystal(c).getModel());
}

std::vector<ModelPair> MSModelContainer::getAllModels()
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

} // end namespace PModels
