#include "LegacyTopologyDataImpl.h"
#include <stdexcept>


LegacyTopologyDataImpl::LegacyTopologyDataImpl(std::shared_ptr<HDVizData> data) : m_data(data) {
  // Construct DSpaceXData Object Hierarchy

  for (unsigned int level = getMinPersistenceLevel(); level <= getMaxPersistenceLevel(); level++) {
    std::vector<std::shared_ptr<Crystal>> lcrystals;
    auto crystals = data->getAllCrystals()[level];

    for (auto crystalIndex = 0; crystalIndex < crystals.size(); crystalIndex++) {      
      auto extrema = data->getAllExtrema()[level][crystalIndex];
      std::shared_ptr<Crystal> crystal(new LegacyCrystalImpl(extrema.second, extrema.first, std::vector<dspacex::ValueIndexPair>(crystals[crystalIndex])));
      lcrystals.push_back(crystal);
    }

    std::shared_ptr<MorseSmaleComplex> complex(new LegacyMorseSmaleComplexImpl(lcrystals));
    m_morseSmaleComplexes.push_back(complex);
  }
}

unsigned int LegacyTopologyDataImpl::getMinPersistenceLevel() const {
  return m_data->getMinPersistenceLevel();
}

unsigned int LegacyTopologyDataImpl::getMaxPersistenceLevel() const {
  return m_data->getMaxPersistenceLevel();
}

std::shared_ptr<MorseSmaleComplex> LegacyTopologyDataImpl::getComplex(unsigned int persistenceLevel) {
  int index = persistenceLevel - getMinPersistenceLevel();
  // unsigned int index = persistenceLevel;
  if (index < 0 || index >= m_morseSmaleComplexes.size()) {
    throw std::out_of_range(std::string("Invalid Persistence Level") + std::to_string(persistenceLevel));
  }
  return m_morseSmaleComplexes[index];
}

nlohmann::json& LegacyTopologyDataImpl::asJson() const {
  using json = nlohmann::json;

  json ms;
//  for (auto i = getMinPersistenceLevel(); i <= getMaxPersistenceLevel(); i++) {
//    ms["crystal_partitions"][std::string("p") + std::to_string(i)] = std::vector<int>(m_data->getCrystalPartitions(i).data, m_data->getCrystalPartitions(i).data + m_data->getCrystalPartitions(i).N());
//  }

  // TODO: add these to ms and return it. DataExport::exportCrystalPartitions can write it to a file
  //   - add m_data->crystalPartitions to an arr in ms
  //   - original data range so it can be used for later analyses
  //   - parameters used for computation: knn, sigma, smooth, noise, num_levels (depth)
  //   - num_levels needs to know how many levels there would be in total so they can be properly numbered
}
