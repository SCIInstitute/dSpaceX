#include "gtest/gtest.h"
#include "DatasetLoader.h"
#include "Dataset.h"

const std::string kExampleDirPath = std::string(EXAMPLE_DATA_DIR);   

//---------------------------------------------------------------------
// Tests
//---------------------------------------------------------------------

TEST(DatasetLoader, loadDataset) {  

  std::string filePath = kExampleDirPath + "gaussian2d/gaussian.yaml";
  Dataset *dataset = nullptr;
  try {
    dataset = DatasetLoader::loadDataset(filePath);
  } catch(std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

const std::vector<std::pair<std::string, std::string>> datasetPaths = { 
    {"Concrete", "../../examples/concrete/"},
    {"Crimes", "../../examples/crimes/"},
    {"Gaussian", "../../examples/gaussian2d/"},
    {"Colorado", "../../examples/truss/"},  
  };