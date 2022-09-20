// tgridinterpolate.cc: test program for gridinterpolate
// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// @author Tammo Jan Dijkema

#include <boost/test/unit_test.hpp>

#include "../gridinterpolate.h"

using std::vector;

BOOST_AUTO_TEST_SUITE(gridinterpolate)

BOOST_AUTO_TEST_CASE(test_nearest_neighbor) {
  vector<double> ax_src = {1, 3};
  vector<double> ax_tgt = {0.5, 1.5, 2.5, 3.5};

  vector<size_t> indices;
  schaapcommon::h5parm::GetAxisIndices(ax_src, ax_tgt, indices);
  BOOST_CHECK_EQUAL(indices.size(), ax_tgt.size());
  BOOST_CHECK_EQUAL(indices[0], 0u);
  BOOST_CHECK_EQUAL(indices[1], 0u);
  BOOST_CHECK_EQUAL(indices[2], 1u);
  BOOST_CHECK_EQUAL(indices[3], 1u);

  const vector<double> x_src = {2, 4, 8, 10};
  const vector<double> y_src = {3, 6, 12};
  const vector<double> x_tgt = {1, 3.5, 9.5, 10};
  const vector<double> y_tgt = {4, 10};
  vector<double> vals_src(x_src.size() * y_src.size());
  for (size_t i = 0; i < vals_src.size(); ++i) {
    vals_src[i] = i;
  }

  schaapcommon::h5parm::GetAxisIndices(x_src, x_tgt, indices);
  BOOST_CHECK_EQUAL(indices.size(), x_tgt.size());
  BOOST_CHECK_EQUAL(indices[0], 0u);
  BOOST_CHECK_EQUAL(indices[1], 1u);
  BOOST_CHECK_EQUAL(indices[2], 3u);
  BOOST_CHECK_EQUAL(indices[3], 3u);

  // Row major indexing of vals_src
  const std::vector<size_t> rm_index = {0, 2, 3, 5, 9, 11, 9, 11};

  // Convert to column major indexes in vals_src
  std::vector<size_t> cm_index;
  for (const auto& idx : rm_index) {
    const size_t i = idx / y_src.size();
    const size_t j = idx % y_src.size();
    cm_index.push_back(j * x_src.size() + i);
  }

  // y changing fastest in vals_src (row major order)
  const std::vector<double> vals_tgt_row =
      schaapcommon::h5parm::GridNearestNeighbor(
          x_src, y_src, x_tgt, y_tgt, vals_src,
          schaapcommon::h5parm::MemoryLayout::kRowMajor);
  BOOST_REQUIRE_EQUAL(vals_tgt_row.size(), x_tgt.size() * y_tgt.size());

  // x changing fastest in vals_src (col major order)
  const std::vector<double> vals_tgt_col =
      schaapcommon::h5parm::GridNearestNeighbor(
          x_src, y_src, x_tgt, y_tgt, vals_src,
          schaapcommon::h5parm::MemoryLayout::kColumnMajor);
  BOOST_REQUIRE_EQUAL(vals_tgt_col.size(), x_tgt.size() * y_tgt.size());

  for (size_t i = 0; i < vals_tgt_row.size(); ++i) {
    BOOST_CHECK_EQUAL(vals_tgt_row[i], vals_src[rm_index[i]]);
    BOOST_CHECK_EQUAL(vals_tgt_col[i], vals_src[cm_index[i]]);
  }
}

BOOST_AUTO_TEST_CASE(test_bilinear) {
  const vector<double> x_src = {2, 4, 8, 10};
  const vector<double> y_src = {3, 6, 12};
  const vector<double> x_tgt = {2, 2.5, 3.5, 9.5, 10};
  const vector<double> y_tgt = {3, 4, 10, 12};

  vector<double> vals_src(x_src.size() * y_src.size());

  for (size_t i = 0; i < x_src.size(); ++i) {
    for (size_t j = 0; j < y_src.size(); ++j) {
      size_t idx = i * y_src.size() + j;
      // linear polynomial f(x, y) = 3*x + x*y - 2 * y + 5
      vals_src[idx] = 3 * x_src[i] + x_src[i] * y_src[j] - 2 * y_src[j] + 5;
    }
  }

  // Col major lay-out in combination with linear interpolation not implemented
  BOOST_CHECK_THROW(
      schaapcommon::h5parm::GridNearestNeighbor(
          x_src, y_src, x_tgt, y_tgt, vals_src,
          schaapcommon::h5parm::MemoryLayout::kColumnMajor, false),
      std::runtime_error);

  // set nearest = false to use bilinear interpolation
  const vector<double> vals_tgt = schaapcommon::h5parm::GridNearestNeighbor(
      x_src, y_src, x_tgt, y_tgt, vals_src,
      schaapcommon::h5parm::MemoryLayout::kRowMajor, false);
  BOOST_REQUIRE_EQUAL(vals_tgt.size(), x_tgt.size() * y_tgt.size());

  for (size_t i = 0; i < x_tgt.size(); ++i) {
    for (size_t j = 0; j < y_tgt.size(); ++j) {
      size_t idx = i * y_tgt.size() + j;
      // linear polynomial f(x, y) = 3*x + x*y - 2 * y + 5 should be reproduced
      // in bilinear interpolation
      const double ref_val =
          3 * x_tgt[i] + x_tgt[i] * y_tgt[j] - 2 * y_tgt[j] + 5;
      BOOST_CHECK_CLOSE(vals_tgt[idx], ref_val, 1e-8);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
