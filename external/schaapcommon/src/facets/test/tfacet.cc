// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include "tfacet.h"

#include <aocommon/io/serialistream.h>
#include <aocommon/io/serialostream.h>
#include <aocommon/imagecoordinates.h>

using schaapcommon::facets::BoundingBox;
using schaapcommon::facets::Coord;
using schaapcommon::facets::Facet;
using schaapcommon::facets::Pixel;

namespace {
const double kScale = 0.001;
const size_t kImageSize = 100;

/**
 * Creates a facet with a diamond shape.
 */
Facet CreateDiamondFacet(int offset_x = 0, int offset_y = 0,
                         bool set_custom_direction = false) {
  Facet facet;

  const std::vector<Coord> coords{
      {0.02, 0.0}, {0.0, 0.01}, {-0.02, 0.0}, {0.0, -0.01}};
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const double ra = offset_x * kScale;
  const double dec = -offset_y * kScale;
  const double l = 0.0;
  const double m = 0.0;
  if (set_custom_direction) {
    facet.SetRA(-0.02);
    facet.SetDec(0.01);
  }
  facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m);

  return facet;
}

/**
 * Creates a facet with a rectangular shape.
 */
Facet CreateRectangularFacet(double padding, size_t align, bool make_square) {
  Facet facet;

  const std::vector<Coord> coords{
      {0.01, -0.02}, {0.01, 0.02}, {-0.01, 0.02}, {-0.01, -0.02}};
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const double ra = 0.0, dec = 0.0, l = 0.0, m = 0.0;
  facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m,
                        padding, align, make_square);
  const std::vector<Pixel>& pixels = facet.GetPixels();
  BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
  BOOST_CHECK_EQUAL(pixels[0], Pixel(40, 30));
  BOOST_CHECK_EQUAL(pixels[1], Pixel(40, 70));
  BOOST_CHECK_EQUAL(pixels[2], Pixel(60, 70));
  BOOST_CHECK_EQUAL(pixels[3], Pixel(60, 30));

  return facet;
}

/**
 * Creates a concave facet at given x/y offset, with shape
 *    o   o
 *   / \ / \
 *  o   o   o
 *   \     /
 *    \   /
 *      o
 */
Facet CreateConcaveFacet(int offset_x = 0, int offset_y = 0) {
  Facet facet;
  const std::vector<Coord> coords{{0.02, 0.0},   {0.0, -0.02}, {-0.02, 0.0},
                                  {-0.01, 0.01}, {0.0, 0.0},   {0.01, 0.01}};
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const double ra = offset_x * kScale;
  const double dec = -offset_y * kScale;
  const double l = 0.0;
  const double m = 0.0;
  facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m);
  return facet;
}

/**
 * Creates a pair of connected facets, to check whether the
 * calculated facet-line intersections seamlessly match across facet boundaries
 *  o--- o---------o
 *  |     \        |
 *  |   1  \   2   |
 *  |       \      |
 *  o -------o-----o
 *
 */
std::pair<Facet, Facet> CreateConnectedFacets() {
  Facet facet1, facet2;
  const std::vector<Coord> coords1{
      {0.05, -0.05}, {0.05, -0.043}, {0.043, -0.041}, {0.041, -0.05}};
  const std::vector<Coord> coords2{
      {0.043, -0.041}, {0.034, -0.043}, {0.034, -0.05}, {0.041, -0.05}};

  for (const Coord& c : coords1) {
    facet1.AddVertex(c.ra, c.dec);
  }

  for (const Coord& c : coords2) {
    facet2.AddVertex(c.ra, c.dec);
  }

  const double ra = 0.0;
  const double dec = 0.0;
  const double l = 0.0;
  const double m = 0.0;
  facet1.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m);
  facet2.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m);

  return std::make_pair(facet1, facet2);
}

