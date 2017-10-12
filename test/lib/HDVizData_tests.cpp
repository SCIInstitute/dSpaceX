#include "gtest/gtest.h"
#include "FileCachedHDVizDataImpl.h"
#include "SimpleHDVizDataImpl.h"
#include "HDProcessResult.h"
#include "HDProcessResultSerializer.h"

#include <iostream>
#include <exception>

std::string data_dir = std::string(TEST_DATA_DIR);   

TEST(FileCachedHDVizDataImpl, loadsFiles) {  
  try {
    HDVizData *data = new FileCachedHDVizDataImpl(data_dir);
  } catch(std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}

TEST(SimpleHDVizDataImpl, loadData) {
  HDProcessResult result;
  HDVizData* data = new SimpleHDVizDataImpl(&result);
}


/** 
 * Compare persistence information of FileCachedHDVizDataImpl
 * with newer SimpleHDVizDataImpl.
 */
TEST(HDVizData, persistence) {
  HDVizData *cachedData = nullptr;
  try {
    cachedData = new FileCachedHDVizDataImpl(data_dir);
  } catch(std::exception &e) {
    std::cout << e.what() << std::endl;
  } 

  HDProcessResult *result = HDProcessResultSerializer::read(data_dir);  
  HDVizData *simpleData = new SimpleHDVizDataImpl(result); 
  
  ASSERT_EQ(cachedData->getPersistence().N(), simpleData->getPersistence().N());
  for (int level=0; level < cachedData->getPersistence().N(); level++) {
    ASSERT_EQ(cachedData->getPersistence()(level), simpleData->getPersistence()(level));
  }

  // cleanup
  delete cachedData;
  delete result;
}

