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

const Image& Dataset::getThumbnail(unsigned idx) const
{
  if (idx >= m_thumbnails.size())
    throw std::runtime_error("Tried to fetch thumbnail " + std::to_string(idx) + ", but there are only " + std::to_string(m_thumbnails.size()));
  return m_thumbnails[idx];
}

int Dataset::getMSComplexIdxForFieldname(const std::string fieldname) const
{
  // A dataset can have models for more than one field, so use fieldname argument to find index of the ms_complex for the given fieldname.
  //   NOTE: passed fieldname is specified in (e.g., CantileverBeam_QoIs.csv) as plain text (e.g., "Max Stress"),
  //         but (FIXME) the ms_complex thinks its fieldname is (e.g.,) maxStress.
  int idx = 0;
  for (; idx < m_msModels.size(); idx++)
    if (m_msModels[idx].getFieldname() == fieldname)
      return idx;
  return -1;
}

dspacex::MSComplex& Dataset::getMSComplex(const std::string fieldname)
{
  int idx = getMSComplexIdxForFieldname(fieldname);
  if (idx < 0)
    throw std::runtime_error("ERROR: no model for fieldname " + fieldname);

  return m_msModels[idx];
}

///////////////////////////////////////////////////////////////////////////////
// Dataset::Builder

std::unique_ptr<Dataset> Dataset::Builder::build()
{
  if (!m_dataset || !m_dataset->valid())
    throw std::runtime_error("dspacex::Dataset in Dataset::Builder is not valid.");

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

Dataset::Builder& Dataset::Builder::withMSModel(std::string name, dspacex::MSComplex ms_model) {

  // TODO: add model types as they're read rather than hardcoding them here

  // only add model type if it doesn't already exist
  std::vector<std::string> types{"ShapeOdds", "PCA", "SharedGP"};
  for (auto type : types) {
    auto it = std::find(m_dataset->m_modelTypes.begin(), m_dataset->m_modelTypes.end(), type);
    if (it != m_dataset->m_modelTypes.end())
      m_dataset->m_modelTypes.push_back(type);
  }
  
  m_dataset->m_msModelFields.push_back(name);
  m_dataset->m_msModels.push_back(ms_model); // ...or std::move(ms_model)
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

