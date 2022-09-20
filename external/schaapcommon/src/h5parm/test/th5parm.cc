// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <array>
#include <sstream>
#include <stdio.h>
#include <vector>

#include "h5parm.h"

using schaapcommon::h5parm::AxisInfo;
using schaapcommon::h5parm::H5Parm;
using schaapcommon::h5parm::SolTab;

using std::string;
using std::vector;

BOOST_AUTO_TEST_SUITE(h5parm)

namespace {

const size_t kNumAntennas = 3;
const size_t kNumFrequencies = 4;
const size_t kNumTimes = 7;

void CheckAxes(SolTab& soltab, size_t ntimes) {
  BOOST_CHECK_EQUAL(soltab.NumAxes(), size_t{3});
  BOOST_CHECK(soltab.HasAxis("ant"));
  BOOST_CHECK(soltab.HasAxis("time"));
  BOOST_CHECK(soltab.HasAxis("bla"));
  BOOST_CHECK_EQUAL(soltab.GetAxis(0).name, "ant");
  BOOST_CHECK_EQUAL(soltab.GetAxis(1).name, "time");
  BOOST_CHECK_EQUAL(soltab.GetAxis(2).name, "bla");
  BOOST_CHECK_EQUAL(soltab.GetAxis(0).size, size_t{3});
  BOOST_CHECK_EQUAL(soltab.GetAxis(1).size, ntimes);
  BOOST_CHECK_EQUAL(soltab.GetAxis(2).size, size_t{1});
}

void InitializeH5(H5Parm& h5parm) {
  // Add some metadata
  vector<string> antNames;
  vector<std::array<double, 3>> antPositions;
  for (unsigned int i = 0; i < 5; ++i) {
    std::stringstream antNameStr;
    antNameStr << "Antenna" << i;
    antNames.push_back(antNameStr.str());
    antPositions.push_back(std::array<double, 3>());
  }
  h5parm.AddAntennas(antNames, antPositions);
  h5parm.AddSources({"aaa", "bbb", "ccc", "ddd"},
                    {std::make_pair(0.0, 0.0), std::make_pair(0.0, 1.0),
                     std::make_pair(1.0, 1.0), std::make_pair(1.0, 0.0)});

  vector<AxisInfo> axes;
  axes.push_back(AxisInfo("ant", 3));
  axes.push_back(AxisInfo("time", kNumTimes));
  axes.push_back(AxisInfo("bla", 1));
  h5parm.CreateSolTab("mysol", "mytype", axes);

  vector<AxisInfo> axes_freq;
  axes_freq.push_back(AxisInfo("ant", kNumAntennas));
  axes_freq.push_back(AxisInfo("freq", kNumFrequencies));
  h5parm.CreateSolTab("mysolwithfreq", "mytype", axes_freq);

  vector<AxisInfo> axes_time_first;
  axes_time_first.push_back(AxisInfo("ant", kNumAntennas));
  axes_time_first.push_back(AxisInfo("time", kNumTimes));
  axes_time_first.push_back(AxisInfo("freq", kNumFrequencies));
  h5parm.CreateSolTab("timefreq", "mytype", axes_time_first);

  vector<AxisInfo> axes_freq_first;
  axes_freq_first.push_back(AxisInfo("ant", kNumAntennas));
  axes_freq_first.push_back(AxisInfo("freq", kNumFrequencies));
  axes_freq_first.push_back(AxisInfo("time", kNumTimes));
  h5parm.CreateSolTab("freqtime", "mytype", axes_freq_first);
}

void SetSolTabMeta(SolTab& soltab, bool set_freq_meta, bool set_time_meta) {
  // Add metadata for stations
  const std::vector<string> someAntNames = {"Antenna1", "Antenna12",
                                            "Antenna123"};
  soltab.SetAntennas(someAntNames);

  if (set_freq_meta) {
    // Add metadata for freqs;
    const std::vector<double> freqs{130e6, 131e6, 135e6, 137e6};
    soltab.SetFreqs(freqs);
  }

  if (set_time_meta) {
    // Add metadata for times
    std::vector<double> times;
    for (size_t time = 0; time < kNumTimes; ++time) {
      times.push_back(57878.5 + 2.0 * time);
    }
    soltab.SetTimes(times);
  }
}

void FillData(H5Parm& h5parm) {
  SolTab soltab = h5parm.GetSolTab("mysol");

  // Add some data
  vector<double> vals(kNumAntennas * kNumTimes);
  vector<double> weights(kNumAntennas * kNumTimes);
  for (size_t ant = 0; ant < kNumAntennas; ++ant) {
    for (size_t time = 0; time < kNumTimes; ++time) {
      vals[ant * kNumTimes + time] = 10 * ant + time;
      weights[ant * kNumTimes + time] = 0.4;
    }
  }

  soltab.SetValues(vals, weights, "CREATE with SchaapCommon-test");

  SetSolTabMeta(soltab, false, true);
}

void FillDataWithFreqAxis(H5Parm& h5parm) {
  SolTab soltab = h5parm.GetSolTab("mysolwithfreq");

  // Add some data
  const std::vector<double> vals(kNumAntennas * kNumFrequencies, 1.0);
  const std::vector<double> weights(kNumAntennas * kNumFrequencies, 1.0);

  soltab.SetValues(vals, weights, "CREATE with SchaapCommon-test");
  SetSolTabMeta(soltab, true, false);
}

void FillDataTimeFirst(H5Parm& h5parm) {
  SolTab soltab = h5parm.GetSolTab("timefreq");

  // Add some data
  vector<double> vals(kNumAntennas * kNumTimes * kNumFrequencies);
  vector<double> weights(kNumAntennas * kNumTimes * kNumFrequencies, 0);
  for (size_t ant = 0; ant < kNumAntennas; ++ant) {
    for (size_t time = 0; time < kNumTimes; ++time) {
      for (size_t freq = 0; freq < kNumFrequencies; ++freq) {
        vals[ant * kNumTimes * kNumFrequencies + time * kNumFrequencies +
             freq] = ant * time * freq;
      }
    }
  }

  // Add some data
  soltab.SetValues(vals, weights, "CREATE with SchaapCommon-test");
  SetSolTabMeta(soltab, true, true);
}

void FillDataFreqFirst(H5Parm& h5parm) {
  SolTab soltab = h5parm.GetSolTab("freqtime");

  // Add some data
  vector<double> vals(kNumAntennas * kNumTimes * kNumFrequencies);
  vector<double> weights(kNumAntennas * kNumTimes * kNumFrequencies, 0);
  for (size_t ant = 0; ant < kNumAntennas; ++ant) {
    for (size_t freq = 0; freq < kNumFrequencies; ++freq) {
      for (size_t time = 0; time < kNumTimes; ++time) {
        vals[ant * kNumTimes * kNumFrequencies + freq * kNumTimes + time] =
            ant * time * freq;
      }
    }
  }

  // Add some data
  soltab.SetValues(vals, weights, "CREATE with SchaapCommon-test");
  SetSolTabMeta(soltab, true, true);
}

struct H5Fixture {
  H5Fixture() {
    H5Parm h5parm("tH5Parm_tmp.h5", true);
    InitializeH5(h5parm);
    FillData(h5parm);
    FillDataWithFreqAxis(h5parm);
    FillDataTimeFirst(h5parm);
    FillDataFreqFirst(h5parm);
  }

