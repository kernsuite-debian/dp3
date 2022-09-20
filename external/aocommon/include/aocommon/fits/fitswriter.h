#ifndef FITSWRITER_H
#define FITSWRITER_H

#include "fitsbase.h"
#include "fitsreader.h"
#include "../polarization.h"
#include "../uvector.h"

#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include <fitsio.h>

namespace aocommon {

/// Requires the fitsio library.
class FitsWriter : public FitsBase {
 public:
  enum DimensionType {
    FrequencyDimension,
    PolarizationDimension,
    AntennaDimension,
    TimeDimension,
    MatrixDimension
  };

  FitsWriter()
      : _width(0),
        _height(0),
        _phaseCentreRA(0.0),
        _phaseCentreDec(0.0),
        _pixelSizeX(0.0),
        _pixelSizeY(0.0),
        _phaseCentreDL(0.0),
        _phaseCentreDM(0.0),
        _frequency(0.0),
        _bandwidth(0.0),
        _dateObs(0.0),
        _hasBeam(false),
        _beamMajorAxisRad(0.0),
        _beamMinorAxisRad(0.0),
        _beamPositionAngle(0.0),
        _polarization(aocommon::Polarization::StokesI),
        _unit(JanskyPerBeam),
        _isUV(false),
        _telescopeName(),
        _observer(),
        _objectName(),
        _origin("AO/WSImager"),
        _originComment("Imager written by Andre Offringa"),
        _multiFPtr(nullptr) {}

  explicit FitsWriter(const class FitsReader& reader)
      : _width(0),
        _height(0),
        _phaseCentreRA(0.0),
        _phaseCentreDec(0.0),
        _pixelSizeX(0.0),
        _pixelSizeY(0.0),
        _phaseCentreDL(0.0),
        _phaseCentreDM(0.0),
        _frequency(0.0),
        _bandwidth(0.0),
        _dateObs(0.0),
        _hasBeam(false),
        _beamMajorAxisRad(0.0),
        _beamMinorAxisRad(0.0),
        _beamPositionAngle(0.0),
        _polarization(aocommon::Polarization::StokesI),
        _unit(JanskyPerBeam),
        _isUV(false),
        _telescopeName(),
        _observer(),
        _objectName(),
        _origin("AO/WSImager"),
        _originComment("Imager written by Andre Offringa"),
        _multiFPtr(nullptr) {
    SetMetadata(reader);
  }

  ~FitsWriter() {
    if (_multiFPtr != nullptr) FinishMulti();
  }

  // template void Write<long double>(const std::string& filename,
  //                                  const long double* image) const;
  // template void Write<double>(const std::string& filename,
  //                             const double* image) const;
  // template void Write<float>(const std::string& filename,
  //                            const float* image) const;

  template <typename NumType>
  void Write(const std::string& filename, const NumType* image) const {
    fitsfile* fptr;

    writeHeaders(fptr, filename);

    long firstPixel[4] = {1, 1, 1, 1};
    writeImage(fptr, filename, image, firstPixel);

    int status = 0;
    fits_close_file(fptr, &status);
    checkStatus(status, filename);
  }

  void WriteMask(const std::string& filename, const bool* mask) const {
    aocommon::UVector<float> maskAsImage(_width * _height);
    for (size_t i = 0; i != _width * _height; ++i)
      maskAsImage[i] = mask[i] ? 1.0 : 0.0;
    Write(filename, maskAsImage.data());
  }

  void StartMulti(const std::string& filename) {
    if (_multiFPtr != 0)
      throw std::runtime_error(
          "StartMulti() called twice without calling FinishMulti()");
    _multiFilename = filename;
    writeHeaders(_multiFPtr, _multiFilename, _extraDimensions);
    _currentPixel.assign(_extraDimensions.size() + 2, 1);
  }

  template <typename NumType>
  void AddToMulti(const NumType* image) {
    if (_multiFPtr == 0)
      throw std::runtime_error("AddToMulti() called before StartMulti()");
    writeImage(_multiFPtr, _multiFilename, image, _currentPixel.data());
    size_t index = 2;
    _currentPixel[index]++;
    while (index < _currentPixel.size() - 1 &&
           _currentPixel[index] > long(_extraDimensions[index - 2].size)) {
      _currentPixel[index] = 1;
      ++index;
      _currentPixel[index]++;
    }
  }

  void FinishMulti() {
    int status = 0;
    fits_close_file(_multiFPtr, &status);
    checkStatus(status, _multiFilename);
    _multiFPtr = 0;
  }

