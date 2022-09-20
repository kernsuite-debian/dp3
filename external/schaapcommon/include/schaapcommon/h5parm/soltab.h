// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCHAAPCOMMON_H5PARM_SOLTAB_H
#define SCHAAPCOMMON_H5PARM_SOLTAB_H

#include <string>
#include <vector>
#include <complex>
#include <map>

#include <H5Cpp.h>

namespace schaapcommon {
namespace h5parm {

/// A name and the length of an exis, e.g. ('freq', 800) for 800 frequencies
struct AxisInfo {
 public:
  AxisInfo(const std::string& name, unsigned int size)
      : name(name), size(size){};

  std::string name;
  unsigned int size;
};

/// @brief SolTab is a solution table as defined in the H5Parm standard. It
/// contains one solution, e.g. all TEC values, with different axes
/// for that solution (e.g. time, freq, pol).
class SolTab : private H5::Group {
 public:
  SolTab(){};

  /// Create a new soltab, add it to its parent
  SolTab(H5::Group group, const std::string& type,
         const std::vector<AxisInfo> axes  /// Axes, fastest varying last
  );

  /// Create a soltab from a H5::Group (for reading existing files)
  SolTab(H5::Group& group);

  /// The destructor could check for valid subtables
  virtual ~SolTab();

  /// Add a version stamp in the attributes of the group
  static void AddVersionStamp(H5::Group& node);

  std::vector<AxisInfo>& GetAxes() { return axes_; }

  const std::vector<AxisInfo>& GetAxes() const { return axes_; }

  AxisInfo GetAxis(unsigned int i) const;

  /// Get an axis, throw an exception if it does not exist
  AxisInfo GetAxis(const std::string& axis_name) const;

  size_t NumAxes() const { return axes_.size(); }

  bool HasAxis(const std::string& axis_name) const;

  /// Get the index of an axis
  size_t GetAxisIndex(const std::string& axis_name) const;

  void SetAntennas(const std::vector<std::string>& sol_antennas);

  void SetSources(const std::vector<std::string>& sol_sources);

  void SetPolarizations(const std::vector<std::string>& polarizations);

  void SetFreqs(const std::vector<double>& freqs);

  /// Get the values of a real-valued axis (e.g. "time" or "freq")
  std::vector<double> GetRealAxis(const std::string& axis_name) const;

  /// Get the values of a string-valued axis (e.g. "dir" or "pol")
  std::vector<std::string> GetStringAxis(const std::string& axis_name);

  /// Get the index of freq, using nearest neighbor.
  /// This function assumes that the frequencies are in increasing order.
  /// @throw std::runtime_error If the frequency is less than a full frequency
  /// width below the lowest or above the highest frequency
  hsize_t GetFreqIndex(double freq) const;

  /// Get the index of a time. Matches with 0.5*timeInterval
  /// This assumes that all times are regularly spaced
  hsize_t GetTimeIndex(double time) const;

  /// Get the index for a direction name
  hsize_t GetDirIndex(const std::string& direction_name);

  /// Gets the interval (in s.) between a time slot (default first) and
  /// the next. Throws error if there is only one time slot.
  double GetTimeInterval(size_t start = 0) const {
    return GetInterval("time", start);
  }

  /// Gets the interval (in s.) between a channel (default first) and
  /// the next. Throws error if there is only one frequency.
  double GetFreqInterval(size_t start = 0) const {
    return GetInterval("freq", start);
  }

  void SetTimes(const std::vector<double>& times);

  /// Set metadata about an axis (like freq or time))
  void SetAxisMeta(const std::string& meta_name,
                   const std::vector<double>& meta_vals);

  /// Set metadata about an axis (like polarization, direction)
  void SetAxisMeta(const std::string& meta_name, size_t str_len,
                   const std::vector<std::string>& meta_vals);

  /// Adds a real solution.
  /// If weights are emtpy, write ones everywhere
  void SetValues(const std::vector<double>& vals,
                 const std::vector<double>& weights,
                 const std::string& history = "");

  /// Add a complex solution, taking either amplitude or phase
  void SetComplexValues(const std::vector<std::complex<double>>& vals,
                        const std::vector<double>& weights, bool to_amplitudes,
                        const std::string& history = "");

  /// Get the name of this SolTab
  std::string GetName() const;

  std::string GetType() const { return type_; }

  /// Get the values of this SolTab for a given antenna.
  std::vector<double> GetValues(const std::string& ant_name,
                                unsigned int starttimeslot, unsigned int ntime,
                                unsigned int timestep, unsigned int startfreq,
                                unsigned int nfreq, unsigned int freqstep,
                                unsigned int pol, unsigned int dir) {
    return GetValuesOrWeights("val", ant_name, starttimeslot, ntime, timestep,
                              startfreq, nfreq, freqstep, pol, dir);
  }

  /// Get the weights of this SolTab for a given antenna.
  std::vector<double> GetWeights(const std::string& ant_name,
                                 unsigned int starttimeslot, unsigned int ntime,
                                 unsigned int timestep, unsigned int startfreq,
                                 unsigned int nfreq, unsigned int freqstep,
                                 unsigned int pol, unsigned int dir) {
    return GetValuesOrWeights("weight", ant_name, starttimeslot, ntime,
                              timestep, startfreq, nfreq, freqstep, pol, dir);
  }

  /// Get the values or weights (method made virtual and public to facilitate
  /// mocking)
  virtual std::vector<double> GetValuesOrWeights(
      const std::string& val_or_weight, const std::string& ant_name,
      const std::vector<double>& times, const std::vector<double>& freqs,
      unsigned int pol, unsigned int dir, bool nearest);

 private:
  static double TakeAbs(std::complex<double> c) { return std::abs(c); }
  static double TakeArg(std::complex<double> c) { return std::arg(c); }

  /// Get the values or weights of this SolTab for a given antenna given an
  /// antenna name, a direction index, and a (range of) times and frequencies.
  /// In the returned vector, the freq will be the fastest changing index,
  /// irrespective of the axis ordering in the underlying h5 data structure.
  std::vector<double> GetValuesOrWeights(
      const std::string& val_or_weight, const std::string& ant_name,
      unsigned int starttimeslot, unsigned int ntime, unsigned int timestep,
      unsigned int startfreq, unsigned int nfreq, unsigned int freqstep,
      unsigned int pol, unsigned int dir);

  void ReadAxes();

  void FillCache(std::map<std::string, hsize_t>& cache,
                 const std::string& table_name);

  /// Get the interval of the axis axis_name
  double GetInterval(const std::string& axis_name, size_t start = 0) const;
  hsize_t GetAntIndex(const std::string& ant_name);
  hsize_t GetNamedIndex(std::map<std::string, hsize_t>& cache,
                        const std::string& table_name,
                        const std::string& element_name);

  std::string type_;
  std::vector<AxisInfo> axes_;
  std::map<std::string, hsize_t> ant_map_;
  std::map<std::string, hsize_t> dir_map_;
};
}  // namespace h5parm
}  // namespace schaapcommon

#endif
