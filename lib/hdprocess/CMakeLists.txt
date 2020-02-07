PROJECT(HDProcessing)

SET(HDPROCESS_HEADER_FILES
  HDProcessor.h
  HDGenericProcessor.h
  HDProcessResult.h
  HDVizData.h
  FileCachedHDVizDataImpl.h
  SimpleHDVizDataImpl.h
  TopologyData.h
  LegacyTopologyDataImpl.h
  )

SET(HDPROCESS_SOURCE_FILES
  HDProcessor.cpp
  HDProcessResultSerializer.cpp
  FileCachedHDVizDataImpl.cpp
  SimpleHDVizDataImpl.cpp
  LegacyTopologyDataImpl.cpp
  )

ADD_LIBRARY(hdprocess ${HDPROCESS_HEADER_FILES} ${HDPROCESS_SOURCE_FILES})
TARGET_LINK_LIBRARIES(hdprocess dspacex_utils ANN)
