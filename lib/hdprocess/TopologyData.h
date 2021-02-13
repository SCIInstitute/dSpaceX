
#pragma once

#include "HDVizData.h"
#include <nlohmann/json.hpp>

class Crystal {
 public:
  virtual unsigned int getMaxSample() = 0;
  virtual unsigned int getMinSample() = 0;
  virtual std::vector<dspacex::ValueIndexPair>& getAllSamples() = 0; 
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
  virtual unsigned int getMinPersistenceLevel() const = 0;
  virtual unsigned int getMaxPersistenceLevel() const = 0;
  virtual std::shared_ptr<MorseSmaleComplex> getComplex(unsigned int persistenceLevel) = 0;
  virtual nlohmann::json& asJson() const = 0;
};


