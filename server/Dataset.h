#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "precision/Precision.h"

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

  FortranLinalg::DenseVector<Precision>& getAttributeVector(int i) {
    return m_attributes[i];
  }

  std::vector<std::string> getAttributeNames() {
    return m_attributeNames;
  }

  std::string getName() {
    return m_name;
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
    Builder& withAttribute(std::string name, FortranLinalg::DenseVector<Precision> &attribute) {
      m_dataset->m_attributeNames.push_back(name);
      m_dataset->m_attributes.push_back(attribute);
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
    Builder& withName(std::string name) {
      m_dataset->m_name = name;
      return (*this);
    }
    // Builder& addThumbnails(std::vector<> thumbnails);
    Dataset* build() {
      // TODO:  Add validation that sample counts match array sizes.
      //        Throw an exception if something doesn't match. 
      if (m_dataset->m_hasSamplesMatrix) {
        m_dataset->m_sampleCount = m_dataset->m_samplesMatrix.N();
      } else if (m_dataset->m_hasDistanceMatrix) {
        m_dataset->m_sampleCount = m_dataset->m_distanceMatrix.N();
      }

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
  std::vector<FortranLinalg::DenseVector<Precision>> m_attributes;
  std::vector<std::string> m_qoiNames;
  std::vector<std::string> m_attributeNames;
  std::vector<FortranLinalg::DenseMatrix<Precision>> m_embeddings;
  std::vector<std::string> m_embeddingNames;
  std::string m_name;

  bool m_hasDistanceMatrix = false;
  bool m_hasSamplesMatrix = false;
};