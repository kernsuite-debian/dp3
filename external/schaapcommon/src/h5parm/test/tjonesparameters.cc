// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include "jonesparameters.h"
#include "h5parm.h"

using schaapcommon::h5parm::AxisInfo;
using schaapcommon::h5parm::H5Parm;
using schaapcommon::h5parm::JonesParameters;
using schaapcommon::h5parm::SolTab;

const std::vector<double> kFreqs{130e6, 131e6};
const std::vector<double> kTimes{0., 1.};
const unsigned int kNAnts = 4;
const JonesParameters::InterpolationType kInterpolationType =
    JonesParameters::InterpolationType::LINEAR;
const unsigned long long kDirection = 42;

class SolTabMock : public SolTab {
 public:
  SolTabMock() : called(0) {}
  std::vector<double> GetValuesOrWeights(const std::string& val_or_weight,
                                         const std::string& ant_name,
                                         const std::vector<double>& times,
                                         const std::vector<double>& freqs,
                                         unsigned int pol, unsigned int dir,
                                         bool nearest) override {
    ++called;
    auto res = std::vector<double>(kNAnts, 200.);

    if (ant_name.back() - '0' >= int(kNAnts)) {
      // Number represented by last character of ant_name is >= kNants
      // E.g. an antenna 'Antenna5' is requested which is not in the soltab
      throw(std::runtime_error("SolTab has no element Antenna" +
                               std::string(1, ant_name.back()) +
                               " in antenna"));
    }

    if (val_or_weight == "val") {
      res.back() = 100.;
    } else {
      res.back() = 0.;
    }
    return res;
  }

  int called;
};

JonesParameters PrepareJonesParameters(JonesParameters::CorrectType ct,
                                       bool invert = false,
                                       float sigma_mmse = 0.,
                                       unsigned int parm_size = 0) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();
  SolTabMock mock2 = SolTabMock();

  JonesParameters jones(kFreqs, kTimes, antNames, ct, kInterpolationType,
                        kDirection, &mock, &mock2, invert, sigma_mmse,
                        parm_size);
  return jones;
}

BOOST_AUTO_TEST_SUITE(jonesparameters)

BOOST_AUTO_TEST_CASE(check_parse_list) {
  std::vector<std::string> list0 = JonesParameters::ParseList("abc,def");
  BOOST_CHECK_EQUAL(list0[0], "abc");
  BOOST_CHECK_EQUAL(list0[1], "def");
  BOOST_CHECK_THROW(std::vector<std::string> list1 =
                        JonesParameters::ParseList("abc,def,ghi"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(make_complex_gain) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::GAIN);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_EQUAL(parms.shape()[1], kNAnts);
  BOOST_CHECK_EQUAL(parms.shape()[2], kTimes.size() * kFreqs.size());
  // Amplitude and phase are 200
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 97.437, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -174.659, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_scalar_gain) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::SCALARGAIN);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_EQUAL(parms.shape()[1], kNAnts);
  BOOST_CHECK_EQUAL(parms.shape()[2], kTimes.size() * kFreqs.size());
  // Amplitude and phase are 200
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 97.437, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -174.659, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_fulljones) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::FULLJONES, true, 1.);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 4);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 0.5006091, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0.00108991, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_tec) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::TEC, false, 0., 1U);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), -0.993747, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0.1116511, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_tec2) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::TEC, true, 0., 2U);
  const auto parms = jones.GetParms();

  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), -0.993747, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -0.1116511, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_r_angle) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::ROTATIONANGLE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 4);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 0.487187, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0., 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_r_angle_inverted) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::ROTATIONANGLE, true);
  const auto parms = jones.GetParms();

  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 0.487187, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0.0, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_r_measure) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::ROTATIONMEASURE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 4);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), -0.185403, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0., 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_r_measure_inverted) {
  JonesParameters jones = PrepareJonesParameters(
      JonesParameters::CorrectType::ROTATIONMEASURE, true, 0.5, 4);
  const auto parms = jones.GetParms();

  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), -0.185403, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0.0, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_phase) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::PHASE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 0.487187, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -0.87329, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_scalar_phase) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::SCALARPHASE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 0.487187, 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -0.87329, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_amplitude) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::AMPLITUDE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 200., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0., 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_scalar_amplitude) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::SCALARAMPLITUDE);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 200., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 0., 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_clock) {
  JonesParameters jones = PrepareJonesParameters(
      JonesParameters::CorrectType::CLOCK, false, 0., 1U);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 1., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 2.08822e-06, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_clock2) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::CLOCK, true, 50, 2U);
  const auto parms = jones.GetParms();

  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 1., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), -2.08822e-06, 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_gain_re_im) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::GAIN_RE_IM);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 200., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 200., 1e-3);
}

