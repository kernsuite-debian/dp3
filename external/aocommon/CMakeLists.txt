cmake_minimum_required(VERSION 2.8.8)

project(aocommon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -ggdb")

# add target to generate API documentation with Doxygen
find_package(Threads REQUIRED)
find_package(Doxygen)

if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                 ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(
    doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif(DOXYGEN_FOUND)

find_package(
  Boost
  COMPONENTS unit_test_framework
  REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Casacore has a separate CMake file in this directory
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
set(CASACORE_MAKE_REQUIRED_EXTERNALS_OPTIONAL TRUE)
find_package(Casacore REQUIRED COMPONENTS casa ms tables measures fits)
include_directories(${CASACORE_INCLUDE_DIRS})

find_package(CFITSIO REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIR})

add_executable(
  runtests
  tests/runtests.cpp
  tests/tbanddata.cpp
  tests/tfits.cpp
  tests/thmatrix4x4.cpp
  tests/tmatrix2x2.cpp
  tests/tmatrix2x2diag.cpp
  tests/tmatrix4x4.cpp
  tests/tparallelfor.cpp
  tests/tqueue.cpp
  tests/tradeccoord.cpp
  tests/tstaticfor.cpp
  tests/tserialstream.cpp
  tests/tthreadpool.cpp
  tests/tuvector.cpp)
target_link_libraries(
  runtests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}
  ${CASACORE_LIBRARIES} ${CFITSIO_LIBRARY})

add_custom_target(execute_runtests_target ALL DEPENDS execute_runtests)
add_custom_command(
  OUTPUT execute_runtests
  COMMAND runtests
  DEPENDS runtests)

add_custom_target(
  coverage
  COMMAND gcovr -r .. -e '.*/tests/.*' -e '.*/CompilerIdCXX/.*'
  COMMAND gcovr -r .. -e '.*/tests/.*' -e '.*/CompilerIdCXX/.*' --xml >
          coverage.xml
  DEPENDS execute_runtests)

message(STATUS "Flags passed to C++ compiler: " ${CMAKE_CXX_FLAGS})
