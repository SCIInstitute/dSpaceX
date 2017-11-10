#include "LegacyTopologyDataImpl.h"
#include <stdexcept>


LegacyTopologyDataImpl::LegacyTopologyDataImpl(HDVizData *data) : m_data(data) {
  // Construct DSpaceXData Object Hierarchy

  // TODO: Convert to Iterator Range Loop
  for (unsigned int level = getMinPersistenceLevel(); level <= getMaxPersistenceLevel(); level++) {

    std::vector<Crystal*> crystals;
    for (unsigned int crystalIndex = 0; crystalIndex < m_data->getCrystals(level).N(); crystalIndex++) {

      unsigned int minIndex = m_data->getCrystals(level)(1, crystalIndex);
      unsigned int maxIndex = m_data->getCrystals(level)(0, crystalIndex);      
      std::vector<unsigned int> samples;      

      auto crystalPartitions = data->getCrystalPartitions(level);
      for (unsigned int s = 0; s < crystalPartitions.N(); s++) {
        if (crystalPartitions(s) == crystalIndex) {
          samples.push_back(s);
        }
      }

      Crystal *crystal = new LegacyCrystalImpl(minIndex, maxIndex, samples); 
      crystals.push_back(crystal);
    }

    MorseSmaleComplex *complex = new LegacyMorseSmaleComplexImpl(crystals);
    m_morseSmaleComplexes.push_back(complex);
  }
}

unsigned int LegacyTopologyDataImpl::getMinPersistenceLevel() {
  return m_data->getMinPersistenceLevel();
}

unsigned int LegacyTopologyDataImpl::getMaxPersistenceLevel() {
  return m_data->getMaxPersistenceLevel();
}

MorseSmaleComplex* LegacyTopologyDataImpl::getComplex(unsigned int persistenceLevel) {
  // int index = persistenceLevel - getMinPersistenceLevel();
  unsigned int index = persistenceLevel;
  if (index < 0 || index >= m_morseSmaleComplexes.size()) {
    throw std::out_of_range(std::string("Invalid Persistence Level") + std::to_string(persistenceLevel));
  }
  return m_morseSmaleComplexes[index];
}