  ~H5Fixture() { remove("tH5Parm_tmp.h5"); }
};
}  // namespace

BOOST_AUTO_TEST_CASE(create) {
  // Create a new H5Parm
  H5Parm h5parm("tH5Parm_tmp.h5", true);

  // Check that something is created
  BOOST_CHECK_EQUAL(((H5::H5File&)(h5parm)).getNumObjs(), 1u);

  // Check the name of the new solset "sol000"
  BOOST_CHECK_EQUAL(h5parm.GetSolSetName(), "sol000");

  InitializeH5(h5parm);

  // Check that the soltab exists
  BOOST_CHECK_EQUAL(h5parm.NumSolTabs(), 4);
  BOOST_CHECK(h5parm.HasSolTab("mysol"));

  // Check the axes
  SolTab soltab = h5parm.GetSolTab("mysol");
  BOOST_CHECK_EQUAL(soltab.GetType(), "mytype");
  CheckAxes(soltab, kNumTimes);
}

BOOST_FIXTURE_TEST_CASE(new_soltab, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", true, true, "harry");
  BOOST_CHECK_EQUAL(h5parm.GetSolSetName(), "harry");
  BOOST_CHECK_EQUAL(h5parm.NumSolTabs(), 0u);
}

BOOST_FIXTURE_TEST_CASE(existing_soltab, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");
  BOOST_CHECK_EQUAL(h5parm.GetSolSetName(), "sol000");
  BOOST_CHECK_EQUAL(h5parm.NumSolTabs(), 4u);
  BOOST_CHECK(h5parm.HasSolTab("mysol"));
  BOOST_CHECK(!h5parm.HasSolTab("nonexistingsol"));
}

