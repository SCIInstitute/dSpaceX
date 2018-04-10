#pragma once

#include "Dataset.h"
#include "yaml-cpp/yaml.h"

#include <string>
#include <vector>

class DatasetLoader {
public:
  DatasetLoader();    
  static Dataset* loadDataset(const std::string &filePath);
private:
  static std::string parseName(const YAML::Node &config);
  static int parseSampleCount(const YAML::Node &config);
};

