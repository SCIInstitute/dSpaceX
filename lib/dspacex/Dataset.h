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

  std::vector<MSComplex>& getMSModels() {
    return m_msModels;
  }

  int getMSComplexIdxForFieldname(const std::string fieldname) const;
  dspacex::MSComplex& getMSComplex(const std::string fieldname);

  const Image& getThumbnail(unsigned idx) const;

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
    Builder& withMSModel(std::string name, dspacex::MSComplex ms_model);  // <ctc> auto ms_model?
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
  // todo: they're not msModels, just msComplexes (one per field, image, and parameter) whose crystals might have predictive models (e.g., ShapeOdds, SharedGP).
  // todo: there may even be predictive models not associated with any m-s complex
  // todo: maybe these should be renamed to EmbeddingModel or something like that... except a model just happens to be associated with a m-s embedding, so PredictiveModel is better
  std::vector<dspacex::MSComplex> m_msModels;  
  std::vector<std::string> m_msModelFields;
  std::vector<Image> m_thumbnails;
  std::string m_name;

  bool m_hasDistanceMatrix = false;
  bool m_hasSamplesMatrix = false;
};

} // dspacex