BOOST_FIXTURE_TEST_CASE(axes, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");

  // Check the axes
  SolTab soltab = h5parm.GetSolTab("mysol");
  BOOST_CHECK_EQUAL(soltab.GetType(), "mytype");
  CheckAxes(soltab, kNumTimes);

  // Return and check nearest source
  BOOST_CHECK_EQUAL(h5parm.GetNearestSource(0.49, 0.49), "aaa");
  BOOST_CHECK_EQUAL(h5parm.GetNearestSource(1.0, 0.51), "ccc");
  BOOST_CHECK_EQUAL(h5parm.GetNearestSource(1.0, 0.49), "ddd");

  double starttime = 57878.49999;
  hsize_t starttimeindex = soltab.GetTimeIndex(starttime);
  vector<double> val = soltab.GetValues("Antenna12", starttimeindex, kNumTimes,
                                        1, 0, 4, 0, 4, 0);
  BOOST_CHECK_CLOSE(val[0], 10., 1e-8);
  BOOST_CHECK_CLOSE(val[1], 11., 1e-8);
  BOOST_CHECK_CLOSE(val[2], 12., 1e-8);
  BOOST_CHECK_CLOSE(val[3], 13., 1e-8);

  starttime = 57880.5;
  starttimeindex = soltab.GetTimeIndex(starttime);
  BOOST_CHECK_EQUAL(starttimeindex, hsize_t{1});
  vector<double> val2 =
      soltab.GetValues("Antenna123", starttimeindex, 2, 2, 0, 4, 0, 4, 0);

  BOOST_CHECK_CLOSE(val2[0], 21., 1e-8);
  BOOST_CHECK_CLOSE(val2[1], 23., 1e-8);
  BOOST_CHECK_CLOSE(soltab.GetTimeInterval(), 2., 1e-8);

  vector<string> antennas = soltab.GetStringAxis("ant");
  BOOST_CHECK_EQUAL(antennas.size(), size_t{3});
  BOOST_CHECK_EQUAL(antennas[0], "Antenna1");
  BOOST_CHECK_EQUAL(antennas[1], "Antenna12");
  BOOST_CHECK_EQUAL(antennas[2], "Antenna123");
}

BOOST_FIXTURE_TEST_CASE(grid_interpolation, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");
  SolTab& soltab = h5parm.GetSolTab("mysol");

  const vector<double> freqs{130e6, 131e6};

  const vector<double> times1{57878.5, 57880.5, 57882.5, 57884.5,
                              57886.5, 57888.5, 57890.5};

  vector<double> newgridvals =
      soltab.GetValuesOrWeights("val", "Antenna1", times1, freqs, 0, 0, true);
  BOOST_REQUIRE_EQUAL(newgridvals.size(), times1.size() * freqs.size());
  size_t idx = 0;
  for (size_t time = 0; time < times1.size(); ++time) {
    for (size_t freq = 0; freq < freqs.size(); ++freq) {
      BOOST_CHECK_CLOSE(newgridvals[idx], time, 1e-8);
      ++idx;
    }
  }

  vector<double> times2;
  for (size_t time = 0; time < 3 * times1.size() + 2; ++time) {
    times2.push_back(57878.5 + 2.0 * time / 3.);
  }
  newgridvals =
      soltab.GetValuesOrWeights("val", "Antenna1", times2, freqs, 0, 0, true);
  BOOST_REQUIRE_EQUAL(newgridvals.size(), times2.size() * freqs.size());
  idx = 0;
  for (size_t time = 0; time < times2.size(); ++time) {
    for (size_t freq = 0; freq < freqs.size(); ++freq) {
      BOOST_CHECK_CLOSE(newgridvals[idx],
                        std::min((time + 1) / 3, times1.size() - 1), 1e-8);
      ++idx;
    }
  }
}