  void SetBeamInfo(double widthRad) { SetBeamInfo(widthRad, widthRad, 0.0); }
  void SetBeamInfo(double majorAxisRad, double minorAxisRad,
                   double positionAngleRad) {
    _hasBeam = true;
    _beamMajorAxisRad = majorAxisRad;
    _beamMinorAxisRad = minorAxisRad;
    _beamPositionAngle = positionAngleRad;
  }
  void SetNoBeamInfo() {
    _hasBeam = false;
    _beamMajorAxisRad = 0.0;
    _beamMinorAxisRad = 0.0;
    _beamPositionAngle = 0.0;
  }
  void SetImageDimensions(size_t width, size_t height) {
    _width = width;
    _height = height;
  }
  void SetImageDimensions(size_t width, size_t height, double pixelSizeX,
                          double pixelSizeY) {
    SetImageDimensions(width, height);
    _pixelSizeX = pixelSizeX;
    _pixelSizeY = pixelSizeY;
  }
  void SetImageDimensions(size_t width, size_t height, double phaseCentreRA,
                          double phaseCentreDec, double pixelSizeX,
                          double pixelSizeY) {
    SetImageDimensions(width, height, pixelSizeX, pixelSizeY);
    _phaseCentreRA = phaseCentreRA;
    _phaseCentreDec = phaseCentreDec;
  }
  void SetFrequency(double frequency, double bandwidth) {
    _frequency = frequency;
    _bandwidth = bandwidth;
  }
  void SetDate(double dateObs) { _dateObs = dateObs; }
  void SetPolarization(aocommon::PolarizationEnum polarization) {
    _polarization = polarization;
  }
  Unit GetUnit() const { return _unit; }
  void SetUnit(Unit unit) { _unit = unit; }
  void SetIsUV(bool isUV) { _isUV = isUV; }
  void SetTelescopeName(const std::string& telescopeName) {
    _telescopeName = telescopeName;
  }
  void SetObserver(const std::string& observer) { _observer = observer; }
  void SetObjectName(const std::string& objectName) {
    _objectName = objectName;
  }
  void SetOrigin(const std::string& origin, const std::string& comment) {
    _origin = origin;
    _originComment = comment;
  }
  void SetHistory(const std::vector<std::string>& history) {
    _history = history;
  }
  void AddHistory(const std::string& historyLine) {
    _history.push_back(historyLine);
  }

  void SetMetadata(const FitsReader& reader) {
    _width = reader.ImageWidth();
    _height = reader.ImageHeight();
    _phaseCentreRA = reader.PhaseCentreRA();
    _phaseCentreDec = reader.PhaseCentreDec();
    _pixelSizeX = reader.PixelSizeX();
    _pixelSizeY = reader.PixelSizeY();
    _frequency = reader.Frequency();
    _bandwidth = reader.Bandwidth();
    _dateObs = reader.DateObs();
    _polarization = reader.Polarization();
    _hasBeam = reader.HasBeam();
    if (_hasBeam) {
      _beamMajorAxisRad = reader.BeamMajorAxisRad();
      _beamMinorAxisRad = reader.BeamMinorAxisRad();
      _beamPositionAngle = reader.BeamPositionAngle();
    }
    _phaseCentreDL = reader.PhaseCentreDL();
    _phaseCentreDM = reader.PhaseCentreDM();
    _telescopeName = reader.TelescopeName();
    _observer = reader.Observer();
    _objectName = reader.ObjectName();
    _origin = reader.Origin();
    _originComment = reader.OriginComment();
    _history = reader.History();
  }

  /**
   * @return double Right ascension of phase centre (rad)
   */
  double RA() const { return _phaseCentreRA; }

  /**
   * @return double Declination of phase centre (rad)
   */
  double Dec() const { return _phaseCentreDec; }

  /**
   * @return double Pixel size in x-direction (rad)
   */
  double PixelSizeX() const { return _pixelSizeX; }

  /**
   * @return double Pixel size in y-direction (rad)
   */
  double PixelSizeY() const { return _pixelSizeY; }

  double Frequency() const { return _frequency; }
  double Bandwidth() const { return _bandwidth; }
  double BeamSizeMajorAxis() const { return _beamMajorAxisRad; }
  double BeamSizeMinorAxis() const { return _beamMinorAxisRad; }
  double BeamPositionAngle() const { return _beamPositionAngle; }

