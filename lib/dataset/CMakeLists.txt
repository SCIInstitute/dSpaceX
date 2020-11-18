SET(DATASET_HEADER_FILES
  Precision.h
  Fieldtype.h
  DatasetLoader.h
  Dataset.h
  ValueIndexPair.h
)

SET(DATASET_SOURCE_FILES
  DatasetLoader.cpp
  Dataset.cpp
)

ADD_LIBRARY(dataset ${DATASET_HEADER_FILES} ${DATASET_SOURCE_FILES})
TARGET_INCLUDE_DIRECTORIES(dataset PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${EIGEN3_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>)
TARGET_LINK_LIBRARIES(dataset
  pmodels
  dspacex_utils
  imageutils
  )
