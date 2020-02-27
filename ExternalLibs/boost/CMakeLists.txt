########################################################
#
#  This is a CMake configuration file.
#  To use it you need CMake which can be
#  downloaded from here:
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

# The interesting parts of Boost have been extracted using
# the BCP tool:
#   http://www.boost.org/doc/libs/1_45_0/tools/bcp/doc/html/index.html
#
# bcp call: bcp boost/filesystem.hpp

# WARNING: When doing this from fresh boost sources, note that
# in the file /boost/config/user.hpp the BOOST_ALL_NO_LIB
# define has been uncommented. Otherwise, we will get linker
# errors on MSVC because of boost auto-linking.

PROJECT(boostparts)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# project version
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)

# project options
OPTION(BUILD_SHARED_LIBS OFF)

# copy header files over
SET(Boost_INSTALLED_HEADER_FILES)
FOREACH(header_file ${Boost_API_HEADER_FILES})
  # When debugging, uncomment this line
  #MESSAGE(STATUS "Copy header file: ${header_file}")
  GET_FILENAME_COMPONENT(file_no_path "${header_file}" NAME)
  CONFIGURE_FILE("${header_file}"
    "${CMAKE_BINARY_DIR}/include/boost/${file_no_path}" COPYONLY IMMEDIATE)
  SET(Boost_INSTALLED_HEADER_FILES
    ${Boost_INSTALLED_HEADER_FILES}
    "${CMAKE_BINARY_DIR}/include/boost/${file_no_path}")
ENDFOREACH(header_file)

SET(Boost_API_HEADER_FILES
    cmdline.hpp
    config.hpp
    environment_iterator.hpp
    ../eof_iterator.hpp
    ../errors.hpp
    ../option.hpp
    ../options_description.hpp
    ../parsers.hpp
    ../positional_options.hpp
    ../value_semantic.hpp
    ../variables_map.hpp
    ../version.hpp
    detail/cmdline.hpp
    detail/config_file.hpp
    detail/convert.hpp
    detail/parsers.hpp
    detail/utf8_codecvt_facet.hpp
    detail/value_semantic.hpp
    )



# source files
SET(SOURCE_FILES
    libs/filesystem/src/codecvt_error_category.cpp
    libs/filesystem/src/operations.cpp
    libs/filesystem/src/path.cpp
    libs/filesystem/src/path_traits.cpp
    libs/filesystem/src/portability.cpp
    libs/filesystem/src/unique_path.cpp
    libs/filesystem/src/utf8_codecvt_facet.cpp
    libs/filesystem/src/windows_file_codecvt.cpp
    libs/filesystem/src/windows_file_codecvt.hpp
    libs/system/src/error_code.cpp
   )

#FILE(GLOB HEADER_FILES ../*.hpp)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

#add_definitions(-DBOOST_ALL_NO_LIB)
# output library
ADD_LIBRARY(boostparts  ${SOURCE_FILES} ${HEADER_FILES})
SET_TARGET_PROPERTIES(boostparts PROPERTIES LINKER_LANGUAGE CXX)
MESSAGE(STATUS "creating library {boost}")

# Special compiler and linker flags for MSVC
if( MSVC )
    MESSAGE(STATUS "Applying MSVC flags for Boost")
    add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
    add_definitions(-DBOOST_ALL_NO_LIB)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" )
    set_target_properties( boostparts PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
