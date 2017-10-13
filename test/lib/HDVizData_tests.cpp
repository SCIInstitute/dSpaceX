#include "gtest/gtest.h"
#include "FileCachedHDVizDataImpl.h"
#include "SimpleHDVizDataImpl.h"
#include "HDProcessResult.h"
#include "HDProcessResultSerializer.h"

#include <iostream>
#include <exception>


//---------------------------------------------------------------------
// Declarations
//---------------------------------------------------------------------

std::string data_dir = std::string(TEST_DATA_DIR);   

template <typename T>
void printVector(FortranLinalg::DenseVector<T> &vector) {
  for (unsigned int i = 0; i < vector.N(); i++) {    
      std::cout << vector(i) << " ";
  }
  std::cout << std::endl;
}

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
void ASSERT_VECTOR_EQ(FortranLinalg::DenseVector<T> &a, FortranLinalg::DenseVector<T> &b) {
  ASSERT_EQ(a.N(), b.N());
  for (unsigned int i = 0; i < a.N(); i++) {
    ASSERT_EQ(a(i), b(i));
  }    
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

//---------------------------------------------------------------------
// Tests
//---------------------------------------------------------------------

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
TEST(HDVizData, compare) {
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
  unsigned int persistenceCount = simpleData->getPersistence().N();
  for (unsigned int level=0; level < persistenceCount; level++) {
    ASSERT_EQ(cachedData->getPersistence()(level), simpleData->getPersistence()(level));
  }

  ASSERT_VECTOR_EQ(cachedData->getRMin(), simpleData->getRMin());
  ASSERT_VECTOR_EQ(cachedData->getRMax(), simpleData->getRMax());

  // Compare Persistence Level Data
  for (unsigned int level=0; level < persistenceCount; level++) {    
    ASSERT_MATRIX_EQ(cachedData->getEdges(level), simpleData->getEdges(level));
    unsigned int crystalCount = simpleData->getEdges(level).N();

    ASSERT_VECTOR_EQ(cachedData->getExtremaValues(level), simpleData->getExtremaValues(level));
    ASSERT_VECTOR_EQ(cachedData->getExtremaNormalized(level), simpleData->getExtremaNormalized(level));
    ASSERT_VECTOR_EQ(cachedData->getExtremaWidthsScaled(level), simpleData->getExtremaWidthsScaled(level));

    for (unsigned int i = 0; i < crystalCount; i++) {
      ASSERT_MATRIX_EQ(cachedData->getReconstruction(level)[i], simpleData->getReconstruction(level)[i]);
      ASSERT_MATRIX_EQ(cachedData->getVariance(level)[i], simpleData->getVariance(level)[i]);
      ASSERT_MATRIX_EQ(cachedData->getGradient(level)[i], simpleData->getGradient(level)[i]);
    }  
    
    ASSERT_VECTOR_EQ(cachedData->getRsMin(level), simpleData->getRsMin(level));
    ASSERT_VECTOR_EQ(cachedData->getRsMax(level), simpleData->getRsMax(level));
    ASSERT_VECTOR_EQ(cachedData->getGradientMin(level), simpleData->getGradientMin(level));
    ASSERT_VECTOR_EQ(cachedData->getGradientMax(level), simpleData->getGradientMax(level));

    ASSERT_EQ(cachedData->getExtremaMinValue(level), simpleData->getExtremaMinValue(level));
    ASSERT_EQ(cachedData->getExtremaMaxValue(level), simpleData->getExtremaMaxValue(level));

    ASSERT_EQ(cachedData->getWidthMin(level), simpleData->getWidthMin(level));
    ASSERT_EQ(cachedData->getWidthMax(level), simpleData->getWidthMax(level));
  }  
 

  // cleanup
  delete cachedData;
  delete result;
}