void CheckBoundingBoxes(const Facet& facet, const Pixel& trimmed_min,
                        const Pixel& trimmed_max, const Pixel& untrimmed_min,
                        const Pixel& untrimmed_max) {
  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Min(), trimmed_min);
  BOOST_CHECK_EQUAL(trimmed_box.Max(), trimmed_max);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  BOOST_CHECK_EQUAL(untrimmed_box.Min(), untrimmed_min);
  BOOST_CHECK_EQUAL(untrimmed_box.Max(), untrimmed_max);
}

void CheckIntersections(const Facet& facet, int y,
                        const std::vector<std::pair<int, int>>& ref) {
  std::vector<std::pair<int, int>> isects = facet.HorizontalIntersections(y);
  BOOST_CHECK_EQUAL(isects.size(), ref.size());
  for (size_t i = 0; i != isects.size(); ++i) {
    BOOST_CHECK_EQUAL(isects[i].first, ref[i].first);
    BOOST_CHECK_EQUAL(isects[i].second, ref[i].second);
  }
}

}  // namespace

BOOST_AUTO_TEST_SUITE(facet)

BOOST_AUTO_TEST_CASE(pixel_operators) {
  const Pixel p1(4, 2);
  const Pixel p2(2, 1);

  BOOST_CHECK(p1 != p2);
  const Pixel p3 = p2 + p2;
  BOOST_CHECK(p1 == p3);
  const Pixel p4 = p3 - p2;
  BOOST_CHECK(p4 == p2);
}

BOOST_AUTO_TEST_CASE(bounding_box_empty) {
  const BoundingBox box;
  BOOST_CHECK_EQUAL(box.Min(), Pixel(0, 0));
  BOOST_CHECK_EQUAL(box.Max(), Pixel(0, 0));
  BOOST_CHECK_EQUAL(box.Width(), 0);
  BOOST_CHECK_EQUAL(box.Height(), 0);
  BOOST_CHECK_EQUAL(box.Centre(), Pixel(0, 0));
}

BOOST_AUTO_TEST_CASE(bounding_box_no_alignment) {
  const std::vector<Pixel> pixels{{-1, -20}, {4, 2}, {0, 5}};
  const BoundingBox box(pixels);
  BOOST_CHECK_EQUAL(box.Min(), Pixel(-1, -20));
  BOOST_CHECK_EQUAL(box.Max(), Pixel(4, 5));
  BOOST_CHECK_EQUAL(box.Width(), 5);
  BOOST_CHECK_EQUAL(box.Height(), 25);
  BOOST_CHECK_EQUAL(box.Centre(),
                    Pixel(1, -7));  // Centre is rounded towards 0.
}

BOOST_AUTO_TEST_CASE(bounding_box_aligned) {
  const std::vector<Pixel> pixels{{-1, -20}, {4, 2}, {0, 5}};
  const size_t align = 4;
  const BoundingBox box(pixels, align);
  BOOST_CHECK_EQUAL(box.Min(), Pixel(-2, -21));
  BOOST_CHECK_EQUAL(box.Max(), Pixel(6, 7));
  BOOST_CHECK_EQUAL(box.Width() % align, 0);
  BOOST_CHECK_EQUAL(box.Height() % align, 0);
  BOOST_CHECK_EQUAL(box.Centre(), Pixel(2, -7));
}

BOOST_AUTO_TEST_CASE(point_in_bounding_box) {
  const std::vector<Pixel> pixels{{0, 0}, {0, 10}, {10, 10}, {10, 0}};
  const BoundingBox box(pixels);

  BOOST_CHECK(box.Contains(Pixel(5, 5)));
  BOOST_CHECK(!box.Contains(Pixel(10, 10)));
}

