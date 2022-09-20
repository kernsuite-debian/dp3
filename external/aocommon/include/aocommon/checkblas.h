#ifndef AOCOMMON_CHECKBLAS_H
#define AOCOMMON_CHECKBLAS_H

#include <cstdio>
#include <cstddef>
#include <dlfcn.h>
#include <stdexcept>
#include <string>

/**
 * Detect if OpenBLAS is linked to the application and determine the version
 * number.
 * @return The encoded version number, for example 30712 for version 3.7.12.
 * @retval -1 If OpenBLAS is not linked.
 * @retval -2 If the OpenBLAS version could not be determined.
 */
inline int get_openblas_version() {
  // Ask the dynamic linker to lookup the openblas_get_config() function.
  char* (*openblas_get_config)(void) = reinterpret_cast<char* (*)(void)>(
      dlsym(RTLD_DEFAULT, "openblas_get_config"));
  if (!openblas_get_config) return -1;

  // The config should contain a version string like "OpenBLAS 0.3.18"
  // Old OpenBLAS versions do not have a version in their config.
  int major = 0;
  int minor = 0;
  int patch = 0;
  const int assigned = std::sscanf(openblas_get_config(), "OpenBLAS %d.%d.%d",
                                   &major, &minor, &patch);
  if (assigned != 3) return -2;

  return major * 10000 + minor * 100 + patch;
}

/**
 * Checks if the application is dynamically linked to OpenBLAS. If it is,
 * checks if the OpenBLAS library is properly configured.
 *
 * @throws runtime_error When the application is linked to an OpenBLAS library
 *         that is not supported or incorrectly configured.
 */
inline void check_openblas_multithreading() {
  // Ask the dynamic linker to lookup the openblas_get_parallel() function.
  int (*openblas_get_parallel)(void) = reinterpret_cast<int (*)(void)>(
      dlsym(RTLD_DEFAULT, "openblas_get_parallel"));
  // If the lookup failed, the application does not use OpenBLAS, so there is
  // no need for further checks.
  if (!openblas_get_parallel) return;

  const std::string please =
      "Please use a multi-threaded OpenBLAS version but configure it to use a "
      "single\n"
      "thread by setting the OPENBLAS_NUM_THREADS environment variable "
      "to 1.";

  switch (openblas_get_parallel()) {
    case 0: {
      // The executable is linked to a single threaded version of OpenBLAS.
      const int version = get_openblas_version();

      if (version >= 0 && version < 307) {
        // Before 0.3.7, OpenBLAS certainly does not support locking.
        throw std::runtime_error(
            "This software was linked to a single-threaded version of OpenBLAS "
            "with a version before 0.3.7, which does not support "
            "multi-threaded environments.\n" +
            please);
      } else {
        // OpenBLAS may support locking, but typically does not.
        // Unfortunately there is no means of checking if OpenBLAS supports
        // locking, so assume that the library is bad.
        throw std::runtime_error(
            "This software was linked to a single-threaded version of "
            "OpenBLAS, which typically does not support multi-threaded "
            "environments.\n" +
            please);
      }

      break;
    }
    case 1: {
      // The executable is linked to a multithreaded version of OpenBLAS.
      // Read the OPENBLAS_NUM_THREADS environment variable
      int openblas_num_threads = 0;
      char* openblas_num_threads_env_var = getenv("OPENBLAS_NUM_THREADS");
      if (openblas_num_threads_env_var != nullptr) {
        openblas_num_threads = atoi(openblas_num_threads_env_var);
      }

      if (openblas_num_threads != 1) {
        // TODO: Fix the problem by calling openblas_set_num_threads(1),
        // resetting thread affinity. aocommon::NCPU will then return the
        // correct value again instead of 1. Then detect whether OpenMP is
        // used (IDG uses OpenMP) and then somehow reinitialize OpenMP
        // such that not all its threads are bound to CPU 0. But for now
        // throw an error and ask the user to fix the problem

        throw std::runtime_error(
            "This software was linked to a multi-threaded version of OpenBLAS. "
            "OpenBLAS multi-threading interferes with other multi-threaded "
            "parts of the code, which has a severe impact on performance. "
            "Please disable OpenBLAS multi-threading by setting the "
            "environment variable OPENBLAS_NUM_THREADS to 1. For csh like "
            "shells, use:  setenv OPENBLAS_NUM_THREADS 1 For bash like shells, "
            "use: export OPENBLAS_NUM_THREADS=1");
      }
      break;
    }
    case 2:
      throw std::runtime_error(
          "This software was linked to an OpenMP version of OpenBLAS, which is "
          "not supported.\n" +
          please);
      break;
    default:
      throw std::runtime_error(
          "This software was linked to an OpenBLAS version with an unknown "
          "multi-threading mode, which is not supported.\n" +
          please);
      break;
  }
}

#endif  // AOCOMMON_CHECKBLAS_H