BOOST_FIXTURE_TEST_CASE(interpolate_single_time, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");
  SolTab& soltab = h5parm.GetSolTab("mysol");

  const vector<double> freqs{130e6, 131e6};
  const vector<double> times{57000, 57878.7, 57880.3, 57890.3, 58000};
  const vector<double> expected_nearest{10, 10, 11, 16, 16};
  const vector<double> expected_bilinear{10.0, 10.1, 10.9, 15.9, 16.0};

  for (size_t time = 0; time < times.size(); ++time) {
    const vector<double> nearest_vals = soltab.GetValuesOrWeights(
        "val", "Antenna12", {times[time]}, freqs, 0, 0, true);
    const vector<double> bilinear_vals = soltab.GetValuesOrWeights(
        "val", "Antenna12", {times[time]}, freqs, 0, 0, false);

    BOOST_REQUIRE_EQUAL(nearest_vals.size(), 1 * freqs.size());
    BOOST_REQUIRE_EQUAL(bilinear_vals.size(), 1 * freqs.size());

    for (size_t freq = 0; freq < freqs.size(); ++freq) {
      BOOST_CHECK_CLOSE(nearest_vals[freq], expected_nearest[time], 1e-8);
      BOOST_CHECK_CLOSE(bilinear_vals[freq], expected_bilinear[time], 1e-8);
    }
  }
}

BOOST_FIXTURE_TEST_CASE(freq_interval_and_index, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");
  const SolTab& soltab = h5parm.GetSolTab("mysolwithfreq");
  BOOST_CHECK_CLOSE(soltab.GetFreqInterval(0), 1.0e6, 1.0e-8);
  BOOST_CHECK_CLOSE(soltab.GetFreqInterval(1), 4.0e6, 1.0e-8);
  BOOST_CHECK_CLOSE(soltab.GetFreqInterval(2), 2.0e6, 1.0e-8);

  BOOST_CHECK_THROW(soltab.GetFreqIndex(128.0e6),
                    std::runtime_error);               // Too far from lowest
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(129.1e6), 0);  // closest to 130e6
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(130.4e6), 0);  // closest to 130e6
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(130.6e6), 1);  // closest to 131e6
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(136.1e6), 3);  // closest to 137e6
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(137.0e6), 3);  // closest to 137e6
  BOOST_CHECK_EQUAL(soltab.GetFreqIndex(137.8e6), 3);  // closest to 137e6
  BOOST_CHECK_THROW(soltab.GetFreqIndex(150.0e6),
                    std::runtime_error);  // Too far from highest
}

BOOST_FIXTURE_TEST_CASE(axis_ordering, H5Fixture) {
  H5Parm h5parm("tH5Parm_tmp.h5", false, false, "sol000");

  SolTab soltab_tf = h5parm.GetSolTab("timefreq");
  SolTab soltab_ft = h5parm.GetSolTab("freqtime");

  const vector<double> freqs{130e6, 135e6, 131e6};
  const vector<double> times{57000, 57878.7, 57880.3, 57890.3, 58000};

  const vector<double> nearest_tf = soltab_tf.GetValuesOrWeights(
      "val", "Antenna12", times, freqs, 0, 0, true);
  const vector<double> nearest_ft = soltab_ft.GetValuesOrWeights(
      "val", "Antenna12", times, freqs, 0, 0, true);

  // GetValuesOrWeights should make sure that frequency is the fastest changing
  // index, even when in the underlying h5 array time was changing fastest
  BOOST_CHECK_EQUAL_COLLECTIONS(nearest_tf.begin(), nearest_tf.end(),
                                nearest_ft.begin(), nearest_ft.end());
}

BOOST_AUTO_TEST_SUITE_END()