BOOST_AUTO_TEST_CASE(polygon_empty_intersection) {
  const std::vector<Pixel> poly1{{0, 0}, {0, 10}, {10, 10}, {10, 0}};
  const std::vector<Pixel> poly2{{11, 11}, {11, 20}, {20, 20}, {20, 11}};
  // PolygonIntersection should only be invoked in case there is one
  // intersection
  BOOST_CHECK_THROW(Facet::PolygonIntersection(poly1, poly2),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(polygon_one_intersection) {
  // Ordering for poly1 and poly2 is on purpose clockwise and anti-clockwise,
  // respectively
  const std::vector<Pixel> poly1{{0, 0}, {0, 10}, {10, 10}, {10, 0}};
  const std::vector<Pixel> poly2{{5, 5}, {15, 5}, {15, 15}, {5, 15}};
  std::vector<Pixel> poly3 = Facet::PolygonIntersection(poly1, poly2);
  BOOST_CHECK_EQUAL(poly3[0], Pixel(5, 10));
  BOOST_CHECK_EQUAL(poly3[1], Pixel(10, 10));
  BOOST_CHECK_EQUAL(poly3[2], Pixel(10, 5));
  BOOST_CHECK_EQUAL(poly3[3], Pixel(5, 5));
}

BOOST_AUTO_TEST_CASE(polygon_two_intersections) {
  const std::vector<Pixel> poly1{{0, 0}, {0, 10}, {10, 10}, {2, 5}, {10, 0}};
  const std::vector<Pixel> poly2{{5, 0}, {5, 10}, {15, 10}, {15, 0}};
  // Intersection would result in two polygons, which is not allowed
  BOOST_CHECK_THROW(Facet::PolygonIntersection(poly1, poly2),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(constructor) {
  Facet facet;

  // BOOST_CHECK_EQUAL(facet.RA(), );
  BOOST_CHECK(std::isnan(facet.RA()));
  // BOOST_CHECK_EQUAL(facet.Dec(), 0.0);
  BOOST_CHECK(std::isnan(facet.Dec()));
  BOOST_CHECK(facet.GetPixels().empty());
  BOOST_CHECK(facet.DirectionLabel().empty());

  CheckBoundingBoxes(facet, Pixel(0, 0), Pixel(0, 0), Pixel(0, 0), Pixel(0, 0));
  BOOST_CHECK_THROW(facet.CalculatePixels(0.0, 0.0, kScale, kScale, kImageSize,
                                          kImageSize, 0.0, 0.0),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(pixel_positions) {
  Facet facet;

  const std::vector<Coord> coords{{0, 0}, {0, 0.02}, {0.02, 0.02}, {0.02, 0}};
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  {
    const double ra = 0.0, dec = 0.0, l = 0.0, m = 0.0;
    facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l,
                          m);
    const std::vector<Pixel>& pixels = facet.GetPixels();
    BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
    BOOST_CHECK_EQUAL(pixels[0], Pixel(50, 50));
    BOOST_CHECK_EQUAL(pixels[1], Pixel(50, 70));
    BOOST_CHECK_EQUAL(pixels[2], Pixel(30, 70));
    BOOST_CHECK_EQUAL(pixels[3], Pixel(30, 50));
  }

  {
    const double ra = 0.0, dec = 0.0, l = 0.01, m = 0.02;
    facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l,
                          m);
    const std::vector<Pixel>& pixels = facet.GetPixels();
    BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
    BOOST_CHECK_EQUAL(pixels[0], Pixel(60, 30));
    BOOST_CHECK_EQUAL(pixels[1], Pixel(60, 50));
    BOOST_CHECK_EQUAL(pixels[2], Pixel(40, 50));
    BOOST_CHECK_EQUAL(pixels[3], Pixel(40, 30));
  }

  {
    const double ra = 0.02, dec = 0.01, l = 0.0, m = 0.0;
    facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l,
                          m);
    const std::vector<Pixel>& pixels = facet.GetPixels();
    BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
    BOOST_CHECK_EQUAL(pixels[0], Pixel(70, 40));
    BOOST_CHECK_EQUAL(pixels[1], Pixel(70, 60));
    BOOST_CHECK_EQUAL(pixels[2], Pixel(50, 60));
    BOOST_CHECK_EQUAL(pixels[3], Pixel(50, 40));
  }
}

// Create a diamond facet in the center, so no clipping occurs.
BOOST_AUTO_TEST_CASE(no_clipping) {
  Facet facet = CreateDiamondFacet();

  const std::vector<Pixel>& pixels = facet.GetPixels();
  BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
  BOOST_CHECK_EQUAL(pixels[0], Pixel(30, 50));
  BOOST_CHECK_EQUAL(pixels[1], Pixel(50, 60));
  BOOST_CHECK_EQUAL(pixels[2], Pixel(70, 50));
  BOOST_CHECK_EQUAL(pixels[3], Pixel(50, 40));

  // Check facet centroid
  const Pixel centroid = facet.Centroid();
  BOOST_CHECK_EQUAL(centroid, Pixel(50, 50));
  BOOST_CHECK_EQUAL(facet.RA(), 0.0);
  BOOST_CHECK_EQUAL(facet.Dec(), 0.0);
}

// Create a diamond in the top right corner of the image.
BOOST_AUTO_TEST_CASE(clip_top_right) {
  const int offset_x = 50;
  const int offset_y = offset_x;
  Facet facet = CreateDiamondFacet(offset_x, offset_y);

  const std::vector<Pixel>& pixels = facet.GetPixels();
  // Facet clipped to triangle
  BOOST_REQUIRE_EQUAL(pixels.size(), 3u);
  BOOST_CHECK_EQUAL(pixels[0], Pixel(100, 90));
  BOOST_CHECK_EQUAL(pixels[1], Pixel(80, 100));
  BOOST_CHECK_EQUAL(pixels[2], Pixel(100, 100));
  Pixel centroid_ref((pixels[0].x + pixels[1].x + pixels[2].x) / 3,
                     (pixels[0].y + pixels[1].y + pixels[2].y) / 3);
  BOOST_CHECK_EQUAL(facet.Centroid(), centroid_ref);

  // Manually convert centroid to ra,dec coords, and check
  const double phase_centre_ra = offset_x * kScale;
  const double phase_centre_dec = -offset_y * kScale;
  double l;
  double m;
  double ra;
  double dec;
  aocommon::ImageCoordinates::XYToLM(facet.Centroid().x, facet.Centroid().y,
                                     kScale, kScale, kImageSize, kImageSize, l,
                                     m);

  aocommon::ImageCoordinates::LMToRaDec(l, m, phase_centre_ra, phase_centre_dec,
                                        ra, dec);
  BOOST_CHECK_CLOSE(facet.RA(), ra, 1e-6);
  BOOST_CHECK_CLOSE(facet.Dec(), dec, 1e-6);
}

// Create a diamond in the bottom left corner of the image.
BOOST_AUTO_TEST_CASE(clip_bottom_left) {
  const bool set_custom_direction = true;
  Facet facet = CreateDiamondFacet(-50, -50, set_custom_direction);

  const std::vector<Pixel>& pixels = facet.GetPixels();
  // Facet clipped to triangle
  BOOST_REQUIRE_EQUAL(pixels.size(), 3u);
  BOOST_CHECK_EQUAL(pixels[0], Pixel(0, 10));
  BOOST_CHECK_EQUAL(pixels[1], Pixel(20, 0));
  BOOST_CHECK_EQUAL(pixels[2], Pixel(0, 0));
  Pixel centroid_ref((pixels[0].x + pixels[1].x + pixels[2].x) / 3,
                     (pixels[0].y + pixels[1].y + pixels[2].y) / 3);
  BOOST_CHECK_EQUAL(facet.Centroid(), centroid_ref);
  // Custom (ra, dec) direction reproduced?
  BOOST_CHECK_EQUAL(facet.RA(), -0.02);
  BOOST_CHECK_EQUAL(facet.Dec(), 0.01);
}

BOOST_AUTO_TEST_CASE(point_in_polygon) {
  Facet facet = CreateDiamondFacet();

  BOOST_CHECK(facet.Contains(Pixel(50, 50)));
  // Pixel on edge that is owned by the facet
  BOOST_CHECK(facet.Contains(Pixel(30, 50)));
  // Pixel on edge that is not owned by the facet
  BOOST_CHECK(!facet.Contains(Pixel(50, 60)));
  BOOST_CHECK(!facet.Contains(Pixel(70, 50)));
  // Pixel outside facet
  BOOST_CHECK(!facet.Contains(Pixel(29, 50)));
  BOOST_CHECK(!facet.Contains(Pixel(50, 61)));
}

BOOST_AUTO_TEST_CASE(facet_bounding_boxes) {
  // Invalid padding
  BOOST_CHECK_THROW(CreateRectangularFacet(0.99, 1, false),
                    std::invalid_argument);

  // Invalid alignment
  BOOST_CHECK_THROW(CreateRectangularFacet(1.0, 42, false),
                    std::invalid_argument);

  // No padding, no alignment, no squaring.
  CheckBoundingBoxes(CreateRectangularFacet(1.0, 1, false), Pixel(40, 30),
                     Pixel(60, 70), Pixel(40, 30), Pixel(60, 70));

  // Only enable padding.
  CheckBoundingBoxes(CreateRectangularFacet(1.5, 1, false), Pixel(40, 30),
                     Pixel(60, 70), Pixel(35, 20), Pixel(65, 80));

  // Enable alignment, facet should not change
  CheckBoundingBoxes(CreateRectangularFacet(1.0, 4, false), Pixel(40, 30),
                     Pixel(60, 70), Pixel(40, 30), Pixel(60, 70));

  // Only enable squaring.
  CheckBoundingBoxes(CreateRectangularFacet(1.0, 1, true), Pixel(30, 30),
                     Pixel(70, 70), Pixel(30, 30), Pixel(70, 70));

  // Enable everything and use a non-power-of-two alignment.
  CheckBoundingBoxes(CreateRectangularFacet(1.5, 25, true), Pixel(25, 25),
                     Pixel(75, 75), Pixel(13, 13), Pixel(88, 88));
}

BOOST_AUTO_TEST_CASE(horizontal_intersections_no_pixels) {
  const Facet facet;
  CheckIntersections(facet, 0, {});
  CheckIntersections(facet, 42, {});
}

BOOST_AUTO_TEST_CASE(horizontal_intersections_rectangle) {
  Facet facet = CreateRectangularFacet(1.0, 1, false);

  // Specified y is below the facet.
  CheckIntersections(facet, 20, {});

  // Specified y is at the bottom of the facet or intersects the facet.
  for (int y = 30; y < 70; ++y) {
    CheckIntersections(facet, y, {std::make_pair(40, 60)});
  }

  // Specified y is at the top of the facet.
  // Since the facet ranges *until* the top, the result should be empty.
  CheckIntersections(facet, 70, {});

  // Specified y is above the facet.
  CheckIntersections(facet, 90, {});
}

BOOST_AUTO_TEST_CASE(horizontal_intersections_diamond) {
  // See the clip_large_box test for the pixel coordinates of the diamond.
  Facet facet = CreateDiamondFacet();

  // Specified y is below the facet.
  CheckIntersections(facet, 20, {});

  // Specified y is at the bottom of the facet.
  // Result is empty (0, 0), since only valid intersections are half-open
  // intervals.
  CheckIntersections(facet, 40, {});

  // Specified y intersects at 1/4 of the facet height.
  CheckIntersections(facet, 45, {std::make_pair(40, 60)});

  // Specified y intersects middle of the facet.
  CheckIntersections(facet, 50, {std::make_pair(30, 70)});

  // Specified y intersects at 3/4 of the facet height.
  CheckIntersections(facet, 55, {std::make_pair(40, 60)});

  // Specified y is at the top of the facet.
  // Since the facet ranges *until* the top, the result should be empty.
  CheckIntersections(facet, 60, {});

  // Specified y is above the facet.
  CheckIntersections(facet, 90, {});
}

BOOST_AUTO_TEST_CASE(horizontal_intersections_clipped_facet) {
  // Create diamond facet in lower left corner, with pixels extending
  // beyond image boundaries, resulting in a clipped, triangular facet
  Facet facet = CreateDiamondFacet(-50, -50);

  // Specified y is at the top of the facet.
  // Since the facet ranges *until* the top, the result should be empty.
  CheckIntersections(facet, 10, {});

  // Specified y is at the bottom of the facet.
  CheckIntersections(facet, 0, {std::make_pair(0, 20)});

  // Specified y intersects at half the facet height
  CheckIntersections(facet, 5, {std::make_pair(0, 10)});
}

BOOST_AUTO_TEST_CASE(horizontal_intersections_concave_facet) {
  Facet facet = CreateConcaveFacet();
#ifdef HAVE_BOOST_LT_166
  // Intersections for concave facet not supported
  BOOST_CHECK_THROW(CheckIntersections(facet, 30, {}), std::runtime_error);
#else
  // Specified y is at bottom of facet
  CheckIntersections(facet, 30, {});

  // Specified y cuts halfway through "convex" part
  CheckIntersections(facet, 40, {std::make_pair(40, 60)});

  // Just before "concavity" starts
  CheckIntersections(facet, 50, {std::make_pair(30, 70)});

  // Halfway "concave facets"
  CheckIntersections(facet, 55,
                     {std::make_pair(35, 45), std::make_pair(55, 65)});

  // Just below max-y vertices
  CheckIntersections(facet, 59,
                     {std::make_pair(39, 41), std::make_pair(59, 61)});

  // Empty at top
  CheckIntersections(facet, 60, {});
  BOOST_CHECK_EQUAL(facet.Centroid(), Pixel(50, 46));
#endif
}

BOOST_AUTO_TEST_CASE(check_facet_alignment) {
  std::pair<Facet, Facet> facets = CreateConnectedFacets();

  for (int y = 0; y != 8; ++y) {
    std::vector<std::pair<int, int>> isects1 =
        facets.first.HorizontalIntersections(y);
    std::vector<std::pair<int, int>> isects2 =
        facets.second.HorizontalIntersections(y);

    BOOST_CHECK_EQUAL(isects1[0].first, 0u);
    BOOST_CHECK_EQUAL(isects2[0].second, 16u);
    // Checks continuity between facets
    BOOST_CHECK_EQUAL(isects1[0].second, isects2[0].first);
  }
}

namespace {
void CheckEncapsulates(const BoundingBox& untrimmed_box,
                       const BoundingBox& trimmed_box) {
  BOOST_CHECK_LE(untrimmed_box.Min().x, trimmed_box.Min().x);
  BOOST_CHECK_GE(untrimmed_box.Max().x, trimmed_box.Max().x);
  BOOST_CHECK_LE(untrimmed_box.Min().y, trimmed_box.Min().y);
  BOOST_CHECK_GE(untrimmed_box.Max().y, trimmed_box.Max().y);
}

void CheckEncapsulates(const BoundingBox& trimmed_box, const Pixel& min_coord,
                       const Pixel& max_coord, int image_size) {
  // Checks are slightly counter-intuitive, but due to the
  // coordinate system conventions. See documentation for CalculatePixels
  BOOST_CHECK_LE(trimmed_box.Min().x, -min_coord.x + image_size / 2);
  BOOST_CHECK_LE(trimmed_box.Min().y, min_coord.y + image_size / 2);
  BOOST_CHECK_GE(trimmed_box.Max().x, -max_coord.x + image_size / 2);
  BOOST_CHECK_GE(trimmed_box.Max().y, max_coord.y + image_size / 2);
}
}  // namespace

BOOST_AUTO_TEST_CASE(square_bounding_box) {
  /**
   * These and the next 4 auto test cases check the make_square=true option. In
   * particular they make sure that if a bounding box requires extra padding to
   * make it square-shaped whether a) the bounding boxes still really
   * encapsulate all coordinates; b) the bounding boxes are still properly
   * aligned; and c) the bounding boxes are square shaped.
   */
  const Pixel min_coord(0, 2);
  const Pixel mid_coord(5, 5);
  const Pixel max_coord(10, 8);
  const std::vector<Coord> coords{{min_coord.x * kScale, min_coord.y * kScale},
                                  {mid_coord.x * kScale, mid_coord.y * kScale},
                                  {max_coord.x * kScale, max_coord.y * kScale}};
  Facet facet;
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const double l = 0.0;
  const double m = 0.0;
  const double ra = 0.0;
  const double dec = 0.0;
  const bool make_square = true;
  const double padding = 1.4;
  const size_t align = 4;
  const int image_size = 100;
  facet.CalculatePixels(ra, dec, kScale, kScale, image_size, image_size, l, m,
                        padding, align, make_square);

  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Width(), trimmed_box.Height());
  CheckEncapsulates(trimmed_box, min_coord, max_coord, image_size);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  CheckEncapsulates(untrimmed_box, trimmed_box);
  BOOST_CHECK_EQUAL(untrimmed_box.Width(), untrimmed_box.Height());
  BOOST_CHECK_EQUAL(untrimmed_box.Width() % 4, 0);
}

BOOST_AUTO_TEST_CASE(square_bounding_box_near_right_edge) {
  const Pixel min_coord(-49, -49);
  const Pixel max_coord(-48, 2);
  const std::vector<Coord> coords{{min_coord.x * kScale, min_coord.y * kScale},
                                  {min_coord.x * kScale, max_coord.y * kScale},
                                  {max_coord.x * kScale, max_coord.y * kScale}};
  Facet facet;
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const bool make_square = true;
  const double padding = 1.4;
  const size_t align = 4;
  const int image_size = 100;
  facet.CalculatePixels(0.0, 0.0, kScale, kScale, image_size, image_size, 0.0,
                        0.0, padding, align, make_square);

  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Width(), trimmed_box.Height());
  CheckEncapsulates(trimmed_box, min_coord, max_coord, image_size);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  CheckEncapsulates(untrimmed_box, trimmed_box);
  BOOST_CHECK_EQUAL(untrimmed_box.Width(), untrimmed_box.Height());
  BOOST_CHECK_EQUAL(untrimmed_box.Width() % 4, 0);
}

BOOST_AUTO_TEST_CASE(square_bounding_box_near_bottom_edge) {
  const Pixel min_coord(2, -49);
  const Pixel max_coord(49, -48);
  const std::vector<Coord> coords{{min_coord.x * kScale, min_coord.y * kScale},
                                  {max_coord.x * kScale, min_coord.y * kScale},
                                  {max_coord.x * kScale, max_coord.y * kScale}};
  Facet facet;
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const bool make_square = true;
  const double padding = 1.4;
  const size_t align = 4;
  const int image_size = 100;
  facet.CalculatePixels(0.0, 0.0, kScale, kScale, image_size, image_size, 0.0,
                        0.0, padding, align, make_square);

  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Width(), trimmed_box.Height());
  CheckEncapsulates(trimmed_box, min_coord, max_coord, image_size);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  CheckEncapsulates(untrimmed_box, trimmed_box);
  BOOST_CHECK_EQUAL(untrimmed_box.Width(), untrimmed_box.Height());
  BOOST_CHECK_EQUAL(untrimmed_box.Width() % 4, 0);
}

