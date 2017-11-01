
#pragma once

#include "HDVizData.h"

class Crystal {
 public:
  virtual unsigned int getMaxSample() = 0;
  virtual unsigned int getMinSample() = 0;
  virtual std::vector<unsigned int> getAllSamples() = 0; 
};

class MorseSmaleComplex {
 public:  
  // TODO replace with Iterator and getCrystalCount() method.
  virtual std::vector<Crystal*>& getCrystals() = 0;
  virtual std::vector<std::pair<unsigned int, unsigned int>> getAdjacency() = 0;
};


class TopologyData {
 public:
  virtual unsigned int getMinPersistenceLevel() = 0;
  virtual unsigned int getMaxPersistenceLevel() = 0;
  virtual MorseSmaleComplex* getComplex(unsigned int persistenceLevel) = 0;
};


