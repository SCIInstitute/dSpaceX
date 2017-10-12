PROJECT(HDProcessing)

SET(HDPROCESS_INCLUDE_FILES 
  HDProcessor.h 
  HDProcessResult.h
  HDVizData.h 
  FileCachedHDVizDataImpl.h 
  SimpleHDVizDataImpl.h)

SET(HDPROCESS_SOURCE_FILES 
  HDProcessor.cpp 
  HDProcessResultSerializer.cpp
  FileCachedHDVizDataImpl.cpp 
  SimpleHDVizDataImpl.cpp)

LINK_DIRECTORIES( ${ANN_LINK_DIR} )

# output library
ADD_LIBRARY(hdprocess ${HDPROCESS_INCLUDE_FILES} ${HDPROCESS_SOURCE_FILES})
TARGET_LINK_LIBRARIES(hdprocess ANN)
