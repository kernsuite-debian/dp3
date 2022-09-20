#ifndef FITSREADER_H
#define FITSREADER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <cmath>

#include <fitsio.h>

#include "fitsbase.h"
#include "../polarization.h"

#include <casacore/fits/FITS/FITSDateUtil.h>
#include <casacore/casa/Quanta/MVTime.h>
#include <casacore/measures/Measures/MeasConvert.h>

namespace aocommon {

/// Requires the fitsio library.
class FitsReader : public FitsBase {
 public:
  explicit FitsReader(const std::string& filename)
      : FitsReader(filename, true, false) {}
  explicit FitsReader(const std::string& filename, bool checkCType,
                      bool allowMultipleImages = false)
      : _meta(filename, checkCType, allowMultipleImages) {
    initialize();
  }
  FitsReader(const FitsReader& source)
      : _fitsPtr(nullptr), _meta(source._meta) {
    int status = 0;
    fits_open_file(&_fitsPtr, _meta.filename.c_str(), READONLY, &status);
    checkStatus(status, _meta.filename);

    // Move to first HDU
    int hduType;
    fits_movabs_hdu(_fitsPtr, 1, &hduType, &status);
    checkStatus(status, _meta.filename);
    if (hduType != IMAGE_HDU)
      throw std::runtime_error("First HDU is not an image");
  }
  FitsReader(FitsReader&& source)
      : _fitsPtr(source._fitsPtr), _meta(std::move(source._meta)) {
    source._fitsPtr = nullptr;
  }
  ~FitsReader() {
    if (_fitsPtr != nullptr) {
      int status = 0;
      fits_close_file(_fitsPtr, &status);
    }
  }

  FitsReader& operator=(const FitsReader& rhs) {
    if (_fitsPtr != nullptr) {
      int status = 0;
      fits_close_file(_fitsPtr, &status);
      checkStatus(status, _meta.filename);
    }

    if (rhs._fitsPtr != nullptr) {
      int status = 0;
      fits_open_file(&_fitsPtr, _meta.filename.c_str(), READONLY, &status);
      checkStatus(status, _meta.filename);

      // Move to first HDU
      int hduType;
      fits_movabs_hdu(_fitsPtr, 1, &hduType, &status);
      checkStatus(status, _meta.filename);
      if (hduType != IMAGE_HDU)
        throw std::runtime_error("First HDU is not an image");
    }

    return *this;
  }
  FitsReader& operator=(FitsReader&& rhs) {
    if (_fitsPtr != nullptr) {
      int status = 0;
      fits_close_file(_fitsPtr, &status);
      checkStatus(status, _meta.filename);
    }
    _meta = std::move(rhs._meta);
    _fitsPtr = rhs._fitsPtr;
    rhs._fitsPtr = nullptr;

    return *this;
  }

  // template void ReadIndex(float* image, size_t index);
  // template void ReadIndex(double* image, size_t index);

  template <typename NumType>
  void ReadIndex(NumType* image, size_t index) {
    int status = 0;
    int naxis = 0;
    fits_get_img_dim(_fitsPtr, &naxis, &status);
    checkStatus(status, _meta.filename);
    std::vector<long> firstPixel(naxis);
    for (int i = 0; i != naxis; ++i) firstPixel[i] = 1;
    if (naxis > 2) firstPixel[2] = index + 1;

    if (sizeof(NumType) == 8)
      fits_read_pix(_fitsPtr, TDOUBLE, &firstPixel[0],
                    _meta.imgWidth * _meta.imgHeight, 0, image, 0, &status);
    else if (sizeof(NumType) == 4)
      fits_read_pix(_fitsPtr, TFLOAT, &firstPixel[0],
                    _meta.imgWidth * _meta.imgHeight, 0, image, 0, &status);
    else
      throw std::runtime_error("sizeof(NumType)!=8 || 4 not implemented");
    checkStatus(status, _meta.filename);
  }

  template <typename NumType>
  void Read(NumType* image) {
    ReadIndex(image, 0);
  }

  size_t ImageWidth() const { return _meta.imgWidth; }
  size_t ImageHeight() const { return _meta.imgHeight; }

  double PhaseCentreRA() const { return _meta.phaseCentreRA; }
  double PhaseCentreDec() const { return _meta.phaseCentreDec; }

  double PixelSizeX() const { return _meta.pixelSizeX; }
  double PixelSizeY() const { return _meta.pixelSizeY; }

  double PhaseCentreDL() const { return _meta.phaseCentreDL; }
  double PhaseCentreDM() const { return _meta.phaseCentreDM; }

  double Frequency() const { return _meta.frequency; }
  double Bandwidth() const { return _meta.bandwidth; }

  double DateObs() const { return _meta.dateObs; }
  aocommon::PolarizationEnum Polarization() const { return _meta.polarization; }

