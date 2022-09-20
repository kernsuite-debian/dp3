// gridinterpolate.h: Interpolate data from regular 2d grid to another
// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

/// @file
/// @brief Interpolate data from regular 2d grid to another
/// @author Tammo Jan Dijkema

#ifndef SCHAAPCOMMON_H5PARM_GRIDINTERPOLATE_H
#define SCHAAPCOMMON_H5PARM_GRIDINTERPOLATE_H

#include <vector>
#include <stdexcept>

namespace schaapcommon {
namespace h5parm {

enum class MemoryLayout { kRowMajor, kColumnMajor };

/**
 * Get the nearest-neighbor indices
 * \param ax_src[in] Vector with points where the data is defined.
 *                   Should be increasing.
 * \param ax_tgt[in] Vector with the points at which the values are
 *                   needed.  Should be increasing.
 * \param[out] indices Vector (same length as ax_tgt) with for each number
 *                     in ax_src, the index of the nearest point in ax_src.
 * \param[in] nearest Get the nearest point. If false, gets the largest
 *                    point that is smaller.
 */
void GetAxisIndices(const std::vector<double>& ax_src,
                    const std::vector<double>& ax_tgt,
                    std::vector<size_t>& indices, bool nearest = true);

/**
 * Regrid 2d-gridded data onto another 2d grid. In the return vector,
 * y changes fastest, so that the returned vector can be interpreted as
 * a row-major 2D matrix.
 *
 * \param[in] x_src x-axis on which the data is defined
 * \param[in] y_src y-axis on which the data is defined
 * \param[in] x_tgt x-axis on which the data will be evaluated
 * \param[in] y_tgt y-axis on which the data will be evaluated
 * \param[in] vals_src original data as a flattened 2D array. The ordering
 * \param[in] is_row_major interpret the input values as a row major 2D array.
 * If false, the vector of input values is interpreted in column major order
 * \param[in] nearest perform nearest interpolation (true), or bilinear
 * interpolation (false). Defaults to nearest = true
 * \return regridded data, y-axis varies fastest
 */
std::vector<double> GridNearestNeighbor(const std::vector<double>& x_src,
                                        const std::vector<double>& y_src,
                                        const std::vector<double>& x_tgt,
                                        const std::vector<double>& y_tgt,
                                        const std::vector<double>& vals_src,
                                        MemoryLayout mem_layout,
                                        bool nearest = true);

}  // namespace h5parm
}  // namespace schaapcommon

#endif
