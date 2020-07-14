#include "Dataset.h"

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

std::shared_ptr<Model> Dataset::getModel(const std::string& fieldname, const std::string& modelname, int p, int c)
{
  for (auto modelset : m_models[fieldname])
    if (modelset->modelName() == modelname)
      if (p < modelset->numPersistenceLevels() && p >= 0)
        if (c < modelset->getPersistenceLevel(p).numCrystals() && c >= 0)
          return modelset->getPersistenceLevel(p).getCrystal(c).getModel();

  return nullptr;
}

bool Dataset::hasModel(const std::string& fieldname, const std::string& modelname, int p, int c) const
{
  return const_cast<Dataset*>(this)->getModel(fieldname, modelname, p, c) ? true : false;
}

std::shared_ptr<MSModelSet> Dataset::getModelSet(const std::string& fieldname, const std::string& modelname) {
  for (auto modelset : m_models[fieldname])
    if (modelset->modelName() == modelname)
      return modelset;

  return nullptr;
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
  return (*this);
}

Dataset::Builder& Dataset::Builder::withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi) {
  m_dataset->m_qoiNames.push_back(name);
  m_dataset->m_qois.push_back(qoi);
  return (*this);
}

Dataset::Builder& Dataset::Builder::withEmbedding(std::string name, FortranLinalg::DenseMatrix<Precision> &embedding) {
  m_dataset->m_embeddingNames.push_back(name);
  m_dataset->m_embeddings.push_back(embedding);
  return (*this);
}

Dataset::Builder& Dataset::Builder::withModel(std::string fieldname, std::shared_ptr<MSModelSet> modelset) {
  // only add model name if it doesn't already exist
  auto it = std::find(m_dataset->m_modelNames.begin(), m_dataset->m_modelNames.end(), modelset->modelName());
  if (it == m_dataset->m_modelNames.end())
    m_dataset->m_modelNames.push_back(modelset->modelName());
  
  m_dataset->m_msModelFields.push_back(fieldname);
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

