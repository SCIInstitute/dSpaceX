#include "DatasetLoader.h"
#include "yaml-cpp/yaml.h"

#include <memory>


std::string basePathOf(const std::string &filePath) {
  size_t found = filePath.find_last_of("/\\");
  std::string path = filePath.substr(0,found);  
  if(path.empty()) {
    path = "./";
  }
  if (path[path.size() - 1] != '/') {
    path = path + "/";
  }
  return path;
}

Dataset* DatasetLoader::loadDataset(const std::string &filePath) {
  YAML::Node config = YAML::LoadFile(filePath);  
  Dataset::Builder builder;

  std::cout << "Reading " << filePath << std::endl;
  
  std::string name = DatasetLoader::parseName(config);
  std::cout << "name: " << name << std::endl;    
  builder.withName(name);
  
  int sampleCount = DatasetLoader::parseSampleCount(config);
  std::cout << "samples: " << sampleCount << std::endl;
  builder.withSampleCount(sampleCount);

  auto qois = DatasetLoader::parseQois(config, filePath);
  for (auto qoi : qois) {
    builder.withQoi(qoi.first, qoi.second);
  }

  auto geometry = DatasetLoader::parseGeometry(config, filePath);
  builder.withSamplesMatrix(geometry);

  return builder.build();
}


std::string DatasetLoader::parseName(const YAML::Node &config) {  
  if (!config["name"]) {
    throw std::runtime_error("Dataset config missing 'name' field.");
  }
  return config["name"].as<std::string>();
}


int DatasetLoader::parseSampleCount(const YAML::Node &config) {
  if(!config["samples"]) {
    throw std::runtime_error("Dataset config missing 'samples' field.");
  }
  if (!config["samples"]["count"]) {
    throw std::runtime_error("Dataset config samples missing 'counts' field.");
  }
  return config["samples"]["count"].as<int>();
}


std::vector<QoiNameValuePair> DatasetLoader::parseQois(
    const YAML::Node &config, const std::string &filePath) {
  std::vector<QoiNameValuePair> qois;
  if(!config["qois"]) {
    throw std::runtime_error("Dataset config missing 'qois' field.");
  }
  const YAML::Node &qoisNode = config["qois"];
  
  if (qoisNode.IsSequence()) {
    // Parse QoIs one by one if in list form:
    std::cout << "Reading sequence of " << qoisNode.size() << " qois" << std::endl;
    for (std::size_t i = 0; i < qoisNode.size(); i++) {
      const YAML::Node &qoiNode = qoisNode[i];
      auto qoi = DatasetLoader::parseQoi(qoiNode, filePath);
      qois.push_back(qoi);
    }

  } else {
    // Parse all Qois from a file if not in list form.
    // TODO: Implement
  }

  return qois;
}

QoiNameValuePair DatasetLoader::parseQoi(
  const YAML::Node &qoiNode, const std::string &filePath) {
  if (!qoiNode["name"]) {
    throw std::runtime_error("Qoi missing 'name' field.");
  }
  std::string name = qoiNode["name"].as<std::string>();
  std::cout << "qoi name: " << name << std::endl;

  if (!qoiNode["format"]) {
     throw std::runtime_error("Qoi missing 'format' field."); 
  }
  std::string format = qoiNode["format"].as<std::string>();
  std::cout << "qoi format: " << format << std::endl;
  if (format != "Linalg.DenseVector") {
    throw std::runtime_error(
        "Dataset config specifies unsupported qoi format: " + format);
  }

  if (!qoiNode["file"]) {
    throw std::runtime_error("Qoi missing 'file' field.");
  }
  std::string filename = qoiNode["file"].as<std::string>();
  std::cout << "qoi filename: " << filename << std::endl;
  std::string path = basePathOf(filePath);

  std::cout << "Loading Linalg.DenseVector from " << path + filename << std::endl;
  auto qoi = FortranLinalg::LinalgIO<Precision>::readVector(path + filename);

  return QoiNameValuePair(name, qoi);
}


FortranLinalg::DenseMatrix<Precision> DatasetLoader::parseGeometry(
    const YAML::Node &config, const std::string &filePath) {
  if(!config["geometry"]) {
    throw std::runtime_error("Dataset config missing 'geometry' field.");
  }
  const YAML::Node &geometryNode = config["geometry"];

  if (!geometryNode["format"]) {
    throw std::runtime_error("Dataset config missing 'geometry.format' field.");
  }
  std::string format = geometryNode["format"].as<std::string>();
  if (format != "Linalg.DenseMetrix") {
    throw std::runtime_error(
          "Dataset config specifies unsupported geometry format: " + format);
  }

  if (!geometryNode["file"]) {
    throw std::runtime_error("Dataset config missing 'geometry.file' field.");
  }
  std::string filename = geometryNode["file"].as<std::string>();
  std::string path = basePathOf(filePath);


  auto x = FortranLinalg::LinalgIO<Precision>::readMatrix(path + filename);
  return x;
}


FortranLinalg::DenseMatrix<Precision> DatasetLoader::parseDistances(
    const YAML::Node &config) {

}