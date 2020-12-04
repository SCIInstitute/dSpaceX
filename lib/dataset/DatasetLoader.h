#pragma once

#include "Dataset.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "imageutils/Image.h"
#include "dataset/Precision.h"
#include <yaml-cpp/yaml.h>
#include "pmodels/Modelset.h"

#include <string>
#include <vector>

namespace dspacex {

using FieldNameValuePair = std::pair<std::string, FortranLinalg::DenseVector<Precision>>;  // field name to its values
using EmbeddingPair = std::pair<std::string, FortranLinalg::DenseMatrix<Precision>>;       // embedding name to embedding matrix
using DistancePair = std::pair<std::string, FortranLinalg::DenseMatrix<Precision>>;        // metric name to distance matrix
using ModelMapPair = std::pair<std::string, ModelMap>;                         // metric name to map of fields to modelsets

class DatasetLoader {
public:
  static std::unique_ptr<Dataset> loadDataset(const std::string &basePath);
  static std::string getDatasetName(const std::string &basePath);

  // load models on demand for interpolation since they can be very large
  static void parseModel(const std::string &modelPath, Model &m, const std::vector<ValueIndexPair> &sample_indices);

private:
  // Disallow creating an instance of this object (reinforces its purpose)
  DatasetLoader() {}

  static std::string parseName(const YAML::Node &config);

  static int parseSampleCount(const YAML::Node &config);

  static FortranLinalg::DenseMatrix<Precision> parseGeometry(const YAML::Node &config, const std::string &basePath);

  static std::vector<FieldNameValuePair> parseFields(const YAML::Node &node, const std::string &basePath);
  static std::vector<FieldNameValuePair> parseParameters(const YAML::Node &config, const std::string &basePath);
  static std::vector<FieldNameValuePair> parseQois(const YAML::Node &config, const std::string &basePath);

  static std::map<std::string, std::vector<EmbeddingPair>> parseEmbeddings(const YAML::Node &config, const std::string &basePath);
  static EmbeddingPair parseEmbedding(const YAML::Node &embeddings, const std::string &basePath);

  static std::vector<ModelMapPair> parseMetricModelsets(const YAML::Node& config, const std::string& filepath);
  static ModelMap parseModels(const YAML::Node &modelsNode, const std::string &basePath);
  static std::unique_ptr<MSModelset> parseModelset(const YAML::Node& model, const std::string& basePath);

  static std::vector<DistancePair> parseDistances(const YAML::Node &config, const std::string &basePath);

  static std::vector<Image> parseThumbnails(const YAML::Node &config, const std::string &basePath);

  static std::string createThumbnailPath(const std::string& imageBasePath,
      int index, const std::string imageSuffix, unsigned int indexOffset,
      bool padZeroes, unsigned int thumbnailCount);
};

// Builder includes necessary functions for a user to build and return a dSpaceX Dataset
class DatasetBuilder {
public:
  DatasetBuilder() {
    m_dataset = std::make_unique<Dataset>();
  }
  std::unique_ptr<Dataset> build();
  DatasetBuilder& withSampleCount(int count);
  DatasetBuilder& withGeometryMatrix(FortranLinalg::DenseMatrix<Precision> &geometryMatrix);
  DatasetBuilder& withDistances(std::vector<DistancePair>& distances);
  DatasetBuilder& withParameter(std::string name, FortranLinalg::DenseVector<Precision> &parameter);
  DatasetBuilder& withQoi(std::string name, FortranLinalg::DenseVector<Precision> &qoi);
  DatasetBuilder& withEmbeddings(std::string metric, std::vector<EmbeddingPair>& embeddings);
  DatasetBuilder& withModelsets(std::string metric, ModelMap& modelsets);
  DatasetBuilder& withName(std::string name);
  DatasetBuilder& withThumbnails(std::vector<Image> thumbnails);
    
private:
  std::unique_ptr<Dataset> m_dataset;
};

} // dspacex