BOOST_AUTO_TEST_CASE(square_bounding_box_near_left_edge) {
  const Pixel min_coord(48, 2);
  const Pixel max_coord(50, 49);
  const std::vector<Coord> coords{{min_coord.x * kScale, min_coord.y * kScale},
                                  {max_coord.x * kScale, min_coord.y * kScale},
                                  {max_coord.x * kScale, max_coord.y * kScale}};
  Facet facet;
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const bool make_square = true;
  const double padding = 1.4;
  const size_t align = 4;
  const int image_size = 100;
  facet.CalculatePixels(0.0, 0.0, kScale, kScale, image_size, image_size, 0.0,
                        0.0, padding, align, make_square);

  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Width(), trimmed_box.Height());
  CheckEncapsulates(trimmed_box, min_coord, max_coord, image_size);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  CheckEncapsulates(untrimmed_box, trimmed_box);
  BOOST_CHECK_EQUAL(untrimmed_box.Width(), untrimmed_box.Height());
  BOOST_CHECK_EQUAL(untrimmed_box.Width() % 4, 0);
}

BOOST_AUTO_TEST_CASE(square_bounding_box_near_top_edge) {
  const Pixel min_coord(2, 48);
  const Pixel max_coord(49, 50);
  const std::vector<Coord> coords{{min_coord.x * kScale, min_coord.y * kScale},
                                  {min_coord.x * kScale, max_coord.y * kScale},
                                  {max_coord.x * kScale, max_coord.y * kScale}};
  Facet facet;
  for (const Coord& c : coords) {
    facet.AddVertex(c.ra, c.dec);
  }

  const bool make_square = true;
  const double padding = 1.4;
  const size_t align = 4;
  const int image_size = 100;
  facet.CalculatePixels(0.0, 0.0, kScale, kScale, image_size, image_size, 0.0,
                        0.0, padding, align, make_square);

  const BoundingBox& trimmed_box = facet.GetTrimmedBoundingBox();
  BOOST_CHECK_EQUAL(trimmed_box.Width(), trimmed_box.Height());
  CheckEncapsulates(trimmed_box, min_coord, max_coord, image_size);

  const BoundingBox& untrimmed_box = facet.GetUntrimmedBoundingBox();
  CheckEncapsulates(untrimmed_box, trimmed_box);
  BOOST_CHECK_EQUAL(untrimmed_box.Width(), untrimmed_box.Height());
  BOOST_CHECK_EQUAL(untrimmed_box.Width() % 4, 0);
}