  void SetExtraKeyword(const std::string& name, const std::string& value) {
    if (_extraStringKeywords.count(name) != 0) _extraStringKeywords.erase(name);
    _extraStringKeywords.insert(std::make_pair(name, value));
  }
  void SetExtraKeyword(const std::string& name, double value) {
    if (_extraNumKeywords.count(name) != 0) _extraNumKeywords.erase(name);
    _extraNumKeywords.insert(std::make_pair(name, value));
  }
  void RemoveExtraKeyword(const std::string& name) {
    if (_extraNumKeywords.count(name) != 0) _extraNumKeywords.erase(name);
    if (_extraStringKeywords.count(name) != 0) _extraStringKeywords.erase(name);
  }
  void SetExtraStringKeywords(
      const std::map<std::string, std::string>& keywords) {
    _extraStringKeywords = keywords;
  }
  void SetExtraNumKeywords(const std::map<std::string, double>& keywords) {
    _extraNumKeywords = keywords;
  }
  void SetPhaseCentreShift(double dl, double dm) {
    _phaseCentreDL = dl;
    _phaseCentreDM = dm;
  }
  size_t Width() const { return _width; }
  size_t Height() const { return _height; }
  double PhaseCentreDL() const { return _phaseCentreDL; }
  double PhaseCentreDM() const { return _phaseCentreDM; }

  void CopyDoubleKeywordIfExists(FitsReader& reader, const char* keywordName) {
    double v;
    if (reader.ReadDoubleKeyIfExists(keywordName, v))
      SetExtraKeyword(keywordName, v);
  }

  void CopyStringKeywordIfExists(FitsReader& reader, const char* keywordName) {
    std::string v;
    if (reader.ReadStringKeyIfExists(keywordName, v))
      SetExtraKeyword(keywordName, v);
  }

  static void MJDToHMS(double mjd, int& hour, int& minutes, int& seconds,
                       int& deciSec) {
    // It might seem one can calculate each of these immediately
    // without adjusting 'mjd', but this way circumvents some
    // catastrophic rounding problems, where "0:59.9" might end up
    // as "1:59.9".
    deciSec = int(fmod(mjd * 36000.0 * 24.0, 10.0));
    mjd -= double(deciSec) / (36000.0 * 24.0);

    seconds = int(fmod(round(mjd * 3600.0 * 24.0), 60.0));
    mjd -= double(seconds) / (3600.0 * 24.0);

    minutes = int(fmod(round(mjd * 60.0 * 24.0), 60.0));
    mjd -= double(minutes) / (60.0 * 24.0);

    hour = int(fmod(round(mjd * 24.0), 24.0));
  }

  void AddExtraDimension(enum DimensionType type, size_t size) {
    _extraDimensions.emplace_back(Dimension{type, size});
  }
  void SetTimeDirectionStart(double time) { _timeDirectionStart = time; }
  void SetTimeDirectionInc(double dTime) { _timeDirectionInc = dTime; }

 private:
  struct Dimension {
    DimensionType type;
    size_t size;
  };

  template <typename T>
  static T setNotFiniteToZero(T num) {
    return std::isfinite(num) ? num : 0.0;
  }
  std::size_t _width, _height;
  double _phaseCentreRA, _phaseCentreDec, _pixelSizeX, _pixelSizeY;
  double _phaseCentreDL, _phaseCentreDM;
  double _frequency, _bandwidth;
  double _dateObs;
  bool _hasBeam;
  double _beamMajorAxisRad, _beamMinorAxisRad, _beamPositionAngle;
  aocommon::PolarizationEnum _polarization;
  Unit _unit;
  bool _isUV;
  std::string _telescopeName, _observer, _objectName;
  std::string _origin, _originComment;
  std::vector<std::string> _history;
  std::vector<Dimension> _extraDimensions;
  std::map<std::string, std::string> _extraStringKeywords;
  std::map<std::string, double> _extraNumKeywords;
  double _timeDirectionStart, _timeDirectionInc;

  void julianDateToYMD(double jd, int& year, int& month, int& day) const {
    int z = jd + 0.5;
    int w = (z - 1867216.25) / 36524.25;
    int x = w / 4;
    int a = z + 1 + w - x;
    int b = a + 1524;
    int c = (b - 122.1) / 365.25;
    int d = 365.25 * c;
    int e = (b - d) / 30.6001;
    int f = 30.6001 * e;
    day = b - d - f;
    while (e - 1 > 12) e -= 12;
    month = e - 1;
    year = c - 4715 - ((e - 1) > 2 ? 1 : 0);
  }