  FitsBase::Unit Unit() const { return _meta.unit; }

  bool HasBeam() const { return _meta.hasBeam; }
  double BeamMajorAxisRad() const { return _meta.beamMajorAxisRad; }
  double BeamMinorAxisRad() const { return _meta.beamMinorAxisRad; }
  double BeamPositionAngle() const { return _meta.beamPositionAngle; }

  const std::string& TelescopeName() const { return _meta.telescopeName; }
  const std::string& Observer() const { return _meta.observer; }
  const std::string& ObjectName() const { return _meta.objectName; }

  const std::string& Origin() const { return _meta.origin; }
  const std::string& OriginComment() const { return _meta.originComment; }

  const std::vector<std::string>& History() const { return _meta.history; }

  bool ReadDoubleKeyIfExists(const char* key, double& dest) {
    int status = 0;
    double doubleValue;
    fits_read_key(_fitsPtr, TDOUBLE, key, &doubleValue, 0, &status);
    if (status == 0) dest = doubleValue;
    return status == 0;
  }
  bool ReadStringKeyIfExists(const char* key, std::string& dest) {
    std::string c;
    return ReadStringKeyIfExists(key, dest, c);
  }
  bool ReadStringKeyIfExists(const char* key, std::string& value,
                             std::string& comment) {
    int status = 0;
    char valueStr[256], commentStr[256];
    fits_read_key(_fitsPtr, TSTRING, key, valueStr, commentStr, &status);
    if (status == 0) {
      value = valueStr;
      comment = commentStr;
    }
    return status == 0;
  }
  bool ReadFloatKeyIfExists(const char* key, float& dest) {
    int status = 0;
    float floatValue;
    fits_read_key(_fitsPtr, TFLOAT, key, &floatValue, 0, &status);
    if (status == 0) dest = floatValue;
    return status == 0;
  }

  static double ParseFitsDateToMJD(const char* valueStr) {
    casacore::MVTime time;
    casacore::MEpoch::Types systypes;
    bool parseSuccess =
        casacore::FITSDateUtil::fromFITS(time, systypes, valueStr, "UTC");
    if (!parseSuccess)
      throw std::runtime_error(std::string("Could not parse FITS date: ") +
                               valueStr);
    casacore::MEpoch epoch(time.get(), systypes);
    return epoch.getValue().get();
  }

  const std::string& Filename() const { return _meta.filename; }

  fitsfile* FitsHandle() const { return _fitsPtr; }

  size_t NMatrixElements() const { return _meta.nMatrixElements; }
  size_t NFrequencies() const { return _meta.nFrequencies; }
  size_t NAntennas() const { return _meta.nAntennas; }
  size_t NTimesteps() const { return _meta.nTimesteps; }

  double TimeDimensionStart() const { return _meta.timeDimensionStart; }
  double TimeDimensionIncr() const { return _meta.timeDimensionIncr; }

  double FrequencyDimensionStart() const { return _meta.frequency; }
  double FrequencyDimensionIncr() const { return _meta.bandwidth; }

  double ReadDoubleKey(const char* key) {
    int status = 0;
    double value;
    fits_read_key(_fitsPtr, TDOUBLE, key, &value, 0, &status);
    checkStatus(status, _meta.filename, std::string("Read float key ") + key);
    return value;
  }

 private:
  void readHistory() {
    int status = 0;
    int npos, moreKeys;
    fits_get_hdrspace(_fitsPtr, &npos, &moreKeys, &status);
    checkStatus(status, _meta.filename);
    char keyCard[256];
    for (int pos = 1; pos <= npos; ++pos) {
      fits_read_record(_fitsPtr, pos, keyCard, &status);
      keyCard[7] = 0;
      if (std::string("HISTORY") == keyCard) {
        _meta.history.push_back(&keyCard[8]);
      }
    }
  }
  bool readDateKeyIfExists(const char* key, double& dest) {
    int status = 0;
    char keyStr[256];
    fits_read_key(_fitsPtr, TSTRING, key, keyStr, 0, &status);
    if (status == 0) {
      dest = FitsReader::ParseFitsDateToMJD(keyStr);
      return true;
    } else
      return false;
  }

