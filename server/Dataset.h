#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "precision/Precision.h"
#include "imageutils/Image.h"
#include "pmodels/MorseSmale.h"

#include <vector>


class Dataset {
 public:
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

  std::vector<std::string> getParameterNames() {
    return m_parameterNames;
  }

  std::string getName() {
    return m_name;
  }

  std::vector<Image> getThumbnails() {
    return m_thumbnails;
  }

  std::vector<PModels::MSComplex>& getMSModels() {
    return m_msModels;
  }

  PModels::MSComplex& getMSComplex(const std::string fieldname)
  {
    // A dataset can have models for more than one field, so use fieldname argument to find index of the ms_complex for the given fieldname.
    //   NOTE: passed fieldname is specified in (e.g., CantileverBeam_QoIs.csv) as plain text (e.g., "Max Stress"),
    //         but (FIXME) the ms_complex thinks its fieldname is (e.g.,) maxStress.
    std::vector<PModels::MSComplex> &ms_complexes(getMSModels());
    unsigned idx = 0;
    for (; idx < ms_complexes.size(); idx++)
      if (ms_complexes[idx].getFieldname() == fieldname) break;
    if (idx >= ms_complexes.size())
      throw std::runtime_error("ERROR: no model for fieldname " + fieldname);

    return ms_complexes[idx];
  }

  const Image& getThumbnail(unsigned idx) {
    if (idx >= m_thumbnails.size())
      throw std::runtime_error("Tried to fetch thumbnail " + std::to_string(idx) + ", but there are only " + std::to_string(m_thumbnails.size()));
    return m_thumbnails[idx];
  }

  class Builder {
   public:
    Builder() {
      m_dataset = new Dataset();
    }
    Builder& withSampleCount(int count) {
      m_dataset->m_sampleCount = count;
      return (*this);
    }
    // TODO:  change to withGeometry of templatized form.
    Builder& withSamplesMatrix(FortranLinalg::DenseMatrix<Precision> &samplesMatrix) {
      m_dataset->m_samplesMatrix = samplesMatrix;
      m_dataset->m_hasSamplesMatrix = true;
      return (*this);
    }
    Builder& withDistanceMatrix(FortranLinalg::DenseMatrix<Precision> &distanceMatrix) {
      m_dataset->m_distanceMatrix = distanceMatrix;
      m_dataset->m_hasDistanceMatrix = true;
      return (*this);
    }
    Builder& withParameter(std::string name, FortranLinalg::DenseVector<Precision> &parameter) {
      m_dataset->m_parameterNames.push_back(name);
      m_dataset->m_parameters.push_back(parameter);
      return (*this);
    }
    Builder& withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi) {
      m_dataset->m_qoiNames.push_back(name);
      m_dataset->m_qois.push_back(qoi);
      return (*this);
    }
    Builder& withEmbedding(std::string name, FortranLinalg::DenseMatrix<Precision> &embedding) {
      m_dataset->m_embeddingNames.push_back(name);
      m_dataset->m_embeddings.push_back(embedding);
      return (*this);
    }
    Builder& withMSModel(std::string name, PModels::MSComplex &ms_model) {
      m_dataset->m_msModelFields.push_back(name);
      m_dataset->m_msModels.push_back(ms_model);
      return (*this);
    }
    Builder& withName(std::string name) {
      m_dataset->m_name = name;
      return (*this);
    }
    Builder& withThumbnails(std::vector<Image> thumbnails) {
      m_dataset->m_thumbnails = thumbnails;
      return (*this);
    }
    Dataset* build() {
      // TODO:  Add validation that sample counts match array sizes.
      //        Throw an exception if something doesn't match. 
      // m_dataset->m_hasSamplesMatrix ==> m_dataset->m_samplesMatrix.N()
      // _dataset->m_hasDistanceMatrix ==> m_dataset->m_distanceMatrix.N();
      return m_dataset;
    }
   private:
    Dataset *m_dataset;
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
  std::vector<PModels::MSComplex> m_msModels;  // PModels models are per M-S complex and stored so they can be accessed by crystals in a given persistence level. <ctc> maybe these should be renamed to EmbeddingModel or something like that
  std::vector<std::string> m_msModelFields;
  std::vector<Image> m_thumbnails;
  std::string m_name;

  bool m_hasDistanceMatrix = false;
  bool m_hasSamplesMatrix = false;
};
