// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include "ds9facetfile.h"
#include "facet.h"
#include "tfacet.h"

using schaapcommon::facets::DS9FacetFile;
using schaapcommon::facets::Facet;

BOOST_AUTO_TEST_SUITE(tds9facetfile)

BOOST_AUTO_TEST_CASE(direction_comment) {
  std::string dir = DS9FacetFile::ParseDirectionLabel(
      DS9FacetFile::TokenType::kComment, "text=CygA, color=green");

  BOOST_CHECK_EQUAL(dir, "CygA");
}

BOOST_AUTO_TEST_CASE(direction_comment_reversed) {
  std::string dir = DS9FacetFile::ParseDirectionLabel(
      DS9FacetFile::TokenType::kComment, "color=green, text=CygA");

  BOOST_CHECK_EQUAL(dir, "CygA");
}

BOOST_AUTO_TEST_CASE(direction_empty) {
  std::string dir = DS9FacetFile::ParseDirectionLabel(
      DS9FacetFile::TokenType::kComment, "some other comment");

  BOOST_CHECK(dir.empty());
}

BOOST_AUTO_TEST_CASE(direction_wrong_type) {
  std::string dir = DS9FacetFile::ParseDirectionLabel(
      DS9FacetFile::TokenType::kWord, "text=CygA");

  BOOST_CHECK(dir.empty());
}

// Check if facets are correctly read from DS9 regions file
BOOST_AUTO_TEST_CASE(read_from_file) {
  DS9FacetFile facet_file("foursources.reg");
  DS9FacetFile facet_file_shared("foursources.reg");

  // Values copied from DP3 integration test
  const double ra = 0.426246;
  const double dec = 0.578747;
  const double px_scale_x = 0.000174533;
  const double px_scale_y = px_scale_x;
  const size_t full_width = 512;
  const size_t full_height = full_width;

  std::vector<Facet> facets_out = facet_file.Read();
  std::vector<std::shared_ptr<Facet>> facets_shared =
      facet_file_shared.ReadShared();

  BOOST_CHECK_EQUAL(facets_out.size(), 4);
  BOOST_CHECK_EQUAL(facets_shared.size(), 4);

  for (Facet& facet : facets_out) {
    facet.CalculatePixels(ra, dec, px_scale_x, px_scale_y, full_width,
                          full_height, 0.0, 0.0);
  }
  for (std::shared_ptr<Facet>& facet : facets_shared) {
    facet->CalculatePixels(ra, dec, px_scale_x, px_scale_y, full_width,
                           full_height, 0.0, 0.0);
  }

  // Text labels
  const std::array<std::string, 4> kDirectionLabels{"CygA", "CygAO", "CygTJ",
                                                    ""};
  // 'point' values from foursources.reg, converted to radians.
  const std::array<double, 4> kFacetRa{
      23.0 * M_PI / 180.0, 23.10 * M_PI / 180.0, 22.92 * M_PI / 180.0};
  const std::array<double, 4> kFacetDec{
      31.5 * M_PI / 180.0, 33.6 * M_PI / 180.0, 31.87 * M_PI / 180.0};

  // Expected bounding box values for the facets.
  const std::array<int, 4> bbox_xmin = {377, 364, 128, 160};
  const std::array<int, 4> bbox_xmax = {512, 512, 384, 352};
  const std::array<int, 4> bbox_ymin = {0, 128, 0, 160};
  const std::array<int, 4> bbox_ymax = {94, 304, 128, 352};

  for (size_t idx = 0; idx < facets_out.size(); ++idx) {
    BOOST_CHECK_EQUAL(facets_out[idx].DirectionLabel(), kDirectionLabels[idx]);
    if (idx < 3) {
      BOOST_CHECK_CLOSE(facets_out[idx].RA(), kFacetRa[idx], 1.0e-6);
      BOOST_CHECK_CLOSE(facets_out[idx].Dec(), kFacetDec[idx], 1.0e-6);
      BOOST_CHECK_CLOSE(facets_shared[idx]->RA(), kFacetRa[idx], 1.0e-6);
      BOOST_CHECK_CLOSE(facets_shared[idx]->Dec(), kFacetDec[idx], 1.0e-6);
    }

    const schaapcommon::facets::BoundingBox box =
        facets_out[idx].GetUntrimmedBoundingBox();
    const schaapcommon::facets::BoundingBox box_shared =
        facets_shared[idx]->GetUntrimmedBoundingBox();

    BOOST_CHECK_EQUAL(box.Min().x, bbox_xmin[idx]);
    BOOST_CHECK_EQUAL(box.Max().x, bbox_xmax[idx]);
    BOOST_CHECK_EQUAL(box.Min().y, bbox_ymin[idx]);
    BOOST_CHECK_EQUAL(box.Max().y, bbox_ymax[idx]);
    BOOST_CHECK_EQUAL(box_shared.Min().x, bbox_xmin[idx]);
    BOOST_CHECK_EQUAL(box_shared.Max().x, bbox_xmax[idx]);
    BOOST_CHECK_EQUAL(box_shared.Min().y, bbox_ymin[idx]);
    BOOST_CHECK_EQUAL(box_shared.Max().y, bbox_ymax[idx]);
  }
}

BOOST_AUTO_TEST_CASE(file_does_not_exist) {
  BOOST_CHECK_THROW(DS9FacetFile facet_file("does-not-exist.reg"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
