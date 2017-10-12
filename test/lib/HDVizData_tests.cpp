#include "gtest/gtest.h"
#include "FileCachedHDVizDataImpl.h"
#include "SimpleHDVizDataImpl.h"
#include "HDProcessResult.h"
#include "HDProcessResultSerializer.h"

#include <iostream>
#include <exception>

std::string data_dir = std::string(TEST_DATA_DIR);   

template <typename T>
void printMatrix(FortranLinalg::DenseMatrix<T> &matrix) {
  for (unsigned int i = 0; i < matrix.M(); i++) {
    for (unsigned int j = 0; j < matrix.N(); j++) {
      std::cout << matrix(i,j) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}


template <typename T>
void ASSERT_MATRIX_EQ(FortranLinalg::DenseMatrix<T> &a, FortranLinalg::DenseMatrix<T> &b) {
  ASSERT_EQ(a.M(), b.M());
  ASSERT_EQ(a.N(), b.N());
  for (unsigned int i = 0; i < a.M(); i++) {
    for (unsigned int j = 0; j < a.N(); j++) {
      ASSERT_EQ(a(i,j), b(i,j));
    }    
  }
}


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
  
  // Compare Persistence
  ASSERT_EQ(cachedData->getPersistence().N(), simpleData->getPersistence().N());
  unsigned int persistenceCount = cachedData->getPersistence().N();
  for (unsigned int level=0; level < persistenceCount; level++) {
    ASSERT_EQ(cachedData->getPersistence()(level), simpleData->getPersistence()(level));
  }

  // Compare Edges
  for (unsigned int level=0; level < persistenceCount; level++) {
    ASSERT_MATRIX_EQ(cachedData->getEdges(level), simpleData->getEdges(level));
  }
  

  // Compare 

  // cleanup
  delete cachedData;
  delete result;
}
