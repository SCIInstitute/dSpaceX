include(ExternalProjectCommon)

#---------------------------------------------------------------------------
# Get and build libigl (which also brings in Eigen3)

set(LIBIGL_SOURCE_DIR "${EXTERNAL_DIR}/libigl")
set(LIBIGL_INSTALL_DIR "${CMAKE_BINARY_DIR}/libigl")

set(LIBIGL_EIGEN_VERSION 3.3.7 CACHE STRING "Eigen version")
set(DSPACEX_LIBIGL_VERSION v2.1.0 CACHE STRING "libigl version")

# ExternalProject_Add(libigl
#   GIT_REPOSITORY  https://github.com/libigl/libigl.git
# 	GIT_TAG         ${DSPACEX_LIBIGL_VERSION}
#   SOURCE_DIR      ${LIBIGL_SOURCE_DIR}
#   BINARY_DIR      ${LIBIGL_SOURCE_DIR}-build
#   CMAKE_GENERATOR ${gen}
#   CMAKE_ARGS
#     ${ep_common_args}
#     -DLIBIGL_EIGEN_VERSION:STRING=${LIBIGL_EIGEN_VERSION}
#   INSTALL_COMMAND ""
# )

FetchContent_Declare(libigl
  GIT_REPOSITORY  https://github.com/libigl/libigl.git
	GIT_TAG         ${DSPACEX_LIBIGL_VERSION}
  )
FetchContent_MakeAvailable(libigl)
  SOURCE_DIR      ${LIBIGL_SOURCE_DIR}
  BINARY_DIR      ${LIBIGL_SOURCE_DIR}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_args}
    -DLIBIGL_EIGEN_VERSION:STRING=${LIBIGL_EIGEN_VERSION}
  INSTALL_COMMAND ""
)

#<ctc>, this is set by FindLibigl... but do we really want to add FindLIBIGL.cmake? Maybe...
# set(LIBIGL_INCLUDE_DIR ${LIBIGL_SOURCE_DIR}/include)
