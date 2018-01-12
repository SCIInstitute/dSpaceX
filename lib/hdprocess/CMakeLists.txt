PROJECT(HDProcessing)

SET(HDPROCESS_INCLUDE_FILES 
  HDProcessor.h 
  HDGenericProcessor.h
  HDProcessResult.h
  HDVizData.h 
  FileCachedHDVizDataImpl.h 
  SimpleHDVizDataImpl.h
  TopologyData.h
  LegacyTopologyDataImpl.h
  util/csv/loaders.h)

SET(HDPROCESS_SOURCE_FILES
  HDProcessor.cpp 
  HDProcessResultSerializer.cpp
  FileCachedHDVizDataImpl.cpp 
  SimpleHDVizDataImpl.cpp
  LegacyTopologyDataImpl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/util/csv/loaders.cpp)

LINK_DIRECTORIES( ${ANN_LINK_DIR} )

# output library
ADD_LIBRARY(hdprocess ${HDPROCESS_INCLUDE_FILES} ${HDPROCESS_SOURCE_FILES})
TARGET_LINK_LIBRARIES(hdprocess ANN)
