// gridinterpolate.cc: Interpolate data from regular 2d grid to another
// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gridinterpolate.h"

#include <iostream>
#include <vector>
#include <stdexcept>

// using namespace std;

namespace schaapcommon {
namespace h5parm {
void GetAxisIndices(const std::vector<double>& ax_src,
                    const std::vector<double>& ax_tgt,
                    std::vector<size_t>& indices, bool nearest) {
  indices.resize(ax_tgt.size());
  if (ax_tgt.empty()) {
    return;
  }
  if (ax_src.empty()) throw std::invalid_argument("ax_src is empty");

  double lowmatch, highmatch;

  std::vector<double>::const_iterator src_val = ax_src.begin();
  std::vector<double>::const_iterator tgt_val = ax_tgt.begin();
  std::vector<size_t>::iterator index_val = indices.begin();

  while (tgt_val != ax_tgt.end()) {
    while (src_val != ax_src.end() && *src_val < *tgt_val) {
      src_val++;
    }
    if (src_val == ax_src.begin()) {
      *index_val = src_val - ax_src.begin();
    } else if (src_val == ax_src.end()) {
      *index_val = src_val - ax_src.begin() - 1;
    } else {
      if (nearest) {
        lowmatch = *(src_val - 1);
        highmatch = *src_val;

        if (highmatch - *tgt_val < *tgt_val - lowmatch) {
          *index_val = src_val - ax_src.begin();
        } else {
          *index_val = src_val - ax_src.begin() - 1;
        }
      } else {
        *index_val = src_val - ax_src.begin() - 1;
      }
    }
    tgt_val++;
    index_val++;
  }
}

std::vector<double> GridNearestNeighbor(const std::vector<double>& x_src,
                                        const std::vector<double>& y_src,
                                        const std::vector<double>& x_tgt,
                                        const std::vector<double>& y_tgt,
                                        const std::vector<double>& vals_src,
                                        MemoryLayout mem_layout, bool nearest) {
  if (mem_layout != MemoryLayout::kRowMajor && !nearest && y_src.size() > 1) {
    // If y_src.size() == 1, no difference between row/column major lay-out
    throw std::runtime_error(
        "Swapping the memory lay-out not possible for bilinear interpolations");
  }

  std::vector<size_t> x_indices;
  std::vector<size_t> y_indices;
  GetAxisIndices(x_src, x_tgt, x_indices, nearest);
  GetAxisIndices(y_src, y_tgt, y_indices, nearest);

  const size_t nx = x_tgt.size();
  const size_t ny = y_tgt.size();
  const size_t ny_src = y_src.size();
  const size_t nx_src = x_src.size();
  std::vector<double> vals_tgt(nx * ny);

  if (nearest) {
    if (mem_layout == MemoryLayout::kRowMajor) {
      // Input vector is indexed as row_major 2D matrix - y changing fastest
      for (size_t i = 0; i < nx; ++i) {
        for (size_t j = 0; j < ny; ++j) {
          vals_tgt[i * ny + j] = vals_src[x_indices[i] * ny_src + y_indices[j]];
        }
      }
    } else {
      // Input vector is indexed as col major 2D matrix - x changing fastest
      for (size_t i = 0; i < nx; ++i) {
        for (size_t j = 0; j < ny; ++j) {
          vals_tgt[i * ny + j] = vals_src[y_indices[j] * nx_src + x_indices[i]];
        }
      }
    }
  } else {
    for (size_t i = 0; i < nx; ++i) {
      for (size_t j = 0; j < ny; ++j) {
        size_t y0_idx, y1_idx;
        bool interpolate_y = true;
        if (y_tgt[j] <= y_src.front()) {
          y0_idx = 0;
          y1_idx = 0;
          interpolate_y = false;
        } else if (y_tgt[j] >= y_src.back()) {
          y0_idx = y_src.size() - 1;
          y1_idx = y_src.size() - 1;
          interpolate_y = false;
        } else {
          y0_idx = y_indices[j];
          y1_idx = y_indices[j] + 1;
        }

        double f_y0, f_y1;
        if (x_tgt[i] <= x_src.front()) {
          f_y0 = vals_src[y0_idx];
          f_y1 = vals_src[y1_idx];
        } else if (x_tgt[i] >= x_src.back()) {
          f_y0 = vals_src[(x_src.size() - 1) * ny_src + y0_idx];
          f_y1 = vals_src[(x_src.size() - 1) * ny_src + y1_idx];
        } else {
          size_t x0_idx = x_indices[i];
          double x0 = x_src[x0_idx];
          double x1 = x_src[x0_idx + 1];
          double x = x_tgt[i];
          f_y0 = vals_src[x0_idx * ny_src + y0_idx] +
                 (x - x0) / (x1 - x0) *
                     (vals_src[(x0_idx + 1) * ny_src + y0_idx] -
                      vals_src[x0_idx * ny_src + y0_idx]);
          f_y1 = vals_src[x0_idx * ny_src + y1_idx] +
                 (x - x0) / (x1 - x0) *
                     (vals_src[(x0_idx + 1) * ny_src + y1_idx] -
                      vals_src[x0_idx * ny_src + y1_idx]);
        }
        if (interpolate_y) {
          double y0 = y_src[y0_idx];
          double y1 = y_src[y0_idx + 1];
          double y = y_tgt[j];
          vals_tgt[i * ny + j] = f_y0 + (y - y0) / (y1 - y0) * (f_y1 - f_y0);
        } else {
          vals_tgt[i * ny + j] = f_y0;
        }
      }
    }
  }

  return vals_tgt;
}
}  // namespace h5parm
}  // namespace schaapcommon
