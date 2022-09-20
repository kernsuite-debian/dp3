# Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
# SPDX-License-Identifier: GPL-3.0-or-later

get_filename_component(MODULE ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(PUBLIC_HEADER_DIR ${SCHAAPCOMMON_SOURCE_DIR}/include/schaapcommon/${MODULE})

set(PUBLIC_HEADERS
    ${PUBLIC_HEADER_DIR}/h5parm.h ${PUBLIC_HEADER_DIR}/jonesparameters.h
    ${PUBLIC_HEADER_DIR}/soltab.h)

target_sources(
  ${SCHAAPCOMMON_PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/h5parm.cc
          ${CMAKE_CURRENT_SOURCE_DIR}/jonesparameters.cc
          ${CMAKE_CURRENT_SOURCE_DIR}/soltab.cc
          ${CMAKE_CURRENT_SOURCE_DIR}/gridinterpolate.cc)

# Simplifies including the public headers.
target_include_directories(${SCHAAPCOMMON_PROJECT_NAME}
                           PRIVATE "$<BUILD_INTERFACE:${PUBLIC_HEADER_DIR}>")

# Install headers and add test directory when built as stand-alone.
if(SCHAAPCOMMON_MASTER_PROJECT)
  install(FILES ${PUBLIC_HEADERS}
          DESTINATION "include/${CMAKE_PROJECT_NAME}/${MODULE}")

  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
