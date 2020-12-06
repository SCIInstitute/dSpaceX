#include "Dataset.h"
#include "utils.h"

#include <algorithm>

namespace dspacex {

const Image& Dataset::getThumbnail(int idx) const
{
  if (idx < 0 || idx >= m_thumbnails.size())
    throw std::runtime_error("Tried to fetch thumbnail " + std::to_string(idx) + ", but there are only " + std::to_string(m_thumbnails.size()));
  return m_thumbnails[idx];
}

std::shared_ptr<MSModelset> Dataset::getModelset(std::string metric, const std::string& fieldname, const std::string& modelname) {
  if (hasModelsAtDistanceMetric(metric)) {
    if (m_models.at(metric).find(fieldname) != m_models.at(metric).end()) {
      for (auto modelset : m_models.at(metric).at(fieldname)) {
        if (modelset->modelName() == modelname) {
          return modelset;
        }
      }
    }
  }
  return nullptr;
}

std::vector<std::string> Dataset::getModelNames(std::string metric, const std::string& fieldname) const {
  std::vector<std::string> names{"None"};
  if (hasModelsAtDistanceMetric(metric)) {
    if (m_models.at(metric).find(fieldname) != m_models.at(metric).end()) {
      for (auto modelset : m_models.at(metric).at(fieldname)) {
        names.push_back(modelset->modelName());
      }
    }
  }
  return std::move(names);
}

/*
 * getFieldvalues
 * returns Eigen::Map wrapping the vector of values for a given field
 */
Eigen::Map<Eigen::VectorXf> Dataset::getFieldvalues(const std::string &name, Fieldtype type, bool normalized) {
  // try to find the index of the data in both parameters and qois since it'll be needed anyway
  auto pnames = getParameterNames();
  auto ploc = std::find(std::begin(pnames), std::end(pnames), name);

  auto qnames = getQoiNames();
  auto qloc = std::find(std::begin(qnames), std::end(qnames), name);
  
  // if passed type is Unknown, see if indices were found and choose a type (QoI has priority)
  if (type == Fieldtype::Unknown) {
    if (ploc != std::end(pnames))
      type = Fieldtype::DesignParameter;
    if (qloc != std::end(qnames))
      type = Fieldtype::QoI;
  }
  
  // wrap the data in a map and return it
  switch(type) {
    case Fieldtype::DesignParameter:
      if (ploc != std::end(pnames)) {
        int index = std::distance(pnames.begin(), ploc);
        FortranLinalg::DenseVector<Precision> values = getParameterVector(index, normalized);
        return Eigen::Map<Eigen::VectorXf>(values.data(), values.N());
      }      
      break;
    case Fieldtype::QoI:
      if (qloc != std::end(qnames)) {
        int index = std::distance(qnames.begin(), qloc);
        FortranLinalg::DenseVector<Precision> values = getQoiVector(index, normalized);
        return Eigen::Map<Eigen::VectorXf>(values.data(), values.N());
      }
  }

  return Eigen::Map<Eigen::VectorXf>(NULL, 0);
}

} // dspacex
