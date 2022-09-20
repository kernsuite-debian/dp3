// Copyright (C) 2021 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jonesparameters.h"

#include <casacore/casa/BasicSL/Complex.h>
#include <casacore/casa/BasicSL/Constants.h>

#include <cassert>

#include <aocommon/matrix2x2.h>

#include <boost/algorithm/string.hpp>

namespace schaapcommon {
namespace h5parm {
JonesParameters::JonesParameters(
    const std::vector<double>& freqs, const std::vector<double>& times,
    const std::vector<std::string>& antenna_names, CorrectType correct_type,
    InterpolationType interpolation_type, unsigned long long direction,
    std::vector<std::vector<std::vector<double>>>&& parm_values, bool invert,
    float sigma_mmse)
    : parm_values_(parm_values) {
  const unsigned int num_parms = GetNParms(correct_type);
  parms_.resize(num_parms, antenna_names.size(), freqs.size() * times.size());
  for (size_t ant = 0; ant < antenna_names.size(); ++ant) {
    MakeComplex(ant, freqs, correct_type, invert);
  }
  if (invert) {
    Invert(parms_, sigma_mmse, correct_type);
  }
}

JonesParameters::JonesParameters(
    const std::vector<double>& freqs, const std::vector<double>& times,
    const std::vector<std::string>& antenna_names, CorrectType correct_type,
    InterpolationType interpolation_type, unsigned long long direction,
    schaapcommon::h5parm::SolTab* sol_tab,
    schaapcommon::h5parm::SolTab* sol_tab2, bool invert, float sigma_mmse,
    unsigned int parm_size, MissingAntennaBehavior missing_antenna_behavior) {
  const unsigned int num_parms = GetNParms(correct_type);

  parms_.resize(num_parms, antenna_names.size(), freqs.size() * times.size());
  if (parm_size == 0U) {
    parm_size = GetNParmValues(correct_type);
  }
  parm_values_.resize(parm_size);
  for (auto& parm_values : parm_values_) {
    parm_values.resize(antenna_names.size());
  }
  for (size_t ant = 0; ant < antenna_names.size(); ++ant) {
    try {
      FillParmValues(sol_tab, sol_tab2, freqs, times, antenna_names, ant,
                     correct_type, interpolation_type, direction);
      MakeComplex(ant, freqs, correct_type, invert);
    } catch (const std::exception& e) {
      if (std::string(e.what()).rfind("SolTab has no element", 0) != 0) {
        throw;
      }
      if (missing_antenna_behavior == MissingAntennaBehavior::kError) {
        throw;
      } else if (missing_antenna_behavior == MissingAntennaBehavior::kFlag) {
        // Insert flagged solution
        const size_t tfsize = parms_.shape()[2];
        for (size_t parm = 0; parm < num_parms; ++parm) {
          for (size_t tf = 0; tf < tfsize; ++tf) {
            parms_(0, ant, tf) = std::numeric_limits<float>::quiet_NaN();
          }
        }
      } else if (missing_antenna_behavior == MissingAntennaBehavior::kUnit) {
        // Insert unit solution
        const size_t tfsize = parms_.shape()[2];
        if (num_parms == 2) {
          for (size_t tf = 0; tf < tfsize; ++tf) {
            parms_(0, ant, tf) = 1.;
            parms_(1, ant, tf) = 1.;
          }
        } else {
          for (size_t tf = 0; tf < tfsize; ++tf) {
            parms_(0, ant, tf) = 1.;
            parms_(1, ant, tf) = 0.;
            parms_(2, ant, tf) = 0.;
            parms_(3, ant, tf) = 1.;
          }
        }
      }
    }
  }
  if (invert) {
    Invert(parms_, sigma_mmse, correct_type);
  }
}

void JonesParameters::MakeComplex(size_t ant, const std::vector<double>& freqs,
                                  CorrectType correct_type, bool invert) {
  for (unsigned int tf = 0; tf < parms_.shape()[2]; ++tf) {
    const double freq = freqs[tf % freqs.size()];

    if (correct_type == GAIN) {
      parms_(0, ant, tf) =
          casacore::polar(parm_values_[0][ant][tf], parm_values_[1][ant][tf]);
      parms_(1, ant, tf) =
          casacore::polar(parm_values_[2][ant][tf], parm_values_[3][ant][tf]);
    } else if (correct_type == FULLJONES) {
      parms_(0, ant, tf) =
          casacore::polar(parm_values_[0][ant][tf], parm_values_[1][ant][tf]);
      parms_(1, ant, tf) =
          casacore::polar(parm_values_[2][ant][tf], parm_values_[3][ant][tf]);
      parms_(2, ant, tf) =
          casacore::polar(parm_values_[4][ant][tf], parm_values_[5][ant][tf]);
      parms_(3, ant, tf) =
          casacore::polar(parm_values_[6][ant][tf], parm_values_[7][ant][tf]);
    } else if (correct_type == SCALARGAIN) {
      parms_(0, ant, tf) =
          casacore::polar(parm_values_[0][ant][tf], parm_values_[1][ant][tf]);
      parms_(1, ant, tf) = parms_(0, ant, tf);
    } else if (correct_type == TEC) {
      parms_(0, ant, tf) =
          casacore::polar(1., parm_values_[0][ant][tf] * -8.44797245e9 / freq);
      if (parm_values_.size() == 1) {  // No TEC:0, only TEC:
        parms_(1, ant, tf) = casacore::polar(
            1., parm_values_[0][ant][tf] * -8.44797245e9 / freq);
      } else {  // TEC:0 and TEC:1
        parms_(1, ant, tf) = casacore::polar(
            1., parm_values_[1][ant][tf] * -8.44797245e9 / freq);
      }
    } else if (correct_type == CLOCK) {
      parms_(0, ant, tf) = casacore::polar(
          1., parm_values_[0][ant][tf] * freq * casacore::C::_2pi);
      if (parm_values_.size() == 1) {  // No Clock:0, only Clock:
        parms_(1, ant, tf) = casacore::polar(
            1., parm_values_[0][ant][tf] * freq * casacore::C::_2pi);
      } else {  // Clock:0 and Clock:1
        parms_(1, ant, tf) = casacore::polar(
            1., parm_values_[1][ant][tf] * freq * casacore::C::_2pi);
      }
    } else if (correct_type == ROTATIONANGLE) {
      double phi = parm_values_[0][ant][tf];
      if (invert) {
        phi = -phi;
      }
      const double sinv = sin(phi);
      const double cosv = cos(phi);
      parms_(0, ant, tf) = cosv;
      parms_(1, ant, tf) = -sinv;
      parms_(2, ant, tf) = sinv;
      parms_(3, ant, tf) = cosv;
    } else if (correct_type == ROTATIONMEASURE) {
      const double lambda2 = (casacore::C::c / freq) * (casacore::C::c / freq);
      double chi = parm_values_[0][ant][tf] * lambda2;
      if (invert) {
        chi = -chi;
      }
      const double sinv = sin(chi);
      const double cosv = cos(chi);
      parms_(0, ant, tf) = cosv;
      parms_(1, ant, tf) = -sinv;
      parms_(2, ant, tf) = sinv;
      parms_(3, ant, tf) = cosv;
    } else if (correct_type == PHASE || correct_type == SCALARPHASE) {
      parms_(0, ant, tf) = casacore::polar(1., parm_values_[0][ant][tf]);
      if (correct_type == SCALARPHASE) {  // Same value for x and y
        parms_(1, ant, tf) = casacore::polar(1., parm_values_[0][ant][tf]);
      } else {  // Different value for x and y
        parms_(1, ant, tf) = casacore::polar(1., parm_values_[1][ant][tf]);
      }
    } else if (correct_type == AMPLITUDE || correct_type == SCALARAMPLITUDE) {
      parms_(0, ant, tf) = parm_values_[0][ant][tf];
      if (correct_type == SCALARAMPLITUDE) {  // Same value for x and y
        parms_(1, ant, tf) = parm_values_[0][ant][tf];
      } else {  // Different value for x and y
        parms_(1, ant, tf) = parm_values_[1][ant][tf];
      }
    } else if (correct_type == GAIN_RE_IM) {
      parms_(0, ant, tf) = std::complex<float>(parm_values_[0][ant][tf],
                                               parm_values_[1][ant][tf]);
      parms_(1, ant, tf) = std::complex<float>(parm_values_[2][ant][tf],
                                               parm_values_[3][ant][tf]);
    } else if (correct_type == FULLJONES_RE_IM) {
      parms_(0, ant, tf) = std::complex<float>(parm_values_[0][ant][tf],
                                               parm_values_[1][ant][tf]);
      parms_(1, ant, tf) = std::complex<float>(parm_values_[2][ant][tf],
                                               parm_values_[3][ant][tf]);
      parms_(2, ant, tf) = std::complex<float>(parm_values_[4][ant][tf],
                                               parm_values_[5][ant][tf]);
      parms_(3, ant, tf) = std::complex<float>(parm_values_[6][ant][tf],
                                               parm_values_[7][ant][tf]);
    }
  }
}

unsigned int JonesParameters::GetNParms(CorrectType correct_type) {
  switch (correct_type) {
    case FULLJONES:
    case ROTATIONANGLE:
    case ROTATIONMEASURE:
    case FULLJONES_RE_IM:
      return 4;
    default:
      return 2;
  }
}

unsigned int JonesParameters::GetNParmValues(CorrectType correct_type) {
  switch (correct_type) {
    case FULLJONES:
    case FULLJONES_RE_IM:
      return 8;
    case GAIN:
    case GAIN_RE_IM:
      return 4;
    case PHASE:
    case AMPLITUDE:
    case SCALARGAIN:
      return 2;
    case ROTATIONANGLE:
    case SCALARPHASE:
    case ROTATIONMEASURE:
    case SCALARAMPLITUDE:
      return 1;
    default:
      throw std::runtime_error(
          "Correction type is variable. Use parameter parm_size instead.");
  }
}

void JonesParameters::FillParmValues(
    schaapcommon::h5parm::SolTab* sol_tab,
    schaapcommon::h5parm::SolTab* sol_tab2, const std::vector<double>& freqs,
    const std::vector<double>& times,
    const std::vector<std::string>& antenna_names, size_t ant,
    CorrectType correct_type, InterpolationType interpolation_type,
    unsigned long long direction) {
  auto get_flagged_values = [times, freqs, direction, interpolation_type](
                                SolTab* sol_tab,
                                const std::string& antenna_name,
                                unsigned int pol) -> std::vector<double> {
    std::vector<double> parmvalues = sol_tab->GetValuesOrWeights(
        "val", antenna_name, times, freqs, pol, direction,
        interpolation_type == InterpolationType::NEAREST);
    std::vector<double> weights = sol_tab->GetValuesOrWeights(
        "weight", antenna_name, times, freqs, pol, direction,
        interpolation_type == InterpolationType::NEAREST);
    ApplyFlags(parmvalues, weights);
    return parmvalues;
  };

  const std::string& ant_name = antenna_names[ant];
  if (correct_type == JonesParameters::CorrectType::FULLJONES ||
      correct_type == JonesParameters::CorrectType::GAIN ||
      correct_type == JonesParameters::CorrectType::SCALARGAIN) {
    for (size_t pol = 0; pol < parm_values_.size() / 2; ++pol) {
      // Place amplitude in even and phase in odd elements
      parm_values_[pol * 2][ant] = get_flagged_values(sol_tab, ant_name, pol);
      if (sol_tab2 == nullptr) {
        throw std::runtime_error(
            "soltab2 cannot be a nullpointer for correct_type=FULLJONES and "
            "correct_type=GAIN");
      }
      parm_values_[pol * 2 + 1][ant] =
          get_flagged_values(sol_tab2, ant_name, pol);
    }
  } else {
    for (size_t pol = 0; pol < parm_values_.size(); ++pol) {
      parm_values_[pol][ant] = get_flagged_values(sol_tab, ant_name, pol);
    }
  }
}

void JonesParameters::ApplyFlags(std::vector<double>& values,
                                 const std::vector<double>& weights) {
  assert(values.size() == weights.size());
  std::vector<double>::const_iterator weights_it = weights.begin();

  for (double& value : values) {
    if (*weights_it == 0.) {
      value = std::numeric_limits<float>::quiet_NaN();
    }
    weights_it++;
  }
}

void JonesParameters::Invert(casacore::Cube<casacore::Complex>& parms,
                             float sigma_mmse, CorrectType correct_type) {
  for (unsigned int tf = 0; tf < parms.shape()[2]; ++tf) {
    for (unsigned int ant = 0; ant < parms.shape()[1]; ++ant) {
      if (parms.shape()[0] == 2) {
        parms(0, ant, tf) = 1.f / parms(0, ant, tf);
        parms(1, ant, tf) = 1.f / parms(1, ant, tf);
      } else if (correct_type == FULLJONES || correct_type == FULLJONES_RE_IM) {
        aocommon::MC2x2F v(&parms(0, ant, tf));

        // Add the variance of the nuisance term to the elements on the
        // diagonal.
        const float variance = sigma_mmse * sigma_mmse;
        v[0] += variance;
        v[3] += variance;
        v.Invert();
        v.AssignTo(&parms(0, ant, tf));
      } else if (correct_type == ROTATIONMEASURE ||
                 correct_type == ROTATIONANGLE) {
        // rotationmeasure and commonrotationangle have been inverted already in
        // MakeComplex
      } else {
        throw std::runtime_error("Invalid correction type");
      }
    }
  }
}

std::string JonesParameters::CorrectTypeToString(
    JonesParameters::CorrectType ct) {
  if (ct == CorrectType::GAIN)
    return "gain";
  else if (ct == CorrectType::FULLJONES)
    return "fulljones";
  else if (ct == CorrectType::TEC)
    return "tec";
  else if (ct == CorrectType::CLOCK)
    return "clock";
  else if (ct == CorrectType::SCALARPHASE)
    return "scalarphase";
  else if (ct == CorrectType::SCALARAMPLITUDE)
    return "scalaramplitude";
  else if (ct == CorrectType::ROTATIONANGLE)
    return "rotationangle";
  else if (ct == CorrectType::ROTATIONMEASURE)
    return "rotationmeasure";
  else if (ct == CorrectType::PHASE)
    return "phase";
  else if (ct == CorrectType::AMPLITUDE)
    return "amplitude";
  else
    throw std::runtime_error("Unknown correction type: " + std::to_string(ct));
  return "";
}

JonesParameters::CorrectType JonesParameters::StringToCorrectType(
    const std::string& ctStr) {
  if (ctStr == "gain")
    return CorrectType::GAIN;
  else if (ctStr == "fulljones")
    return CorrectType::FULLJONES;
  else if (ctStr == "tec")
    return CorrectType::TEC;
  else if (ctStr == "clock")
    return CorrectType::CLOCK;
  else if (ctStr == "scalarphase" || ctStr == "commonscalarphase")
    return CorrectType::SCALARPHASE;
  else if (ctStr == "scalaramplitude" || ctStr == "commonscalaramplitude")
    return CorrectType::SCALARAMPLITUDE;
  else if (ctStr == "phase")
    return CorrectType::PHASE;
  else if (ctStr == "amplitude")
    return CorrectType::AMPLITUDE;
  else if (ctStr == "rotationangle" || ctStr == "commonrotationangle" ||
           ctStr == "rotation")
    return CorrectType::ROTATIONANGLE;
  else if (ctStr == "rotationmeasure")
    return CorrectType::ROTATIONMEASURE;
  else
    throw std::runtime_error("Unknown correction type: " + ctStr);
  return CorrectType::GAIN;
}

JonesParameters::MissingAntennaBehavior
JonesParameters::StringToMissingAntennaBehavior(
    const std::string& behavior_str) {
  if (behavior_str == "error") {
    return JonesParameters::MissingAntennaBehavior::kError;
  } else if (behavior_str == "flag") {
    return JonesParameters::MissingAntennaBehavior::kFlag;
  } else if (behavior_str == "unit") {
    return JonesParameters::MissingAntennaBehavior::kUnit;
  } else {
    throw std::runtime_error(
        "missingantennabehavior should be one of 'error', 'flag' or "
        "'unit', not '" +
        behavior_str + "'");
  }
}

std::string JonesParameters::MissingAntennaBehaviorToString(
    JonesParameters::MissingAntennaBehavior behavior) {
  if (behavior == JonesParameters::MissingAntennaBehavior::kError) {
    return "error";
  } else if (behavior == JonesParameters::MissingAntennaBehavior::kFlag) {
    return "flag";
  } else if (behavior == JonesParameters::MissingAntennaBehavior::kUnit) {
    return "unit";
  }
  throw std::logic_error("Unexpected missingantennabehavior");
}

std::vector<std::string> JonesParameters::ParseList(
    const std::string& list_str) {
  std::vector<std::string> list;
  boost::split(list, list_str, [](char c) { return c == ','; });
  if (list.size() > 2) {
    throw std::runtime_error(
        "List of soltab names should contain at most two entries.");
  }
  return list;
}

}  // namespace h5parm
}  // namespace schaapcommon
