PROJECT(SharedGP)

# libigl
# TODO: automatically clone and include libigl using: include(External-libigl)
#       for now, just clone libigl into a directory adjacent to dSpaceX

find_package(LIBIGL REQUIRED)  #QUIET)

SET(SHAREDGP_HEADER_FILES
  SharedGP.h
)

SET(SHAREDGP_SOURCE_FILES
  SharedGP.cpp
)

ADD_LIBRARY(sharedgp ${SHAREDGP_HEADER_FILES} ${SHAREDGP_SOURCE_FILES})
TARGET_LINK_LIBRARIES(sharedgp igl::core)
