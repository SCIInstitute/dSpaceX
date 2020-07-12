#pragma once

#include "Dataset.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "imageutils/Image.h"
#include "dspacex/Precision.h"
#include "yaml-cpp/yaml.h"
#include "pmodels/MorseSmale.h"

#include <string>
#include <vector>

namespace dspacex {

using ParameterNameValuePair = std::pair<std::string, FortranLinalg::DenseVector<Precision>>;
using QoiNameValuePair = std::pair<std::string, FortranLinalg::DenseVector<Precision>>;
using EmbeddingPair = std::pair<std::string, FortranLinalg::DenseMatrix<Precision>>;

class DatasetLoader {
public:
  static std::unique_ptr<Dataset> loadDataset(const std::string &filePath);
  static std::string getDatasetName(const std::string &filePath);

private:
  // Disallow creating an instance of this object (reinforces its purpose)
  DatasetLoader() {}

  static std::string parseName(const YAML::Node &config);

  static int parseSampleCount(const YAML::Node &config);

  static FortranLinalg::DenseMatrix<Precision> parseGeometry(
      const YAML::Node &config, const std::string &filePath);

  static std::vector<ParameterNameValuePair> parseParameters(
      const YAML::Node &config, const std::string &filePath);

  static ParameterNameValuePair parseParameter(
      const YAML::Node &parameterNode, const std::string &filePath);

  static std::vector<QoiNameValuePair> parseQois(
      const YAML::Node &config, const std::string &filePath);

  static QoiNameValuePair parseQoi(
      const YAML::Node &qoiNode,const std::string &filePath);

  static std::vector<EmbeddingPair> parseEmbeddings(
      const YAML::Node &config, const std::string &filePath);

  static EmbeddingPair parseEmbedding(
      const YAML::Node &embeddingNode, const std::string &filePath);

  static ModelMap parseModels(const YAML::Node &config,
                              const std::string &filePath);

  static std::unique_ptr<MSModelSet> parseModel(const YAML::Node& modelNode,
                                                 const std::string& filePath);

  static void parseModel(const std::string &modelPath, Model &m);

  static FortranLinalg::DenseMatrix<Precision> parseDistances(
      const YAML::Node &config, const std::string &filePath);

  static std::vector<Image> parseThumbnails(
      const YAML::Node &config, const std::string &filePath);

  static std::string createThumbnailPath(const std::string& imageBasePath,
      int index, const std::string imageSuffix, unsigned int indexOffset,
      bool padZeroes, unsigned int thumbnailCount);
};

} // dspacex
