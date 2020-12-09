SET(UTILS_HEADER_FILES
  Heap.h
  IO.h
  MaxHeap.h
  MinHeap.h
  Random.h 
  StringUtils.h
  utils.h
  loaders.h
  Data.h
)

SET(UTILS_SOURCE_FILES
  StringUtils.cpp
  utils.cpp
  loaders.cpp
)

ADD_LIBRARY(dspacex_utils ${UTILS_HEADER_FILES} ${UTILS_SOURCE_FILES})
TARGET_INCLUDE_DIRECTORIES(dspacex_utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)
