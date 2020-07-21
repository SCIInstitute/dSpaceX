#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "dspacex/Precision.h"
#include "imageutils/Image.h"
#include "pmodels/MorseSmale.h"

#include <vector>

namespace dspacex {

class Dataset {
 public:
  ~Dataset() { std::cerr << "Dataset::~Dataset()\n"; }

  bool valid() const;

  int numberOfSamples() {
    return m_sampleCount;
  }

  int numberOfQois() {
    return m_qois.size();
  }

  int numberOfEmbeddings() {
    return m_embeddings.size();
  }

  int numberOfModels() {
    return m_models.size();
  }

  bool hasSamplesMatrix() {
    return m_hasSamplesMatrix;
  }

  FortranLinalg::DenseMatrix<Precision>& getSamplesMatrix() {
    return m_samplesMatrix;
  }

  bool hasDistanceMatrix() {
    return m_hasDistanceMatrix;
  }

  FortranLinalg::DenseMatrix<Precision>& getDistanceMatrix() {
    return m_distanceMatrix;
  }

  FortranLinalg::DenseVector<Precision>& getQoiVector(int i) {
    return m_qois[i];
  }

  std::vector<std::string>& getQoiNames() {
    return m_qoiNames;
  }

  FortranLinalg::DenseMatrix<Precision>& getEmbeddingMatrix(int i) {
    return m_embeddings[i];
  }

  std::vector<std::string>& getEmbeddingNames() {
    return m_embeddingNames;
  }

  std::vector<std::string>& getModelNames() {
    return m_modelNames;
  }

  FortranLinalg::DenseVector<Precision>& getParameterVector(int i) {
    return m_parameters[i];
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

  ModelMap& getModels() {
    return m_models;
  }

  /// is there a Model associated with this field, modelname, persistence, and crystal?
  bool hasModel(const std::string& fieldname, const std::string& modelname, int persistence, int crystal) const;

  /// return specific Model associated with this field, modelname, persistence, and crystal
  std::shared_ptr<Model> getModel(const std::string& fieldname, const std::string& modelname, int persistence, int crystal);

  /// return MSModelSet associated with this field and modelname
  std::shared_ptr<MSModelSet> getModelSet(const std::string& fieldname, const std::string& modelname);

  /// return the image thumbnail for this sample index
  const Image& getThumbnail(int idx) const;

  /// return the field values for the given field (of the specified type, otherwise the first with that name)
  Eigen::Map<Eigen::VectorXf> getFieldvalues(const std::string &name, Fieldtype type = Fieldtype::Unknown);

  // Builder includes necessary functions for a user to build and return a dSpaceX Dataset
  class Builder {
   public:
    Builder() {
      m_dataset = std::make_unique<Dataset>();
    }
    std::unique_ptr<Dataset> build();
    Builder& withSampleCount(int count);
    Builder& withSamplesMatrix(FortranLinalg::DenseMatrix<Precision> &samplesMatrix);     // TODO:  change to withGeometry of templatized form.
    Builder& withDistanceMatrix(FortranLinalg::DenseMatrix<Precision> &distanceMatrix);
    Builder& withParameter(std::string name, FortranLinalg::DenseVector<Precision> &parameter);
    Builder& withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi);
    Builder& withEmbedding(std::string name, FortranLinalg::DenseMatrix<Precision> &embedding);
    Builder& withModel(std::string name, std::shared_ptr<MSModelSet> modelset);
    Builder& withName(std::string name);
    Builder& withThumbnails(std::vector<Image> thumbnails);
    
  private:
    std::unique_ptr<Dataset> m_dataset;
  };

 private:
  int m_sampleCount;
  FortranLinalg::DenseMatrix<Precision> m_samplesMatrix;
  FortranLinalg::DenseMatrix<Precision> m_distanceMatrix;
  std::vector<FortranLinalg::DenseVector<Precision>> m_qois;
  std::vector<FortranLinalg::DenseVector<Precision>> m_parameters;
  std::vector<std::string> m_qoiNames;
  std::vector<std::string> m_parameterNames;
  std::vector<FortranLinalg::DenseMatrix<Precision>> m_embeddings;
  std::vector<std::string> m_embeddingNames;
  std::vector<std::string> m_modelNames;      // Names of individual models (e.g., PCA1, PCA2 for diff params used to compute PCA)
  ModelMap m_models;                  // map of fields' sets of models associated with crystals in a Morse-Smale decomposition 
  std::vector<std::string> m_msModelFields;
  std::vector<Image> m_thumbnails;
  std::string m_name;

  bool m_hasDistanceMatrix = false;
  bool m_hasSamplesMatrix = false;
};

} // dspacex
