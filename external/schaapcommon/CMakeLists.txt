# Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
# SPDX-License-Identifier: GPL-3.0-or-later

# Top level CMakeLists.txt file for SchaapCommon
cmake_minimum_required(VERSION 3.7)

# Define a couple of dedicated variables, to prevent that standard CMAKE
# variables get screwed up when schaapcommon included in another package
set(SCHAAPCOMMON_PROJECT_NAME schaapcommon)
set(SCHAAPCOMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(${SCHAAPCOMMON_PROJECT_NAME})

option(BUILD_TESTING "Build tests" OFF)

set(CMAKE_MODULE_PATH ${SCHAAPCOMMON_SOURCE_DIR}/cmake
                      ${SCHAAPCOMMON_SOURCE_DIR}/cmake/external)

# CMP0063 is needed to hide symbols in static libraries
cmake_policy(SET CMP0063 NEW)
# Suppress warnings related to ROOT variables
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

# Convenience conditional to distinguish between stand-alone vs dependency builds
set(SCHAAPCOMMON_MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(SCHAAPCOMMON_MASTER_PROJECT ON)
endif()

if(SCHAAPCOMMON_MASTER_PROJECT)
  # Require the AOCOMMON headers when compiled in standalone.
  set(AOCOMMON_INCLUDE_DIR
      ""
      CACHE FILEPATH "Path to aocommon include")
  if("${AOCOMMON_INCLUDE_DIR}" STREQUAL "")
    message(
      FATAL_ERROR
        "AOCOMMON_INCLUDE_DIR is not defined!\n"
        "Please use 'cmake -DAOCOMMON_INCLUDE_DIR=<aocommon include path>'")
  endif()
  if(NOT EXISTS ${AOCOMMON_INCLUDE_DIR}/aocommon/uvector.h)
    message(
      FATAL_ERROR
        "Can't find uvector.h in AOCOMMON_INCLUDE_DIR (\"${AOCOMMON_INCLUDE_DIR}\")"
    )
  endif()
  include_directories(${AOCOMMON_INCLUDE_DIR})

  if(BUILD_TESTING)
    include(CTest)
    find_package(
      Boost
      COMPONENTS unit_test_framework
      REQUIRED)
  endif()
endif()

# Compile in release mode if schaapcommon is included from external
# dependency
if(NOT SCHAAPCOMMON_MASTER_PROJECT)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Spawn all compiler warnings
add_compile_options(-Wall)

# Load dependencies, could be done a bit neater with CMP0079, i.e.
# cmake_policy(SET CMP0079 NEW)
# but requires CMake >= 3.13

# Find and include HDF5
find_package(
  HDF5
  COMPONENTS C CXX
  REQUIRED)
add_definitions(${HDF5_DEFINITIONS})
include_directories(${HDF5_INCLUDE_DIR})

# Temporary include casacore until this is replaced with aocommon::MC2x2
find_package(
  Casacore
  COMPONENTS casa
  REQUIRED)
include_directories(${CASACORE_INCLUDE_DIR})

# Find and include Boost (geometry) headers
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Boost_VERSION has 0's instead of periods for older versions of boost
string(REPLACE 0 . Boost_VERSION_TMP ${Boost_VERSION})
if(Boost_VERSION_TMP VERSION_LESS "1.66")
  add_definitions(-DHAVE_BOOST_LT_166)
endif()

# Let schaapcommon be a STATIC library, to reduce the risk on linking problems
add_library(schaapcommon STATIC "")
target_link_libraries(${SCHAAPCOMMON_PROJECT_NAME} ${HDF5_LIBRARIES}
                      ${HDF5_CXX_LIBRARIES} ${CASACORE_LIBRARIES})

if(SCHAAPCOMMON_MASTER_PROJECT)
  # Install targets
  install(
    TARGETS schaapcommon
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
endif()

# Source directories
add_subdirectory(src/facets)
add_subdirectory(src/h5parm)
