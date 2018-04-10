#include "DatasetLoader.h"
#include "yaml-cpp/yaml.h"


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