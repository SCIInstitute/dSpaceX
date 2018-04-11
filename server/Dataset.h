#pragma once

#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "precision/Precision.h"

#include <vector>


class Dataset {
 public:
  int numberOfSamples() {
    return m_distances.M();
  }

  int numberOfQois() {
    return m_qois.size();
  }

  FortranLinalg::DenseMatrix<Precision>& getDistanceMatrix() {
    return m_distances;
  }

  FortranLinalg::DenseVector<Precision>& getQoiVector(int i) {
    return m_qois[i];
  }

  std::vector<std::string> getQoiNames() {
    return m_qoiNames;
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
    }
    Builder& withDistanceMatrix(FortranLinalg::DenseMatrix<Precision> &distanceMatrix) {
      m_dataset->m_distances = distanceMatrix;
      return (*this);
    }
    Builder& withAttribute(std::string name, FortranLinalg::DenseVector<Precision> &vector) {
      m_dataset->m_attributeNames.push_back(name);
      m_dataset->m_attributes.push_back(vector);
      return (*this);
    }
    Builder& withQoi(std::string name, FortranLinalg::DenseVector<Precision> &vector) {
      m_dataset->m_qoiNames.push_back(name);
      m_dataset->m_qois.push_back(vector);
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
      return m_dataset;
    }
   private:
    Dataset *m_dataset;
  };
 private:
  int m_sampleCount;
  FortranLinalg::DenseMatrix<Precision> m_samplesMatrix;
  FortranLinalg::DenseMatrix<Precision> m_distances;
  std::vector<FortranLinalg::DenseVector<Precision>> m_qois;
  std::vector<FortranLinalg::DenseVector<Precision>> m_attributes;
  std::vector<std::string> m_qoiNames;
  std::vector<std::string> m_attributeNames;
  std::string m_name;
};