# Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
# SPDX-License-Identifier: GPL-3.0-or-later

# FindHDF5 uses NATIVE_COMMAND in separate_arguments, which requires CMake 3.9.
# The RESULTS_VARIABLE argument of execute_process() requires CMake 3.10.
cmake_minimum_required(VERSION 3.10)

# CMake >= 3.19 gives warnings when these policies are not 'NEW'.
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19")
  cmake_policy(SET CMP0074 NEW)
  cmake_policy(SET CMP0110 NEW)
endif()

# Set version number and project name.
set(DP3_VERSION 5.2.0)
if(DP3_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")
  set(DP3_VERSION_MAJOR "${CMAKE_MATCH_1}")
  set(DP3_VERSION_MINOR "${CMAKE_MATCH_2}")
  set(DP3_VERSION_PATCH "${CMAKE_MATCH_3}")
else()
  message(FATAL_ERROR "Failed to parse DP3_VERSION='${DP3_VERSION}'")
endif()

project(DP3 VERSION ${DP3_VERSION})

include(CheckCXXCompilerFlag)

option(BUILD_TESTING "" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

add_compile_options(-Wall -O3)
string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--no-undefined")

option(PORTABLE "Generate portable code" OFF)
if(PORTABLE)
  if(DEFINED TARGET_CPU)
    message(WARNING "You have selected to build PORTABLE binaries. "
                    "TARGET_CPU settings will be ignored.")
    unset(TARGET_CPU CACHE)
  endif()
else()
  if(DEFINED TARGET_CPU)
    add_compile_options(-march=${TARGET_CPU})
  else()
    check_cxx_compiler_flag("-march=native" COMPILER_HAS_MARCH_NATIVE)
    if(COMPILER_HAS_MARCH_NATIVE)
      add_compile_options(-march=native)
    else()
      message(
        WARNING "The compiler doesn't support -march=native for your CPU.")
    endif()
  endif()
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add CPack directory if user want to generate Debian packages
option(BUILD_PACKAGES "Build Debian packages" OFF)
if(BUILD_PACKAGES)
  add_subdirectory(CPack)
endif()

# === Load external packages ===

# DDECal dependencies
find_package(Armadillo)
if(${ARMADILLO_FOUND})
  add_definitions(-DHAVE_ARMADILLO)
  include_directories(${ARMADILLO_INCLUDE_DIRS})
  set(DDE_ARMADILLO_FILES
      ddecal/constraints/KLFitter.cc ddecal/constraints/PiercePoint.cc
      ddecal/constraints/ScreenConstraint.cc)
else()
  message(
    WARNING "Armadillo was not found, not including screenfitter inside DDECal")
  set(DDE_ARMADILLO_FILES)
endif()

# Casacore depends on HDF5 -> First load HDF5.
find_package(
  HDF5
  COMPONENTS C CXX
  REQUIRED)
add_definitions(${HDF5_DEFINITIONS})
include_directories(${HDF5_INCLUDE_DIRS})

find_package(
  Casacore
  COMPONENTS casa ms tables fits
  REQUIRED)
include_directories(${CASACORE_INCLUDE_DIR})

find_package(CFITSIO REQUIRED)
include_directories(${CFITSIO_INCLUDE_DIRS})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12")
  find_package(
    Python3
    COMPONENTS Interpreter Development
    REQUIRED)
  find_package(Python3 COMPONENTS NumPy)
else() # Use old, deprecated means of detecting python.
  find_package(PythonInterp 3 REQUIRED)
  find_package(PythonLibs 3 REQUIRED)
  set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
  set(Python3_VERSION ${PYTHON_VERSION_STRING})
  set(Python3_VERSION_MAJOR ${PYTHON_VERSION_MAJOR})
  set(Python3_VERSION_MINOR ${PYTHON_VERSION_MINOR})
  set(Python3_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS})
  set(Python3_LIBRARIES ${PYTHON_LIBRARIES})

  execute_process(
    COMMAND "${Python3_EXECUTABLE}" "-c" "import numpy; print(True)"
    OUTPUT_VARIABLE Python3_NumPy_FOUND
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
message(STATUS "Using python version ${Python3_VERSION}")
include_directories(${Python3_INCLUDE_DIRS})

if(BUILD_PACKAGES)
  set(PYTHON_INSTALL_DIR
      lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/dist-packages)
else()
  set(PYTHON_INSTALL_DIR
      lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages)
endif()

# Prevent accidentally finding old BoostConfig.cmake file from casapy
set(Boost_NO_BOOST_CMAKE ON)
find_package(
  Boost
  COMPONENTS filesystem program_options system unit_test_framework
  REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)

# === Load astron packages ===

find_package(AOFlagger 3.1 REQUIRED)
include_directories(${AOFLAGGER_INCLUDE_DIR})

# We could use find_package(EveryBeam 0.1.1 REQUIRED), however conditions below
# make it somewhat more explicit
find_package(EveryBeam NO_MODULE)
if(${EVERYBEAM_FOUND})
  if(${EVERYBEAM_VERSION} VERSION_LESS "0.3.0" OR ${EVERYBEAM_VERSION}
                                                  VERSION_GREATER_EQUAL "0.4.0")
    message(
      FATAL_ERROR
        "DP3 needs EveryBeam version 0.3.x - with x >= 0 - but found version ${EveryBeam_VERSION}"
    )
  endif()
  include_directories(${EVERYBEAM_INCLUDE_DIR})
else(${EVERYBEAM_FOUND})
  message(
    FATAL_ERROR
      "DP3 requires EveryBeam, but EveryBeam was not found. "
      "Please install https://git.astron.nl/RD/EveryBeam and make sure that "
      "EveryBeam is added to the CMAKE_PREFIX_PATH")
endif(${EVERYBEAM_FOUND})

find_package(IDGAPI NO_MODULE QUIET)
if(IDGAPI_FOUND)
  # Throw error if IDG version < 0.8 or version not provided at all
  if((IDGAPI_VERSION VERSION_LESS "0.8") OR (NOT DEFINED IDGAPI_VERSION))
    message(
      FATAL_ERROR
        "IDGAPI was found, but DP3 requires IDGAPI to have version >= 0.8. "
        "Please compile IDG repository at a version >= 0.8")
  endif()
endif()
if(IDGAPI_LIBRARIES AND IDGAPI_INCLUDE_DIRS)
  include_directories(${IDGAPI_INCLUDE_DIRS})
  set(HAVE_IDG TRUE)
  add_definitions(-DHAVE_IDG)
  message(STATUS "Image domain gridder API libraries found.")
else(IDGAPI_LIBRARIES AND IDGAPI_INCLUDE_DIRS)
  set(IDGAPI_LIBRARIES "")
  message(
    WARNING
      "Image domain gridder API libraries NOT found. IDG prediction will not be available."
  )
endif(IDGAPI_LIBRARIES AND IDGAPI_INCLUDE_DIRS)

# === Load internal submodule packages. ===

# Update submodules as needed
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  option(GIT_SUBMODULE "Update submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Syncing submodules")
    # Account for potential changes in git repo URL's
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule sync --recursive
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "Syncing submodules did not succeed"
          "command '${GIT_EXECUTABLE} submodule sync --recursive' failed with exit code ${GIT_SUBMOD_RESULT}"
      )
    endif()
    message(STATUS "Updating submodules")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --checkout
              --depth 1
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endif()
endif()

# Include aocommon
include_directories("${CMAKE_SOURCE_DIR}/external/aocommon/include/")

# Include pybind11
set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/pybind11")
include_directories(${pybind11_INCLUDE_DIR})

# Include schaapcommon
add_subdirectory("${CMAKE_SOURCE_DIR}/external/schaapcommon")
include_directories("${CMAKE_SOURCE_DIR}/external/schaapcommon/include")

# Add cmake information to headers
configure_file(base/Version.h.in base/Version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/base)

# The following section will set the "rpath" correctly, so that
# LD_LIBRARY_PATH doesn't have to be set.

# Include GNUInstallDirs for CMAKE_INSTALL_FULL_LIBDIR
include(GNUInstallDirs)
# Use, i.e. don't skip the full RPATH for the build tree.
set(CMAKE_SKIP_BUILD_RPATH FALSE)
# When building, don't use the install RPATH already
# (but later on when installing).
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
# Add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# The RPATH to be used when installing, but only if it's not a system directory.
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_FULL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")

add_library(
  Blob OBJECT
  blob/BlobAipsIO.cc
  blob/BlobArray.cc
  blob/BlobHeader.cc
  blob/BlobIBufStream.cc
  blob/BlobIStream.cc
  blob/BlobOBufStream.cc
  blob/BlobOStream.cc)

add_library(
  Common OBJECT
  common/BaselineSelect.cc
  common/ClusterDesc.cc
  common/DataConvert.cc
  common/Memory.cc
  common/NodeDesc.cc
  common/ParameterHandler.cc
  common/ParameterRecord.cc
  common/ParameterSet.cc
  common/ParameterSetImpl.cc
  common/ParameterValue.cc
  common/PrettyUnits.cc
  common/ProximityClustering.cc
  common/StringTools.cc
  common/TypeNames.cc
  common/VdsDesc.cc
  common/VdsMaker.cc
  common/VdsPartDesc.cc)

add_library(
  DDECal OBJECT
  steps/BdaDdeCal.cc
  steps/DDECal.cc
  ddecal/Settings.cc
  ddecal/SolutionResampler.cc
  ddecal/SolutionWriter.cc
  ddecal/SolverFactory.cc
  ddecal/constraints/RotationAndDiagonalConstraint.cc
  ddecal/constraints/RotationConstraint.cc
  ddecal/constraints/SmoothnessConstraint.cc
  ddecal/constraints/TECConstraint.cc
  ddecal/gain_solvers/BdaSolverBuffer.cc
  ddecal/gain_solvers/DiagonalSolver.cc
  ddecal/gain_solvers/FullJonesSolver.cc
  ddecal/gain_solvers/HybridSolver.cc
  ddecal/gain_solvers/IterativeDiagonalSolver.cc
  ddecal/gain_solvers/IterativeFullJonesSolver.cc
  ddecal/gain_solvers/IterativeScalarSolver.cc
  ddecal/gain_solvers/ScalarSolver.cc
  ddecal/gain_solvers/SolveData.cc
  ddecal/gain_solvers/SolverBase.cc
  ddecal/gain_solvers/SolverBuffer.cc
  ddecal/linear_solvers/LLSSolver.cc
  ${DDE_ARMADILLO_FILES})

add_library(
  DP3_OBJ OBJECT
  base/Apply.cc
  base/BaselineSelection.cc
  base/BDABuffer.cc
  base/CalType.cc
  base/DemixInfo.cc
  base/DemixWorker.cc
  base/DPBuffer.cc
  base/DPInfo.cc
  base/DPLogger.cc
  base/DP3.cc
  base/EstimateMixed.cc
  base/EstimateMixedLBFGS.cc
  base/EstimateNew.cc
  base/FlagCounter.cc
  base/GainCalAlgorithm.cc
  base/GaussianSource.cc
  base/ModelComponent.cc
  base/ModelComponentVisitor.cc
  base/MS.cc
  base/Patch.cc
  base/PhaseFitter.cc
  base/PointSource.cc
  base/ProgressMeter.cc
  base/Simulate.cc
  base/Simulator.cc
  base/SolutionInterval.cc
  base/SourceDBUtil.cc
  base/Stokes.cc
  base/SubtractMixed.cc
  base/SubtractNew.cc
  base/UVWCalculator.cc
  steps/AOFlaggerStep.cc
  steps/ApplyCal.cc
  steps/Averager.cc
  steps/BDAAverager.cc
  steps/BDAExpander.cc
  steps/BdaGroupPredict.cc
  steps/ColumnReader.cc
  steps/Counter.cc
  steps/Demixer.cc
  steps/DummyStep.cc
  steps/Filter.cc
  steps/GainCal.cc
  steps/H5ParmPredict.cc
  steps/IDGPredict.cc
  steps/InputStep.cc
  steps/Interpolate.cc
  steps/MedFlagger.cc
  steps/MSBDAReader.cc
  steps/MSBDAWriter.cc
  steps/MSReader.cc
  steps/MSUpdater.cc
  steps/MSWriter.cc
  steps/MultiMSReader.cc
  steps/OneApplyCal.cc
  steps/PhaseShift.cc
  steps/Predict.cc
  steps/PreFlagger.cc
  steps/OnePredict.cc
  steps/ScaleData.cc
  steps/SetBeam.cc
  steps/Split.cc
  steps/StationAdder.cc
  steps/Step.cc
  steps/Upsample.cc
  steps/UVWFlagger.cc
  steps/ApplyBeam.cc
  steps/DemixerNew.cc)

add_library(
  ParmDB OBJECT
  parmdb/Axis.cc
  parmdb/AxisMapping.cc
  parmdb/Box.cc
  parmdb/Grid.cc
  parmdb/Parm.cc
  parmdb/ParmCache.cc
  parmdb/ParmDB.cc
  parmdb/ParmDBBlob.cc
  parmdb/ParmDBCasa.cc
  parmdb/ParmDBLocker.cc
  parmdb/ParmDBMeta.cc
  parmdb/ParmFacade.cc
  parmdb/ParmFacadeLocal.cc
  parmdb/ParmFacadeRep.cc
  parmdb/ParmSet.cc
  parmdb/ParmValue.cc
  parmdb/PatchInfo.cc
  parmdb/SkymodelToSourceDB.cc
  parmdb/SourceData.cc
  parmdb/SourceDB.cc
  parmdb/SourceDBBlob.cc
  parmdb/SourceDBCasa.cc
  parmdb/SourceDBSkymodel.cc
  parmdb/SourceInfo.cc)

add_library(PythonDP3 OBJECT pythondp3/PyStep.cc)

# dp3_testdyndp3 requires using position independent code.
set_property(TARGET Blob Common DDECal DP3_OBJ ParmDB PythonDP3
             PROPERTY POSITION_INDEPENDENT_CODE ON)

set(DP3_OBJECTS
    $<TARGET_OBJECTS:Blob> $<TARGET_OBJECTS:Common> $<TARGET_OBJECTS:DDECal>
    $<TARGET_OBJECTS:DP3_OBJ> $<TARGET_OBJECTS:ParmDB>
    $<TARGET_OBJECTS:PythonDP3>)

set(LIBDIRAC_PREFIX
    ""
    CACHE FILEPATH "Path to libdirac install root")
if(NOT "${LIBDIRAC_PREFIX}" STREQUAL "")
  #Find pkg-config and try to find libdirac (only if -DLIBDIRAC_PREFIX given)
  find_package(PkgConfig REQUIRED)
  # Set search path to install directory of libdirac
  set(ENV{PKG_CONFIG_PATH} "${LIBDIRAC_PREFIX}/lib/pkgconfig")
  pkg_search_module(LIBDIRAC libdirac)
endif()
if(LIBDIRAC_FOUND)
  message("-- Found libdirac: ${LIBDIRAC_INCLUDE_DIRS}")
  include_directories(${LIBDIRAC_INCLUDE_DIRS})
endif()

set(DP3_LIBRARIES
    ${AOFLAGGER_LIB}
    ${ARMADILLO_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CASACORE_LIBRARIES}
    ${CFITSIO_LIBRARY}
    ${EVERYBEAM_LIB}
    ${HDF5_LIBRARIES}
    ${IDGAPI_LIBRARIES}
    ${Python3_LIBRARIES}
    schaapcommon
    Threads::Threads
    pybind11::embed)

# If  libdirac is found, use it
if(LIBDIRAC_FOUND)
  # add preprocessor def
  add_definitions(-DHAVE_LIBDIRAC)
  # add link flags
  set(DP3_LIBRARIES ${DP3_LIBRARIES} ${LIBDIRAC_LIBRARIES} ${LIBDIRAC_LDFLAGS})
endif()

# Perform the BLAS check from aocommon
include("${CMAKE_SOURCE_DIR}/external/aocommon/CMake/CheckBLAS.cmake")
check_blas(LIBRARIES ${DP3_LIBRARIES})

add_subdirectory(base)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_subdirectory(pythondp3)
endif()

set(SOURCEDB_OBJECTS $<TARGET_OBJECTS:Blob> $<TARGET_OBJECTS:Common>
                     $<TARGET_OBJECTS:ParmDB>)

set(SOURCEDB_LIBRARIES ${CASACORE_LIBRARIES} ${Boost_SYSTEM_LIBRARY})

add_executable(makesourcedb parmdb/makesourcedb.cc ${SOURCEDB_OBJECTS})
target_link_libraries(makesourcedb ${SOURCEDB_LIBRARIES})

add_executable(showsourcedb parmdb/showsourcedb.cc ${SOURCEDB_OBJECTS})
target_link_libraries(showsourcedb ${SOURCEDB_LIBRARIES})

add_executable(msoverview base/msoverview.cc)
target_link_libraries(msoverview ${CASACORE_LIBRARIES})

install(TARGETS makesourcedb showsourcedb msoverview DESTINATION bin)

# Install a script that warns users that DP3 is the new name of the executable
install(
  PROGRAMS scripts/DPPP-deprecation.sh
  DESTINATION bin
  RENAME DPPP)

add_subdirectory(docs)

if(BUILD_TESTING)
  include(CTest)

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_SPECIFIC_TESTS) # No specific tests for Apple
  else()
    # These run only on Linux
    set(OS_SPECIFIC_TESTS # steps/dynamic_test_step/test/tDynamicTestStep.cc #
                          # This test still fails
    )
    add_library(dp3_testdyndp3 SHARED steps/dynamic_test_step/DynamicTestStep.cc
                                      ${DP3_OBJECTS})
    target_link_libraries(dp3_testdyndp3 ${DP3_LIBRARIES})
    add_dependencies(dp3_testdyndp3 schaapcommon)
  endif()

  set(TEST_FILENAMES
      common/test/unit/fixtures/fSkymodel.cc
      common/test/unit/tProximityClustering.cc
      common/test/unit/tTimer.cc
      base/test/runtests.cc
      base/test/unit/tBaselineSelection.cc
      base/test/unit/tBDABuffer.cc
      base/test/unit/tDPBuffer.cc
      # base/test/unit/tDemixer.cc # Parset is no longer valid in this test
      base/test/unit/tDP3.cc
      base/test/unit/tMirror.cc
      base/test/unit/tSimulate.cc
      base/test/unit/tSolutionInterval.cc
      base/test/unit/tSourceDBUtil.cc
      base/test/unit/tUvwCalculator.cc
      ddecal/test/unit/SolverTester.cc
      ddecal/test/unit/tBdaSolverBuffer.cc
      ddecal/test/unit/tLinearSolvers.cc
      ddecal/test/unit/tLLSSolver.cc
      ddecal/test/unit/tRotationConstraint.cc
      ddecal/test/unit/tSmoothnessConstraint.cc
      ddecal/test/unit/tSolverFactory.cc
      ddecal/test/unit/tSolvers.cc
      steps/test/unit/mock/MockInput.cc
      steps/test/unit/mock/MockStep.cc
      steps/test/unit/tAOFlaggerStep.cc
      steps/test/unit/tApplyCal.cc
      steps/test/unit/tApplyCalH5.cc
      steps/test/unit/tAverager.cc
      steps/test/unit/tBdaDdeCal.cc
      steps/test/unit/tBdaPredict.cc
      steps/test/unit/tBDAResultStep.cc
      steps/test/unit/tDDECal.cc
      steps/test/unit/tFilter.cc
      steps/test/unit/tGainCal.cc
      steps/test/unit/tInterpolate.cc
      steps/test/unit/tMedFlagger.cc
      steps/test/unit/tMSReader.cc
      steps/test/unit/tPreFlagger.cc
      steps/test/unit/tPhaseShift.cc
      steps/test/unit/tOnePredict.cc
      steps/test/unit/tUpsample.cc
      steps/test/unit/tUVWFlagger.cc
      steps/test/unit/tPSet.cc
      steps/test/unit/tScaleData.cc
      steps/test/unit/tStationAdder.cc
      steps/test/unit/tStepCommon.cc)
  if(${Python3_NumPy_FOUND})
    list(APPEND TEST_FILENAMES "steps/test/unit/tPyStep.cc")
  else(${Python3_NumPy_FOUND})
    message(
      WARNING "NumPy not found on machine, tPyStep test is excluded from tests."
    )
  endif()

  # Boost 1.59 introduced BOOST_TEST. The tests below use this feature.
  if(Boost_VERSION_STRING VERSION_GREATER_EQUAL "1.59")
    list(
      APPEND
      TEST_FILENAMES
      common/test/unit/tMemory.cc
      common/test/unit/tStringTools.cc
      base/test/unit/tDPInfo.cc
      base/test/unit/tSimulator.cc
      ddecal/test/unit/tSolutionResampler.cc
      ddecal/test/unit/tSolveData.cc
      parmdb/test/unit/tSkymodelToSourceDB.cc
      parmdb/test/unit/tSourceDB.cc
      steps/test/unit/tBDAAverager.cc
      steps/test/unit/tBDAExpander.cc
      steps/test/unit/tCounter.cc
      steps/test/unit/tInputStep.cc
      steps/test/unit/tMSBDAReader.cc
      steps/test/unit/tMSBDAWriter.cc
      steps/test/unit/tScaleDataBDA.cc)
    if(HAVE_IDG)
      list(APPEND TEST_FILENAMES steps/test/unit/tIDGPredict.cc)
    endif()
  else()
    message(WARNING "Boost < 1.59 detected. Some unit tests are disabled.")
  endif()

  # Add boost dynamic link flag for all test files.
  # https://www.boost.org/doc/libs/1_66_0/libs/test/doc/html/boost_test/usage_variants.html
  # Without this flag, linking is incorrect and boost performs duplicate
  # delete() calls after running all tests, in the cleanup phase.
  set_source_files_properties(
    ${TEST_FILENAMES} PROPERTIES COMPILE_DEFINITIONS "BOOST_TEST_DYN_LINK")

  set(DP3_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
  set(EXTRACT_CMD ${CMAKE_COMMAND} -E tar xzf)

  add_custom_target(
    extract_test_resources
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tApplyCal_tmp.parmdb.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tIDGPredict.sources.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tNDPPP.in_MS.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tNDPPP_bda.in_MS.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tNDPPP-generic.MS.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tOSKAR.in_MS.tgz
    COMMAND ${EXTRACT_CMD} ${DP3_RESOURCE_DIR}/tDDECal.in_MS.tgz)

  add_test(NAME extract_resources
           COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
                   extract_test_resources)
  set_tests_properties(extract_resources PROPERTIES FIXTURES_SETUP
                                                    extract_resources)

  add_executable(unittests ${TEST_FILENAMES} ${OS_SPECIFIC_TESTS}
                           ${DP3_OBJECTS})
  set_target_properties(unittests PROPERTIES ENABLE_EXPORTS ON)
  target_link_libraries(unittests ${DP3_LIBRARIES})
  add_dependencies(unittests schaapcommon)

  # Automatically (re)build the unit tests on every ctest run.
  add_test(buildunittests ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
           unittests)
  set_tests_properties(buildunittests PROPERTIES FIXTURES_SETUP unittests)

  # unittests.sh adjusts the PYTHONPATH to make tPyStep working.
  configure_file(scripts/unittests.sh.in unittests.sh)
  add_test(NAME unittests COMMAND unittests.sh -t !@slow -f JUNIT -k
                                  unittests.xml --catch_system_error=yes)
  set_tests_properties(
    unittests PROPERTIES LABELS unit FIXTURES_REQUIRED
                         "unittests;extract_resources" COST 1)

  # Long running tests are labeled 'slow' and use a separate add_test call, so
  # ctest can run them in parallel. Unfortunately there is no easy means of
  # retreiving a list of all slow tests.
  set(SLOW_TESTS
      idgpredict/process
      idgpredict/process_beam
      msbdawriter/process_simple
      msbdawriter/create_default_subtables
      msbdawriter/different_bda_intervals
      solvers/scalar
      solvers/scalar_normaleq
      solvers/diagonal
      solvers/full_jones
      solvers/iterative_scalar
      solvers/iterative_scalar_dd_intervals
      solvers/iterative_diagonal
      solvers/iterative_diagonal_dd_intervals
      solvers/iterative_full_jones
      solvers/iterative_full_jones_dd_intervals
      solvers/hybrid
      solvers/min_iterations)

  foreach(TEST ${SLOW_TESTS})
    string(REPLACE "/" "_" XMLNAME ${TEST})
    set(XMLNAME "unittest_${XMLNAME}.xml")
    add_test(NAME ${TEST} COMMAND unittests -t ${TEST} -f JUNIT -k ${XMLNAME}
                                  --catch_system_error=yes)
    # The COST indicators are merely very rough indicators of the relative run
    # times, where one COST unit (originally) represents 10 seconds of run time.
    # On the gitlab runners on CI, these times may vary a lot between runs.
    set(COST $<IF:${TEST} MATCHES solver,10,1>)
    set_tests_properties(
      ${TEST} PROPERTIES LABELS "unit;slow" FIXTURES_REQUIRED
                         "unittests;extract_resources" COST ${COST})
  endforeach()

  function(add_sh_integration_tests)
    foreach(TEST ${ARGN})
      # Even if the 'source' symbolic link exists (see below), use
      # ${CMAKE_CURRENT_SOURCE_DIR} instead since not all systems
      # support symbolic links.
      add_test(${TEST} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.sh)
      set_tests_properties(${TEST} PROPERTIES LABELS "integration")
    endforeach()
  endfunction()

  execute_process(COMMAND python3 -m pytest OUTPUT_VARIABLE PYTEST_OUTPUT)
  if(NOT ${PYTEST_OUTPUT} MATCHES "no tests ran")
    message(
      FATAL_ERROR
        "Could not run pytest.\n"
        "Please install pytest or disable testing using -DBUILD_TESTING=Off")
  endif()

  # Adds python tests using a separate ctest test for each python test function.
  # The python tests should thus support parallel runs, e.g., each test function
  # should use its own directory.
  # Notes:
  # - The collection of python-based integration tests can be invoked with ctest -L pyintegration
  # - The tPredict.py docstring describes how the tests can be invoked stand-alone.
  function(add_python_integrationtests)
    foreach(TEST ${ARGN})
      # Extract a list of sub-tests using "pytest --co -q".
      # The last two output lines are an empty line followed by
      # "no tests ran in 0.18s" -> Strip those using "head -n -2".
      execute_process(
        COMMAND python3 -m pytest --collect-only -q
                ${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.py
        COMMAND head -n -2
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} RESULTS_VARIABLE
                          LIST_RESULTS
        OUTPUT_VARIABLE LIST_OUTPUT)
      foreach(LIST_RESULT ${LIST_RESULTS})
        if(LIST_RESULT)
          message(
            FATAL_ERROR
              "Could not determine pytest functions in ${TEST}. Result: ${LIST_RESULT} Output:\n${LIST_OUTPUT}"
          )
        endif()
      endforeach()

      # LIST_OUTPUT has one test name at each line.
      # Convert newlines into CMake list separators (";").
      string(REPLACE "\n" ";" PYTESTS ${LIST_OUTPUT})

      foreach(PYTEST ${PYTESTS})
        # PYTEST is of the form: tDDECal.py::test[1-0-complexgain]
        # Recent pytest versions also add the path before tDDECal.py.
        # Remove everything up to and including "::".
        string(REGEX REPLACE ".*${TEST}.py::" "" PYTEST ${PYTEST})
        add_test(
          NAME ${TEST}::${PYTEST}
          COMMAND
            python3 -m pytest --exitfirst -v -k ${PYTEST}
            --junitxml=${CMAKE_BINARY_DIR}/integration_${TEST}_${PYTEST}.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/${TEST}.py)
        set_tests_properties(${TEST}::${PYTEST} PROPERTIES LABELS
                                                           "pyintegration")
      endforeach()
    endforeach()
  endfunction()

  # Add integration tests
  foreach(DIR ddecal parmdb steps)
    set(INTEGRATION_DIR ${DIR}/test/integration)
    set(BINARY_INTEGRATION_DIR ${CMAKE_BINARY_DIR}/${INTEGRATION_DIR})

    file(MAKE_DIRECTORY ${BINARY_INTEGRATION_DIR})
    configure_file(${CMAKE_SOURCE_DIR}/scripts/test/testconfig.py.in
                   ${BINARY_INTEGRATION_DIR}/testconfig.py)
    configure_file(${CMAKE_SOURCE_DIR}/scripts/test/utils.py.in
                   ${BINARY_INTEGRATION_DIR}/utils.py)

    # The 'source' symbolic link simplifies running the tests manually inside
    # ${BINARY_INTEGRATION_DIR}: It allows using 'source/tApplyBeam.sh' instead
    # of '../../../../DP3/steps/test/integration/tApplyBeam.sh. (Using 'RESULT',
    # fatal errors won't occur on systems without symlink support.)
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
      file(
        CREATE_LINK ${CMAKE_SOURCE_DIR}/${INTEGRATION_DIR}
        ${BINARY_INTEGRATION_DIR}/source
        RESULT DUMMY_RESULT
        SYMBOLIC)
    endif()

    add_subdirectory(${INTEGRATION_DIR})
  endforeach()

endif() # BUILD_TESTING
