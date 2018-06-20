#pragma once

#include "Dataset.h"
#include "flinalg/Linalg.h"
#include "flinalg/LinalgIO.h"
#include "imageutils/Image.h"
#include "precision/Precision.h"
#include "yaml-cpp/yaml.h"

#include <string>
#include <vector>

typedef 
std::pair<std::string, FortranLinalg::DenseVector<Precision>> ParameterNameValuePair;
typedef 
std::pair<std::string, FortranLinalg::DenseVector<Precision>> QoiNameValuePair;
typedef
std::pair<std::string, FortranLinalg::DenseMatrix<Precision>> EmbeddingPair;

class DatasetLoader {
public:
  static Dataset* loadDataset(const std::string &filePath);
private:
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
  
  static FortranLinalg::DenseMatrix<Precision> parseDistances(
      const YAML::Node &config, const std::string &filePath);

  static std::vector<Image*> parseThumbnails(
          const YAML::Node &config, const std::string &filePath);
};