BOOST_AUTO_TEST_CASE(make_complex_fulljones_re_im) {
  JonesParameters jones =
      PrepareJonesParameters(JonesParameters::CorrectType::FULLJONES_RE_IM);
  const auto parms = jones.GetParms();

  BOOST_CHECK_EQUAL(parms.shape()[0], 4);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).real(), 200., 1e-3);
  BOOST_CHECK_CLOSE(parms(0, 0, 0).imag(), 200., 1e-3);
}

BOOST_AUTO_TEST_CASE(fulljones_with_nullptr) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();

  BOOST_CHECK_THROW(
      JonesParameters jones(kFreqs, kTimes, antNames,
                            JonesParameters::CorrectType::FULLJONES,
                            kInterpolationType, kDirection, &mock, nullptr),
      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(missing_antenna_error) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts + 1; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();

  BOOST_CHECK_THROW(
      JonesParameters jones(kFreqs, kTimes, antNames,
                            JonesParameters::CorrectType::AMPLITUDE,
                            kInterpolationType, kDirection, &mock, nullptr),
      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(missing_antenna_flag) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts + 1; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();

  JonesParameters jones(kFreqs, kTimes, antNames,
                        JonesParameters::CorrectType::AMPLITUDE,
                        kInterpolationType, kDirection, &mock, nullptr, false,
                        0, 0, JonesParameters::MissingAntennaBehavior::kFlag);

  const auto parms = jones.GetParms();
  BOOST_CHECK_EQUAL(parms.shape()[1], kNAnts + 1);
  BOOST_CHECK(std::isfinite(parms(0, kNAnts - 1, 0).real()));
  BOOST_CHECK(!std::isfinite(parms(0, kNAnts, 0).real()));
}

BOOST_AUTO_TEST_CASE(missing_antenna_unit_diag) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts + 1; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();

  JonesParameters jones(kFreqs, kTimes, antNames,
                        JonesParameters::CorrectType::AMPLITUDE,
                        kInterpolationType, kDirection, &mock, nullptr, false,
                        0, 0, JonesParameters::MissingAntennaBehavior::kUnit);

  const auto parms = jones.GetParms();
  BOOST_CHECK_EQUAL(parms.shape()[0], 2);
  BOOST_CHECK_EQUAL(parms.shape()[1], kNAnts + 1);
  BOOST_CHECK_EQUAL(parms(0, kNAnts, 0).real(), 1.);
  BOOST_CHECK_EQUAL(parms(1, kNAnts, 0).real(), 1.);
  BOOST_CHECK_EQUAL(parms(0, kNAnts, 0).imag(), 0.);
}

BOOST_AUTO_TEST_CASE(missing_antenna_unit_full) {
  std::vector<std::string> antNames;
  for (unsigned int i = 0; i < kNAnts + 1; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
  }

  SolTabMock mock = SolTabMock();

  JonesParameters jones(kFreqs, kTimes, antNames,
                        JonesParameters::CorrectType::ROTATIONANGLE,
                        kInterpolationType, kDirection, &mock, nullptr, false,
                        0, 0, JonesParameters::MissingAntennaBehavior::kUnit);

  const auto parms = jones.GetParms();
  BOOST_CHECK_EQUAL(parms.shape()[0], 4);
  BOOST_CHECK_EQUAL(parms.shape()[1], kNAnts + 1);
  BOOST_CHECK_EQUAL(parms(0, kNAnts, 0).real(), 1.);
  BOOST_CHECK_EQUAL(parms(0, kNAnts, 0).imag(), 0.);
  BOOST_CHECK_EQUAL(parms(1, kNAnts, 0).real(), 0.);
  BOOST_CHECK_EQUAL(parms(1, kNAnts, 0).imag(), 0.);
  BOOST_CHECK_EQUAL(parms(2, kNAnts, 0).real(), 0.);
  BOOST_CHECK_EQUAL(parms(2, kNAnts, 0).imag(), 0.);
  BOOST_CHECK_EQUAL(parms(3, kNAnts, 0).real(), 1.);
  BOOST_CHECK_EQUAL(parms(3, kNAnts, 0).imag(), 0.);
}

BOOST_AUTO_TEST_CASE(correcttype_to_str) {
  std::vector<std::string> correction_names = {
      "gain",        "fulljones",       "tec",           "clock",
      "scalarphase", "scalaramplitude", "rotationangle", "rotationmeasure",
      "phase",       "amplitude"};

  for (const std::string& correction_name : correction_names) {
    JonesParameters::CorrectType correction_type =
        JonesParameters::StringToCorrectType(correction_name);
    std::string correction_name_back =
        JonesParameters::CorrectTypeToString(correction_type);
    BOOST_CHECK_EQUAL(correction_name, correction_name_back);
  }
  BOOST_CHECK_THROW(JonesParameters::StringToCorrectType("nosuchcorrection"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
