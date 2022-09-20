# When calling the function, CMAKE_CURRENT_LIST_DIR is from the caller.
set(CHECKBLAS_DIR ${CMAKE_CURRENT_LIST_DIR})

# Define a function for testing OpenBLAS compatibility.
# Arguments:
# LIBRARIES: All libraries that the application normally links to. The test
# application dynamically checks if OpenBLAS is linked or not.
function(check_blas)
  cmake_parse_arguments(ARG "" "" "LIBRARIES" ${ARGN})

  # Create a list with the locations of all imported libraries.
  set(LIBRARY_LOCATIONS)
  foreach(LIB ${ARG_LIBRARIES})
    if(TARGET ${LIB})
      # Only add imported non-interface libraries.
      get_target_property(IMPORTED ${LIB} IMPORTED)
      get_target_property(TYPE ${LIB} TYPE)
      if (IMPORTED AND NOT ${TYPE} STREQUAL "INTERFACE_LIBRARY")
        get_target_property(LOCATION ${LIB} LOCATION)
        list(APPEND LIBRARY_LOCATIONS ${LOCATION})
      endif()
    else()
      list(APPEND LIBRARY_LOCATIONS ${LIB})
    endif()
  endforeach()

  # Run check_openblas_multithreading() from checkblas.h at configure time,
  # which provides an early warning about a wrong OpenBLAS library.
  # The application should also call this function at run time, since it
  # then may use a different OpenBLAS library.
  try_run(RUN_RESULT COMPILE_RESULT
          "${CMAKE_CURRENT_BINARY_DIR}" "${CHECKBLAS_DIR}/checkblas.cpp"
          CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${CHECKBLAS_DIR}/../include"
          LINK_LIBRARIES "dl;${LIBRARY_LOCATIONS}"
          COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
          RUN_OUTPUT_VARIABLE RUN_OUTPUT)
  if (NOT COMPILE_RESULT)
    message(FATAL_ERROR "Error compiling BLAS Check! Compile output:\n${COMPILE_OUTPUT}")
  endif()
  if (RUN_RESULT)
    message(FATAL_ERROR "${RUN_OUTPUT}")
  endif()
  message(STATUS "BLAS check succeeded.")
endfunction()
