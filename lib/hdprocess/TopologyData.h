
#pragma once

#include "HDVizData.h"

class Crystal {
 public:
  virtual unsigned int getMaxSample() = 0;
  virtual unsigned int getMinSample() = 0;
  virtual std::vector<unsigned int>& getAllSamples() = 0; 
  // virtual std::vector<SampleType> getReconstruction() = 0;
};

class MorseSmaleComplex {
 public:  
  // TODO replace with Iterator and getCrystalCount() method.
  virtual std::vector<std::shared_ptr<Crystal>>& getCrystals() = 0;
  virtual std::vector<std::pair<unsigned int, unsigned int>> getAdjacency() = 0;
};


class TopologyData {
 public:
  virtual unsigned int getMinPersistenceLevel() = 0;
  virtual unsigned int getMaxPersistenceLevel() = 0;
  virtual std::shared_ptr<MorseSmaleComplex> getComplex(unsigned int persistenceLevel) = 0;
};


