#include "Dataset.h"
#include "utils.h"

#include <algorithm>

using namespace dspacex;

bool Dataset::valid() const
{
  // TODO:  Add validation that sample counts match array sizes.
  // m_dataset->m_hasSamplesMatrix ==> m_dataset->m_samplesMatrix.N()
  // _dataset->m_hasDistanceMatrix ==> m_dataset->m_distanceMatrix.N();
  return true;
}

const Image& Dataset::getThumbnail(int idx) const
{
  if (idx < 0 || idx >= m_thumbnails.size())
    throw std::runtime_error("Tried to fetch thumbnail " + std::to_string(idx) + ", but there are only " + std::to_string(m_thumbnails.size()));
  return m_thumbnails[idx];
}

std::shared_ptr<MSModelset> Dataset::getModelset(const std::string& fieldname, const std::string& modelname) {
  if (m_models.find(fieldname) != m_models.end()) {
    for (auto modelset : m_models[fieldname])
      if (modelset->modelName() == modelname)
        return modelset;
  }
  return nullptr;
}

std::vector<std::string> Dataset::getModelNames(const std::string& fieldname) {
  std::vector<std::string> names{"None"};
  if (m_models.find(fieldname) != m_models.end()) {
    for (auto modelset : m_models[fieldname]) {
      names.push_back(modelset->modelName());
    }
  }

  return names;
}

/*
 * getFieldvalues
 * returns Eigen::Map wrapping the vector of values for a given field
 */
Eigen::Map<Eigen::VectorXf> Dataset::getFieldvalues(const std::string &name, Fieldtype type, bool normalized)
{
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


///////////////////////////////////////////////////////////////////////////////
// Dataset::Builder

std::unique_ptr<Dataset> Dataset::Builder::build()
{
  if (!m_dataset || !m_dataset->valid())
    throw std::runtime_error("Dataset in Dataset::Builder is not valid.");

  return std::move(m_dataset); // std::move releases ownership of m_dataset
}

Dataset::Builder& Dataset::Builder::withSampleCount(int count) {
  m_dataset->m_sampleCount = count;
  return (*this);
}

// TODO:  change to withGeometry of templatized form.
Dataset::Builder& Dataset::Builder::withSamplesMatrix(FortranLinalg::DenseMatrix<Precision> &samplesMatrix) {
  m_dataset->m_samplesMatrix = samplesMatrix;
  m_dataset->m_hasSamplesMatrix = true;
  return (*this);
}

Dataset::Builder& Dataset::Builder::withDistanceMatrix(FortranLinalg::DenseMatrix<Precision> &distanceMatrix) {
  m_dataset->m_distanceMatrix = distanceMatrix;
  m_dataset->m_hasDistanceMatrix = true;
  return (*this);
}

Dataset::Builder& Dataset::Builder::withParameter(std::string name, FortranLinalg::DenseVector<Precision> &parameter) {
  m_dataset->m_parameterNames.push_back(name);
  m_dataset->m_parameters.push_back(parameter);
  m_dataset->m_normalized_parameters.push_back(normalize(parameter));
  return (*this);
}

Dataset::Builder& Dataset::Builder::withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi) {
  m_dataset->m_qoiNames.push_back(name);
  m_dataset->m_qois.push_back(qoi);
  m_dataset->m_normalized_qois.push_back(normalize(qoi));
  return (*this);
}

Dataset::Builder& Dataset::Builder::withEmbedding(std::string name, FortranLinalg::DenseMatrix<Precision> &embedding) {
  m_dataset->m_embeddingNames.push_back(name);
  m_dataset->m_embeddings.push_back(embedding);
  return (*this);
}

Dataset::Builder& Dataset::Builder::withModel(std::string fieldname, std::shared_ptr<MSModelset> modelset) {
  m_dataset->m_msModelFields.push_back(fieldname);
  modelset->setFieldvals(m_dataset->getFieldvalues(fieldname, Fieldtype::Unknown, false /*normalized*/));
  m_dataset->m_models[modelset->fieldName()].push_back(std::move(modelset));
  return (*this);
}

Dataset::Builder& Dataset::Builder::withName(std::string name) {
  m_dataset->m_name = name;
  return (*this);
}

Dataset::Builder& Dataset::Builder::withThumbnails(std::vector<Image> thumbnails) {
  m_dataset->m_thumbnails = thumbnails;
  return (*this);
}

