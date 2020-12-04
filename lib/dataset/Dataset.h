#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "dataset/Precision.h"
#include "imageutils/Image.h"
#include "pmodels/Modelset.h"

#include <vector>

namespace dspacex {

class DatasetBuilder;

class Dataset {
 public:
  ~Dataset() { std::cerr << "Dataset::~Dataset()\n"; }

  int numberOfSamples() {
    return m_sampleCount;
  }

  int numberOfQois() {
    return m_qois.size();
  }

  int numberOfEmbeddings(std::string metric) {
    if (!hasDistanceMetric(metric)) return 0;
    return m_embeddings.at(metric).size();
  }

  int numberOfDistanceMetrics() {
    return m_distanceMetricNames.size();
  }

  int numberOfModels(std::string metric) {
    if (!hasDistanceMetric(metric)) {
      return m_models.at(metric).size();
    }
    else {
      return 0;
    }
  }

  bool hasGeometryMatrix() {
    return m_hasGeometryMatrix;
  }

  FortranLinalg::DenseMatrix<Precision>& getGeometryMatrix() {
    return m_geometryMatrix;
  }

  bool hasDistanceMetric(std::string metric) const {
    return std::find(m_distanceMetricNames.begin(),
                     m_distanceMetricNames.end(),
                     metric) != m_distanceMetricNames.end();
  }
  
  bool hasDistanceMatrix(std::string metric) const {
    bool has_matrix = m_distances.find(metric) != m_distances.end();
    if (has_matrix) assert(hasDistanceMetric(metric));
    return has_matrix;
  }
  
  FortranLinalg::DenseMatrix<Precision>& getDistanceMatrix(std::string metric) {
    if (!hasDistanceMatrix(metric)) {
      return m_distances.at(metric);
    }
    else throw std::runtime_error("unknown distance matrix requested");
  }

  FortranLinalg::DenseVector<Precision>& getQoiVector(int i, bool normalized = false) {
    return normalized ? m_normalized_qois[i] : m_qois[i];
  }

  const std::vector<std::string>& getQoiNames() const {
    return m_qoiNames;
  }

  FortranLinalg::DenseMatrix<Precision>& getEmbeddingMatrix(std::string metric, int i) {
    if (!hasDistanceMetric(metric)) {
      throw std::runtime_error("tried to fetch embedding for unknown metric " + metric);
    }
    else {
      if (i >= m_embeddings.at(metric).size()) {
        throw std::runtime_error("there is no " + std::to_string(i) + "th embedding for metric " + metric);
      }
      return m_embeddings.at(metric)[i];
    }
  }

  std::vector<std::string>& getEmbeddingNames(std::string metric) {
    if (!hasDistanceMetric(metric)) {
      throw std::runtime_error("tried to fetch embedding names for unknown metric " + metric);
    }
    return m_embeddingNames.at(metric);
  }

  const std::vector<std::string>& getDistanceMetricNames() const {
    return m_distanceMetricNames;
  }

  const std::vector<std::string>& getModelNames(std::string metric, const std::string& fieldname) const;

  FortranLinalg::DenseVector<Precision>& getParameterVector(int i, bool normalized = false) {
    return normalized ? m_normalized_parameters[i] : m_parameters[i];
  }

  std::vector<std::string>& getParameterNames() {
    return m_parameterNames;
  }

  std::string getName() {
    return m_name;
  }

  std::vector<Image> getThumbnails() {
    return m_thumbnails;
  }

  ModelMap& getModels(std::string metric) {
    if (!hasDistanceMetric(metric)) {
      throw std::runtime_error("tried to fetch model for unknown metric " + metric);
    }
    else {
      return m_models.at(metric);
    }
  }

  /// return MSModelset associated with this field and modelname
  std::shared_ptr<MSModelset> getModelset(std::string metric, const std::string& fieldname, const std::string& modelname);

  /// return the image thumbnail for this sample index
  const Image& getThumbnail(int idx) const;

  /// return the field values for the given field (of the specified type, otherwise the first with that name)
  Eigen::Map<Eigen::VectorXf> getFieldvalues(const std::string &name, Fieldtype type = Fieldtype::Unknown,
                                             bool normalized = false);

 private:
  int m_sampleCount;
  std::string m_name;
  std::vector<Image> m_thumbnails;

  std::vector<std::string> m_qoiNames;
  std::vector<std::string> m_parameterNames;
  std::vector<FortranLinalg::DenseVector<Precision>> m_qois;
  std::vector<FortranLinalg::DenseVector<Precision>> m_parameters;
  std::vector<FortranLinalg::DenseVector<Precision>> m_normalized_qois;
  std::vector<FortranLinalg::DenseVector<Precision>> m_normalized_parameters;

  std::vector<std::string> m_distanceMetricNames;
  std::map<std::string, FortranLinalg::DenseMatrix<Precision>> m_distances; // distances per metric

  std::map<std::string, std::vector<std::string>> m_embeddingNames;                       // embedding names per metric
  std::map<std::string, std::vector<FortranLinalg::DenseMatrix<Precision>>> m_embeddings; // embeddings per metric

  std::map<std::string, std::vector<std::string>> m_msModelFields;  // fieldnames of the modelsets in each metric
  std::map<std::string, ModelMap> m_models; // map of dist_metrics' sets of fields' sets of models associated with crystals in M-S 

  bool m_hasGeometryMatrix = false;
  FortranLinalg::DenseMatrix<Precision> m_geometryMatrix;

  friend class DatasetBuilder;
};

} // dspacex
