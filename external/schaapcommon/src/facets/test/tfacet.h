// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TFACET_H
#define TFACET_H

// This headers contains common code for facet tests.
// tfacet.cc contains both the implementation(s) for this header
// and the facet tests.

#include "facet.h"

#include <ostream>

namespace schaapcommon {
namespace facets {

// This operator allows using BOOST_CHECK_EQUAL on Pixel objects.
inline std::ostream& operator<<(std::ostream& stream, const Pixel& pixel) {
  stream << "Pixel(" << pixel.x << ", " << pixel.y << ")";
  return stream;
}

}  // namespace facets
}  // namespace schaapcommon

#endif  // TFACET_H