BOOST_AUTO_TEST_CASE(serialization) {
  // Since a facet contains Coord, Boundingbox and Pixel objects, this test
  // covers serialization of those objects, too.

  const double kRa = 42.0;
  const double kDec = 43.0;
  const std::string kDirection = "FirstLeftThenRight";

  // Create a facet with different bounding boxes. See facet_bounding_boxes
  // test.
  Facet input = CreateRectangularFacet(1.5, 25, true);
  BOOST_CHECK_EQUAL(input.RA(), 0.0);
  BOOST_CHECK_EQUAL(input.Dec(), 0.0);
  input.SetRA(kRa);
  input.SetDec(kDec);
  input.SetDirectionLabel(kDirection);
  aocommon::SerialOStream ostr;
  input.Serialize(ostr);

  aocommon::SerialIStream istr(std::move(ostr));
  Facet output;
  output.Unserialize(istr);

  BOOST_REQUIRE_EQUAL(output.GetCoords().size(), 4u);
  BOOST_REQUIRE_EQUAL(output.GetPixels().size(), 4u);
  for (size_t i = 0; i < input.GetCoords().size(); ++i) {
    BOOST_CHECK_EQUAL(input.GetCoords()[i].ra, output.GetCoords()[i].ra);
    BOOST_CHECK_EQUAL(input.GetCoords()[i].dec, output.GetCoords()[i].dec);
    BOOST_CHECK_EQUAL(input.GetPixels()[i], output.GetPixels()[i]);
  }
  BOOST_CHECK_EQUAL(output.RA(), kRa);
  BOOST_CHECK_EQUAL(output.Dec(), kDec);
  BOOST_CHECK_EQUAL(input.Centroid(), output.Centroid());
  BOOST_CHECK_EQUAL(output.DirectionLabel(), kDirection);
  BOOST_CHECK_EQUAL(input.GetTrimmedBoundingBox().Min(),
                    output.GetTrimmedBoundingBox().Min());
  BOOST_CHECK_EQUAL(input.GetTrimmedBoundingBox().Max(),
                    output.GetTrimmedBoundingBox().Max());
  BOOST_CHECK_EQUAL(input.GetUntrimmedBoundingBox().Min(),
                    output.GetUntrimmedBoundingBox().Min());
  BOOST_CHECK_EQUAL(input.GetUntrimmedBoundingBox().Max(),
                    output.GetUntrimmedBoundingBox().Max());
}

BOOST_AUTO_TEST_SUITE_END()
