#include "gtest/gtest.h"
#include "FileCachedHDVizDataImpl.h"
#include "SimpleHDVizDataImpl.h"
#include "HDProcessResult.h"

#include <iostream>
#include <exception>

std::string data_dir = std::string(TEST_DATA_DIR);   

TEST(FileCachedHDVizDataImpl, loadsFiles) {  
  try {
    HDVizData* data = new FileCachedHDVizDataImpl(data_dir);
  } catch(std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(SimpleHDVizDataImpl, loadData) {
  HDProcessResult result;
  HDVizData* data = new SimpleHDVizDataImpl(&result);
}
