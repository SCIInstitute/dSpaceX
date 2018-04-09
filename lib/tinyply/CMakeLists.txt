project(tinyply LANGUAGES CXX)
cmake_minimum_required(VERSION 2.8)

# Compiler requirements
set(PROJECT_VERSION 2.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TINYPLY_HEADER_FILES tinyply.h)
set(TINYPLY_SOURCE_FILES tinyply.cpp)


# Library
add_library(tinyply ${TINYPLY_HEADER_FILES} ${TINYPLY_SOURCE_FILES})