  void writeHeaders(fitsfile*& fptr, const std::string& filename) const {
    if (_extraDimensions.empty()) {
      std::vector<Dimension> dimensions(2);
      dimensions[0].type = FrequencyDimension;
      dimensions[0].size = 1;
      dimensions[1].type = PolarizationDimension;
      dimensions[1].size = 1;
      writeHeaders(fptr, filename, dimensions);
    } else {
      writeHeaders(fptr, filename, _extraDimensions);
    }
  }

  void writeHeaders(fitsfile*& fptr, const std::string& filename,
                    const std::vector<Dimension>& extraDimensions) const {
    int status = 0;
    fits_create_file(&fptr, (std::string("!") + filename).c_str(), &status);
    checkStatus(status, filename);

    // append image HDU
    int bitPixInt = FLOAT_IMG;
    std::vector<long> naxes(2 + extraDimensions.size());
    naxes[0] = _width;
    naxes[1] = _height;
    for (size_t i = 0; i != extraDimensions.size(); ++i)
      naxes[i + 2] = extraDimensions[i].size;
    fits_create_img(fptr, bitPixInt, naxes.size(), naxes.data(), &status);
    checkStatus(status, filename);
    double zero = 0, one = 1, equinox = 2000.0;
    fits_write_key(fptr, TDOUBLE, "BSCALE", (void*)&one, "", &status);
    checkStatus(status, filename);
    fits_write_key(fptr, TDOUBLE, "BZERO", (void*)&zero, "", &status);
    checkStatus(status, filename);

    switch (_unit) {
      default:
      case JanskyPerBeam:
        fits_write_key(fptr, TSTRING, "BUNIT", (void*)"JY/BEAM",
                       "Units are in Jansky per beam", &status);
        checkStatus(status, filename);
        break;
      case JanskyPerPixel:
        fits_write_key(fptr, TSTRING, "BUNIT", (void*)"JY/PIXEL",
                       "Units are in Jansky per pixel", &status);
        checkStatus(status, filename);
        break;
      case Jansky:
        fits_write_key(fptr, TSTRING, "BUNIT", (void*)"JY",
                       "Units are in Jansky", &status);
        checkStatus(status, filename);
        break;
      case Kelvin:
        fits_write_key(fptr, TSTRING, "BUNIT", (void*)"K",
                       "Units are in Kelvin", &status);
        checkStatus(status, filename);
        break;
      case MilliKelvin:
        fits_write_key(fptr, TSTRING, "BUNIT", (void*)"mK",
                       "Units are in milli Kelvin", &status);
        checkStatus(status, filename);
        break;
    }

    if (_hasBeam) {
      double majDeg = setNotFiniteToZero(_beamMajorAxisRad * 180.0 / M_PI),
             minDeg = setNotFiniteToZero(_beamMinorAxisRad * 180.0 / M_PI),
             posAngle = setNotFiniteToZero(_beamPositionAngle * 180.0 / M_PI);
      fits_write_key(fptr, TDOUBLE, "BMAJ", (void*)&majDeg, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "BMIN", (void*)&minDeg, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "BPA", (void*)&posAngle, "", &status);
      checkStatus(status, filename);
    }

    fits_write_key(fptr, TDOUBLE, "EQUINOX", (void*)&equinox, "J2000", &status);
    checkStatus(status, filename);
    fits_write_key(fptr, TSTRING, "BTYPE", (void*)"Intensity", "", &status);
    checkStatus(status, filename);
    if (!_telescopeName.empty()) {
      fits_write_key(fptr, TSTRING, "TELESCOP", (void*)_telescopeName.c_str(),
                     "", &status);
      checkStatus(status, filename);
    }
    if (!_observer.empty()) {
      fits_write_key(fptr, TSTRING, "OBSERVER", (void*)_observer.c_str(), "",
                     &status);
      checkStatus(status, filename);
    }
    if (!_objectName.empty()) {
      fits_write_key(fptr, TSTRING, "OBJECT", (void*)_objectName.c_str(), "",
                     &status);
      checkStatus(status, filename);
    }
    fits_write_key(fptr, TSTRING, "ORIGIN", (void*)_origin.c_str(),
                   _originComment.c_str(), &status);
    checkStatus(status, filename);
    double phaseCentreRADeg = (_phaseCentreRA / M_PI) * 180.0,
           phaseCentreDecDeg = (_phaseCentreDec / M_PI) * 180.0;
    double centrePixelX =
               _pixelSizeX != 0.0
                   ? ((_width / 2.0) + 1.0 + _phaseCentreDL / _pixelSizeX)
                   : (_width / 2.0) + 1.0,
           centrePixelY =
               _pixelSizeY != 0.0
                   ? ((_height / 2.0) + 1.0 - _phaseCentreDM / _pixelSizeY)
                   : (_height / 2.0) + 1.0;
    if (_isUV) {
      double deltX, deltY;
      if (_pixelSizeX == 0.0 || _pixelSizeY == 0.0) {
        deltX = 1.0;
        deltY = 1.0;
      } else {
        deltX = 1.0 / (_width * _pixelSizeX);
        deltY = 1.0 / (_height * _pixelSizeY);
      }
      fits_write_key(fptr, TSTRING, "CTYPE1", (void*)"U---WAV",
                     "U axis of UV plane", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRPIX1", (void*)&centrePixelX, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRVAL1", (void*)&zero, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CDELT1", (void*)&deltX, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TSTRING, "CUNIT1", (void*)"lambda", "", &status);
      checkStatus(status, filename);

      fits_write_key(fptr, TSTRING, "CTYPE2", (void*)"V---WAV",
                     "V axis of UV plane", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRPIX2", (void*)&centrePixelY, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRVAL2", (void*)&zero, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CDELT2", (void*)&deltY, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TSTRING, "CUNIT2", (void*)"lambda", "", &status);
      checkStatus(status, filename);
    } else {
      double stepXDeg = (-_pixelSizeX / M_PI) * 180.0,
             stepYDeg = (_pixelSizeY / M_PI) * 180.0;
      fits_write_key(fptr, TSTRING, "CTYPE1", (void*)"RA---SIN",
                     "Right ascension angle cosine", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRPIX1", (void*)&centrePixelX, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRVAL1", (void*)&phaseCentreRADeg, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CDELT1", (void*)&stepXDeg, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TSTRING, "CUNIT1", (void*)"deg", "", &status);
      checkStatus(status, filename);

      fits_write_key(fptr, TSTRING, "CTYPE2", (void*)"DEC--SIN",
                     "Declination angle cosine", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRPIX2", (void*)&centrePixelY, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CRVAL2", (void*)&phaseCentreDecDeg, "",
                     &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TDOUBLE, "CDELT2", (void*)&stepYDeg, "", &status);
      checkStatus(status, filename);
      fits_write_key(fptr, TSTRING, "CUNIT2", (void*)"deg", "", &status);
      checkStatus(status, filename);
    }

    char ctypeDim[7] = "CTYPE?", crpixDim[7] = "CRPIX?", crvalDim[7] = "CRVAL?",
         cdeltDim[7] = "CDELT?", cunitDim[7] = "CUNIT?";
    for (size_t i = 0; i != extraDimensions.size(); ++i) {
      ctypeDim[5] = (i + '3');
      crpixDim[5] = (i + '3');
      crvalDim[5] = (i + '3');
      cdeltDim[5] = (i + '3');
      cunitDim[5] = (i + '3');
      switch (extraDimensions[i].type) {
        case FrequencyDimension:
          fits_write_key(fptr, TSTRING, ctypeDim, (void*)"FREQ",
                         "Central frequency", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crpixDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crvalDim, (void*)&_frequency, "",
                         &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, cdeltDim, (void*)&_bandwidth, "",
                         &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TSTRING, cunitDim, (void*)"Hz", "", &status);
          checkStatus(status, filename);
          break;
        case PolarizationDimension: {
          double pol;
          switch (_polarization) {
            case aocommon::Polarization::StokesI:
              pol = 1.0;
              break;
            case aocommon::Polarization::StokesQ:
              pol = 2.0;
              break;
            case aocommon::Polarization::StokesU:
              pol = 3.0;
              break;
            case aocommon::Polarization::StokesV:
              pol = 4.0;
              break;
            case aocommon::Polarization::RR:
              pol = -1.0;
              break;
            case aocommon::Polarization::LL:
              pol = -2.0;
              break;
            case aocommon::Polarization::RL:
              pol = -3.0;
              break;
            case aocommon::Polarization::LR:
              pol = -4.0;
              break;
            case aocommon::Polarization::XX:
              pol = -5.0;
              break;
            case aocommon::Polarization::YY:
              pol = -6.0;
              break;  // yup, this is really the right value
            case aocommon::Polarization::XY:
              pol = -7.0;
              break;
            case aocommon::Polarization::YX:
              pol = -8.0;
              break;
            case aocommon::Polarization::FullStokes:
            case aocommon::Polarization::Instrumental:
              throw std::runtime_error(
                  "Incorrect polarization given to fits writer");
          }
          fits_write_key(fptr, TSTRING, ctypeDim, (void*)"STOKES", "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crpixDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crvalDim, (void*)&pol, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, cdeltDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TSTRING, cunitDim, (void*)"", "", &status);
          checkStatus(status, filename);
        } break;
        case AntennaDimension:
          fits_write_key(fptr, TSTRING, ctypeDim, (void*)"ANTENNA", "",
                         &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crpixDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crvalDim, (void*)&zero, "", &status);
          checkStatus(status, filename);
          break;
        case TimeDimension:
          fits_write_key(fptr, TSTRING, ctypeDim, (void*)"TIME", "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crpixDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crvalDim, (void*)&_timeDirectionStart,
                         "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, cdeltDim, (void*)&_timeDirectionInc, "",
                         &status);
          checkStatus(status, filename);
          break;
        case MatrixDimension:
          fits_write_key(fptr, TSTRING, ctypeDim, (void*)"MATRIX", "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crpixDim, (void*)&one, "", &status);
          checkStatus(status, filename);
          fits_write_key(fptr, TDOUBLE, crvalDim, (void*)&zero, "", &status);
          checkStatus(status, filename);
          break;
      }
    }

    // RESTFRQ ?
    fits_write_key(fptr, TSTRING, "SPECSYS", (void*)"TOPOCENT", "", &status);
    checkStatus(status, filename);

    int year, month, day, hour, min, sec, deciSec;
    julianDateToYMD(_dateObs + 2400000.5, year, month, day);
    MJDToHMS(_dateObs, hour, min, sec, deciSec);
    char dateStr[40];
    std::sprintf(dateStr, "%d-%02d-%02dT%02d:%02d:%02d.%01d", year, month, day,
                 hour, min, sec, deciSec);
    fits_write_key(fptr, TSTRING, "DATE-OBS", (void*)dateStr, "", &status);
    checkStatus(status, filename);

    // Extra keywords
    for (std::map<std::string, std::string>::const_iterator i =
             _extraStringKeywords.begin();
         i != _extraStringKeywords.end(); ++i) {
      const char* name = i->first.c_str();
      char* valueStr = const_cast<char*>(i->second.c_str());
      fits_write_key(fptr, TSTRING, name, valueStr, "", &status);
      checkStatus(status, filename);
    }
    for (std::map<std::string, double>::const_iterator i =
             _extraNumKeywords.begin();
         i != _extraNumKeywords.end(); ++i) {
      const char* name = i->first.c_str();
      double value = setNotFiniteToZero(i->second);
      fits_write_key(fptr, TDOUBLE, name, (void*)&value, "", &status);
      checkStatus(status, filename);
    }

    // History
    std::ostringstream histStr;
    for (std::vector<std::string>::const_iterator i = _history.begin();
         i != _history.end(); ++i) {
      fits_write_history(fptr, i->c_str(), &status);
      checkStatus(status, filename);
    }
  }
  void writeImage(fitsfile* fptr, const std::string& filename,
                  const double* image, long* currentPixel) const {
    double nullValue = std::numeric_limits<double>::max();
    int status = 0;
    fits_write_pixnull(fptr, TDOUBLE, currentPixel, _width * _height,
                       const_cast<double*>(image), &nullValue, &status);
    checkStatus(status, filename);
  }
  void writeImage(fitsfile* fptr, const std::string& filename,
                  const float* image, long* currentPixel) const {
    float nullValue = std::numeric_limits<float>::max();
    int status = 0;
    fits_write_pixnull(fptr, TFLOAT, currentPixel, _width * _height,
                       const_cast<float*>(image), &nullValue, &status);
    checkStatus(status, filename);
  }

  template <typename NumType>
  void writeImage(fitsfile* fptr, const std::string& filename,
                  const NumType* image, long* currentPixel) const {
    double nullValue = std::numeric_limits<double>::max();
    int status = 0;
    size_t totalSize = _width * _height;
    std::vector<double> copy(totalSize);
    for (size_t i = 0; i != totalSize; ++i) copy[i] = image[i];
    fits_write_pixnull(fptr, TDOUBLE, currentPixel, totalSize, &copy[0],
                       &nullValue, &status);
    checkStatus(status, filename);
  }

  std::string _multiFilename;
  fitsfile* _multiFPtr;
  std::vector<long> _currentPixel;
};
}  // namespace aocommon

#endif