  void initialize() {
    _meta.nMatrixElements = 1;
    _meta.nFrequencies = 1;
    _meta.nAntennas = 1;
    _meta.nTimesteps = 1;
    _meta.phaseCentreRA = 0.0;
    _meta.pixelSizeX = 0.0;
    _meta.phaseCentreDec = 0.0;
    _meta.pixelSizeY = 0.0;
    _meta.dateObs = 0.0;
    _meta.frequency = 0.0;
    _meta.bandwidth = 0.0;
    _meta.polarization = aocommon::Polarization::StokesI;
    _meta.unit = JanskyPerBeam;

    int status = 0;
    fits_open_file(&_fitsPtr, _meta.filename.c_str(), READONLY, &status);
    checkStatus(status, _meta.filename);

    // Move to first HDU
    int hduType;
    fits_movabs_hdu(_fitsPtr, 1, &hduType, &status);
    checkStatus(status, _meta.filename);
    if (hduType != IMAGE_HDU)
      throw std::runtime_error("First HDU is not an image");

    int naxis = 0;
    fits_get_img_dim(_fitsPtr, &naxis, &status);
    checkStatus(status, _meta.filename);
    if (naxis < 2) throw std::runtime_error("NAxis in image < 2");

    std::vector<long> naxes(naxis);
    fits_get_img_size(_fitsPtr, naxis, &naxes[0], &status);
    checkStatus(status, _meta.filename);

    _meta.imgWidth = naxes[0];
    _meta.imgHeight = naxes[1];

    // There are fits files that say naxis=2 but then still define
    // the third and fourth axes, so we always continue reading
    // at least 4 axes:
    if (naxis < 4) {
      naxis = 4;
      while (naxes.size() < 4) naxes.emplace_back(1);
    }

    std::string tmp;
    for (int i = 2; i != naxis; ++i) {
      std::ostringstream name;
      name << "CTYPE" << (i + 1);
      if (ReadStringKeyIfExists(name.str().c_str(), tmp)) {
        std::ostringstream crval, cdelt;
        crval << "CRVAL" << (i + 1);
        cdelt << "CDELT" << (i + 1);
        if (tmp == "FREQ" || tmp == "VRAD" || tmp == "FREQ-OBS") {
          _meta.nFrequencies = naxes[i];
          _meta.frequency = ReadDoubleKey(crval.str().c_str());
          _meta.bandwidth = ReadDoubleKey(cdelt.str().c_str());
        } else if (tmp == "ANTENNA")
          _meta.nAntennas = naxes[i];
        else if (tmp == "TIME") {
          _meta.nTimesteps = naxes[i];
          _meta.timeDimensionStart = ReadDoubleKey(crval.str().c_str());
          _meta.timeDimensionIncr = ReadDoubleKey(cdelt.str().c_str());
        } else if (tmp == "STOKES") {
          double val = ReadDoubleKey(crval.str().c_str());
          switch (int(val)) {
            default:
              throw std::runtime_error(
                  "Unknown polarization specified in fits file");
            case 1:
              _meta.polarization = aocommon::Polarization::StokesI;
              break;
            case 2:
              _meta.polarization = aocommon::Polarization::StokesQ;
              break;
            case 3:
              _meta.polarization = aocommon::Polarization::StokesU;
              break;
            case 4:
              _meta.polarization = aocommon::Polarization::StokesV;
              break;
            case -1:
              _meta.polarization = aocommon::Polarization::RR;
              break;
            case -2:
              _meta.polarization = aocommon::Polarization::LL;
              break;
            case -3:
              _meta.polarization = aocommon::Polarization::RL;
              break;
            case -4:
              _meta.polarization = aocommon::Polarization::LR;
              break;
            case -5:
              _meta.polarization = aocommon::Polarization::XX;
              break;
            case -6:
              _meta.polarization = aocommon::Polarization::YY;
              break;
            case -7:
              _meta.polarization = aocommon::Polarization::XY;
              break;
            case -8:
              _meta.polarization = aocommon::Polarization::YX;
              break;
          }
          if (naxes[i] != 1 && !_meta.allowMultipleImages)
            throw std::runtime_error(
                "Multiple polarizations given in fits file");
        } else if (tmp == "MATRIX") {
          _meta.nMatrixElements = naxes[i];
        } else if (naxes[i] != 1)
          throw std::runtime_error("Multiple images given in fits file");
      }
    }

    if (_meta.nMatrixElements != 1 && !_meta.allowMultipleImages)
      throw std::runtime_error("Multiple matrix elements given in fits file");
    if (_meta.nFrequencies != 1 && !_meta.allowMultipleImages)
      throw std::runtime_error("Multiple frequencies given in fits file");
    if (_meta.nAntennas != 1 && !_meta.allowMultipleImages)
      throw std::runtime_error("Multiple antennas given in fits file");
    if (_meta.nTimesteps != 1 && !_meta.allowMultipleImages)
      throw std::runtime_error("Multiple timesteps given in fits file");

    double bScale = 1.0, bZero = 0.0, equinox = 2000.0;
    ReadDoubleKeyIfExists("BSCALE", bScale);
    ReadDoubleKeyIfExists("BZERO", bZero);
    ReadDoubleKeyIfExists("EQUINOX", equinox);
    if (bScale != 1.0) throw std::runtime_error("Invalid value for BSCALE");
    if (bZero != 0.0) throw std::runtime_error("Invalid value for BZERO");
    if (equinox != 2000.0) {
      std::string str;
      ReadStringKeyIfExists("EQUINOX", str);
      throw std::runtime_error("Invalid value for EQUINOX: " + str);
    }

    if (ReadStringKeyIfExists("CTYPE1", tmp) && tmp != "RA---SIN" &&
        _meta.checkCType)
      throw std::runtime_error("Invalid value for CTYPE1");

    ReadDoubleKeyIfExists("CRVAL1", _meta.phaseCentreRA);
    _meta.phaseCentreRA *= M_PI / 180.0;
    ReadDoubleKeyIfExists("CDELT1", _meta.pixelSizeX);
    _meta.pixelSizeX *= -M_PI / 180.0;
    if (ReadStringKeyIfExists("CUNIT1", tmp) && tmp != "deg" &&
        _meta.checkCType)
      throw std::runtime_error("Invalid value for CUNIT1");
    double centrePixelX = 0.0;
    if (ReadDoubleKeyIfExists("CRPIX1", centrePixelX))
      _meta.phaseCentreDL =
          (centrePixelX - ((_meta.imgWidth / 2.0) + 1.0)) * _meta.pixelSizeX;
    else
      _meta.phaseCentreDL = 0.0;

    if (ReadStringKeyIfExists("CTYPE2", tmp) && tmp != "DEC--SIN" &&
        _meta.checkCType)
      throw std::runtime_error("Invalid value for CTYPE2");
    ReadDoubleKeyIfExists("CRVAL2", _meta.phaseCentreDec);
    _meta.phaseCentreDec *= M_PI / 180.0;
    ReadDoubleKeyIfExists("CDELT2", _meta.pixelSizeY);
    _meta.pixelSizeY *= M_PI / 180.0;
    if (ReadStringKeyIfExists("CUNIT2", tmp) && tmp != "deg" &&
        _meta.checkCType)
      throw std::runtime_error("Invalid value for CUNIT2");
    double centrePixelY = 0.0;
    if (ReadDoubleKeyIfExists("CRPIX2", centrePixelY))
      _meta.phaseCentreDM =
          ((_meta.imgHeight / 2.0) + 1.0 - centrePixelY) * _meta.pixelSizeY;
    else
      _meta.phaseCentreDM = 0.0;

    readDateKeyIfExists("DATE-OBS", _meta.dateObs);

    double bMaj = 0.0, bMin = 0.0, bPa = 0.0;
    if (ReadDoubleKeyIfExists("BMAJ", bMaj) &&
        ReadDoubleKeyIfExists("BMIN", bMin) &&
        ReadDoubleKeyIfExists("BPA", bPa)) {
      _meta.hasBeam = true;
      _meta.beamMajorAxisRad = bMaj * (M_PI / 180.0);
      _meta.beamMinorAxisRad = bMin * (M_PI / 180.0);
      _meta.beamPositionAngle = bPa * (M_PI / 180.0);
    } else {
      _meta.hasBeam = false;
      _meta.beamMajorAxisRad = 0.0;
      _meta.beamMinorAxisRad = 0.0;
      _meta.beamPositionAngle = 0.0;
    }

    _meta.telescopeName = std::string();
    ReadStringKeyIfExists("TELESCOP", _meta.telescopeName);
    _meta.observer = std::string();
    ReadStringKeyIfExists("OBSERVER", _meta.observer);
    _meta.objectName = std::string();
    ReadStringKeyIfExists("OBJECT", _meta.objectName);

    _meta.origin = std::string();
    _meta.originComment = std::string();
    ReadStringKeyIfExists("ORIGIN", _meta.origin, _meta.originComment);

    _meta.history.clear();
    readHistory();
  }

  fitsfile* _fitsPtr;

  struct MetaData {
    MetaData(const std::string& filename_, bool checkCType_,
             bool allowMultipleImages_)
        : filename(filename_),
          hasBeam(false),
          checkCType(checkCType_),
          allowMultipleImages(allowMultipleImages_) {}
    std::string filename;
    size_t imgWidth, imgHeight;
    size_t nMatrixElements, nAntennas, nFrequencies, nTimesteps;
    double phaseCentreRA, phaseCentreDec;
    double pixelSizeX, pixelSizeY;
    double phaseCentreDL, phaseCentreDM;
    double frequency, bandwidth, dateObs;
    bool hasBeam;
    double beamMajorAxisRad, beamMinorAxisRad, beamPositionAngle;
    double timeDimensionStart, timeDimensionIncr;

    aocommon::PolarizationEnum polarization;
    FitsBase::Unit unit;
    std::string telescopeName, observer, objectName;
    std::string origin, originComment;
    std::vector<std::string> history;

    bool checkCType, allowMultipleImages;
  } _meta;
};
}  // namespace aocommon

#endif
