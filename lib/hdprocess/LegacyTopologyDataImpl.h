#pragma once

#include "HDVizData.h"
#include "TopologyData.h"


/**
 *
 */
class LegacyTopologyDataImpl : public TopologyData {
 public:
  LegacyTopologyDataImpl(std::shared_ptr<HDVizData> data);
  unsigned int getMinPersistenceLevel() const override;
  unsigned int getMaxPersistenceLevel() const override;
  std::shared_ptr<MorseSmaleComplex> getComplex(unsigned int persistenceLevel) override;
  nlohmann::json& asJson() const override; 

 private:
  std::shared_ptr<HDVizData> m_data;
  std::vector<std::shared_ptr<MorseSmaleComplex>> m_morseSmaleComplexes;
};


/**
 *
 */
class LegacyMorseSmaleComplexImpl : public MorseSmaleComplex {
 public:
  LegacyMorseSmaleComplexImpl(std::vector<std::shared_ptr<Crystal>> &crystals) {
    m_crystals = crystals;
  }
  virtual std::vector<std::shared_ptr<Crystal>>& getCrystals() { return m_crystals;  }
  virtual std::vector<std::pair<unsigned int, unsigned int>> getAdjacency() {
    return std::vector<std::pair<unsigned int, unsigned int>>();
  } 
  
 private:
  std::vector<std::shared_ptr<Crystal>> m_crystals;
};


/**
 *
 */
class LegacyCrystalImpl : public Crystal{
 public:
  LegacyCrystalImpl(unsigned int minIndex, unsigned int maxIndex, std::vector<unsigned int> samples) : 
      m_minIndex(minIndex), m_maxIndex(maxIndex), m_samples(samples) {}
  virtual unsigned int getMaxSample() { return m_minIndex; }
  virtual unsigned int getMinSample() { return m_maxIndex; }  
  virtual std::vector<unsigned int>& getAllSamples() { return m_samples; }
 private:
  unsigned int m_minIndex;
  unsigned int m_maxIndex;
  std::vector<unsigned int> m_samples;
};
