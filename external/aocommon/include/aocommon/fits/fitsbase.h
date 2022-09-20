#ifndef FITS_IO_CHECKER_H
#define FITS_IO_CHECKER_H

#include <fitsio.h>

#include <sstream>
#include <stdexcept>
#include <string>

namespace aocommon {

/// Base class for reading and writing fits files.
/// Requires the fitsio library.
class FitsBase {
 protected:
  static void checkStatus(int status, const std::string& filename) {
    if (status) {
      /* fits_get_errstatus returns at most 30 characters */
      char err_text[31];
      fits_get_errstatus(status, err_text);
      char err_msg[81];
      std::stringstream errMsg;
      errMsg << "CFITSIO reported error when performing IO on file '"
             << filename << "':" << err_text << " (";
      while (fits_read_errmsg(err_msg)) errMsg << err_msg;
      errMsg << ')';
      throw std::runtime_error(errMsg.str());
    }
  }
  static void checkStatus(int status, const std::string& filename,
                          const std::string& operation) {
    if (status) {
      /* fits_get_errstatus returns at most 30 characters */
      char err_text[31];
      fits_get_errstatus(status, err_text);
      char err_msg[81];
      std::stringstream errMsg;
      errMsg << "During operation " << operation
             << ", CFITSIO reported error when performing IO on file '"
             << filename << "': " << err_text << " (";
      while (fits_read_errmsg(err_msg)) errMsg << err_msg;
      errMsg << ')';
      throw std::runtime_error(errMsg.str());
    }
  }

 public:
  enum Unit { JanskyPerBeam, JanskyPerPixel, Jansky, Kelvin, MilliKelvin };
  static const char* UnitName(Unit unit) {
    switch (unit) {
      case JanskyPerBeam:
        return "Jansky/beam";
      case JanskyPerPixel:
        return "Jansky/pixel";
      case Jansky:
        return "Jansky";
      case Kelvin:
        return "Kelvin";
      case MilliKelvin:
        return "Milli-Kelvin";
    }
    return "";
  }
};
}  // namespace aocommon

#endif
