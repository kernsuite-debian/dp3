// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include "facetimage.h"
#include "tfacet.h"

#include <aocommon/uvector.h>

#include <iostream>

using schaapcommon::facets::BoundingBox;
using schaapcommon::facets::Facet;
using schaapcommon::facets::FacetImage;
using schaapcommon::facets::Pixel;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(facetimage)

namespace {

const double kScale = 0.001;
const size_t kImageSize = 100;
const size_t kNTerms = 2;
const double kPadding = 1.25;
const size_t kAlign = 4;

/// Creates a facet with pixels (0, 0), (size, 0), (size, size), (0, size).
Facet CreateSquareFacet(size_t size) {
  Facet facet;

  const double radecZero = kScale * kImageSize / 2;
  const double radecSize = radecZero - size * kScale;
  facet.AddVertex(radecZero, -radecZero);
  facet.AddVertex(radecSize, -radecZero);
  facet.AddVertex(radecSize, -radecSize);
  facet.AddVertex(radecZero, -radecSize);

  const double ra = 0.0, dec = 0.0, l = 0.0, m = 0.0;
  facet.CalculatePixels(ra, dec, kScale, kScale, kImageSize, kImageSize, l, m,
                        kPadding, kAlign, true);
  const std::vector<Pixel>& pixels = facet.GetPixels();
  BOOST_REQUIRE_EQUAL(pixels.size(), 4u);
  BOOST_CHECK_EQUAL(pixels[0], Pixel(0, 0));
  BOOST_CHECK_EQUAL(pixels[1], Pixel(size, 0));
  BOOST_CHECK_EQUAL(pixels[2], Pixel(size, size));
  BOOST_CHECK_EQUAL(pixels[3], Pixel(0, size));

  return facet;
}

/**
 * Fill the data for a facet with a value.
 * Use a different value in the padding area.
 */
void FillFacetData(float* data, const Facet& facet, float value) {
  const float pad_value = -42.0f;

  const BoundingBox unpadded_box(facet.GetPixels());
  const BoundingBox& padded_box = facet.GetUntrimmedBoundingBox();

  // Check that the padded box is larger on all sides.
  BOOST_REQUIRE_LT(padded_box.Min().x, unpadded_box.Min().x);
  BOOST_REQUIRE_LT(padded_box.Min().y, unpadded_box.Min().y);
  BOOST_REQUIRE_GT(padded_box.Max().x, unpadded_box.Max().x);
  BOOST_REQUIRE_GT(padded_box.Max().y, unpadded_box.Max().y);

  const int pad_top = unpadded_box.Min().y - padded_box.Min().y;
  for (int y = 0; y < pad_top; ++y) {
    std::fill_n(data, padded_box.Width(), pad_value);
    data += padded_box.Width();
  }

  const int pad_left = unpadded_box.Min().x - padded_box.Min().x;
  const int pad_right = padded_box.Max().x - unpadded_box.Max().x;
  for (int y = unpadded_box.Min().y; y < unpadded_box.Max().y; ++y) {
    std::fill_n(data, pad_left, pad_value);
    data += pad_left;
    std::fill_n(data, unpadded_box.Width(), value);
    data += unpadded_box.Width();
    std::fill_n(data, pad_right, pad_value);
    data += pad_right;
  }

  const int pad_bottom = padded_box.Max().y - unpadded_box.Max().y;
  for (int y = 0; y < pad_bottom; ++y) {
    std::fill_n(data, padded_box.Width(), pad_value);
    data += padded_box.Width();
  }
}

}  // namespace

BOOST_AUTO_TEST_CASE(constructor) {
  FacetImage fi(kImageSize, kImageSize, kNTerms);
  BOOST_CHECK_EQUAL(fi.Width(), 0);
  BOOST_CHECK_EQUAL(fi.Height(), 0);
  BOOST_CHECK_EQUAL(fi.OffsetX(), 0);
  BOOST_CHECK_EQUAL(fi.OffsetY(), 0);

  // When SetFacet was not called, CopyToFacet should throw.
  std::vector<aocommon::UVector<float>> input_images;
  for (std::size_t i = 0; i < kNTerms; ++i) {
    input_images.emplace_back(kImageSize * kImageSize);
  }
  BOOST_CHECK_THROW(fi.CopyToFacet(input_images), std::runtime_error);

  // When SetFacet was not called, AddToImage should throw.
  std::vector<float*> output_images(kNTerms);
  BOOST_CHECK_THROW(fi.AddToImage(output_images), std::runtime_error);

  BOOST_CHECK_THROW(FacetImage(0, kImageSize, kNTerms), std::invalid_argument);
  BOOST_CHECK_THROW(FacetImage(kImageSize, 0, kNTerms), std::invalid_argument);
  BOOST_CHECK_THROW(FacetImage(kImageSize, kImageSize, 0),
                    std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(set_facet) {
  const Facet facet = CreateSquareFacet(40);
  BOOST_CHECK_EQUAL(facet.GetTrimmedBoundingBox().Min(), Pixel(0, 0));
  BOOST_CHECK_EQUAL(facet.GetTrimmedBoundingBox().Max(), Pixel(40, 40));

  // Padding increases the facet to -5...45.
  // Alignment rounds -5 down to -6 and 45 up to 46, yielding (46 -- 6) % 4 = 0
  BOOST_CHECK_EQUAL(facet.GetUntrimmedBoundingBox().Min(), Pixel(-6, -6));
  BOOST_CHECK_EQUAL(facet.GetUntrimmedBoundingBox().Max(), Pixel(46, 46));

  FacetImage fi(kImageSize, kImageSize, kNTerms);
  // Trimmed box
  fi.SetFacet(facet, true);
  BOOST_CHECK_EQUAL(fi.Width(), 40);
  BOOST_CHECK_EQUAL(fi.Height(), 40);
  BOOST_CHECK_EQUAL(fi.OffsetX(), 0);
  BOOST_CHECK_EQUAL(fi.OffsetY(), 0);

  // Untrimmed, padded box
  fi.SetFacet(facet, false);
  BOOST_CHECK_EQUAL(fi.Width(), 52);
  BOOST_CHECK_EQUAL(fi.Height(), 52);
  BOOST_CHECK_EQUAL(fi.OffsetX(), -6);
  BOOST_CHECK_EQUAL(fi.OffsetY(), -6);
}

BOOST_AUTO_TEST_CASE(copy_to_facet) {
  const size_t kFacetSize = 30;

  const Facet facet = CreateSquareFacet(kFacetSize);

  std::vector<aocommon::UVector<float>> input_images;
  for (size_t term = 0; term < kNTerms; ++term) {
    aocommon::UVector<float> image(kImageSize * kImageSize);
    for (size_t y = 0; y < kImageSize; ++y) {
      for (size_t x = 0; x < kImageSize; ++x) {
        image[y * kImageSize + x] = x + y + term + 0.5;
      }
    }
    input_images.push_back(std::move(image));
  }

  FacetImage facet_image(kImageSize, kImageSize, kNTerms);
  facet_image.SetFacet(facet, false);
  facet_image.CopyToFacet(input_images);

  for (size_t term = 0; term < kNTerms; ++term) {
    const float* data = facet_image.Data(term);
    for (int y = facet.GetUntrimmedBoundingBox().Min().y;
         y < facet.GetUntrimmedBoundingBox().Max().y; ++y) {
      for (int x = facet.GetUntrimmedBoundingBox().Min().x;
           x < facet.GetUntrimmedBoundingBox().Max().x; ++x) {
        // Values in the padding and alignment areas should be zero.
        float ref_value = 0.0;
        if (x >= 0 && x < static_cast<int>(kFacetSize) && y >= 0 &&
            y < static_cast<int>(kFacetSize)) {
          ref_value = x + y + term + 0.5;
        }
        BOOST_CHECK_CLOSE(*data, ref_value, 1e-8);
        ++data;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(multiply) {
  const size_t kFacetSize = 30;

  const Facet facet = CreateSquareFacet(kFacetSize);

  std::vector<aocommon::UVector<float>> input_images(kNTerms);
  for (size_t term = 0; term < kNTerms; ++term) {
    input_images[term].assign(kImageSize * kImageSize, term + 1.0f);
  }

  FacetImage facet_image(kImageSize, kImageSize, kNTerms);
  facet_image.SetFacet(facet, false);
  facet_image.CopyToFacet(input_images);

  BOOST_CHECK_NO_THROW(facet_image *= 2.0);

  for (size_t term = 0; term < kNTerms; ++term) {
    const float* data = facet_image.Data(term);
    for (int y = facet.GetUntrimmedBoundingBox().Min().y;
         y < facet.GetUntrimmedBoundingBox().Max().y; ++y) {
      for (int x = facet.GetUntrimmedBoundingBox().Min().x;
           x < facet.GetUntrimmedBoundingBox().Max().x; ++x) {
        // Values in the padding and alignment areas should be zero.
        float ref_value = 0.0;
        if (x >= 0 && x < static_cast<int>(kFacetSize) && y >= 0 &&
            y < static_cast<int>(kFacetSize)) {
          ref_value = 2.0f * (term + 1.0f);
        }
        BOOST_CHECK_CLOSE(*data, ref_value, 1e-8);
        ++data;
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(add_facet_to_image) {
  // Properties of the main image:
  const size_t image_width = kImageSize * 2;
  const size_t image_height = kImageSize;
  const double dl = 0.002;
  const double dm = dl;
  const double phase_centre_ra = 0.0;
  const double phase_centre_dec = 0.0;
  const double shift_l = 0.0;
  const double shift_m = 0.0;

  // Make a facet in the lower left corner and upper right corner
  // (please note coord system conventions!)
  Facet facet1;
  Facet facet2;

  std::vector<std::pair<double, double>> coords{
      {0.1, -0.05}, {0.05, -0.05}, {0.05, -0.03}, {0.1, -0.03}};

  for (const auto& coord : coords) {
    facet1.AddVertex(coord.first, coord.second);
    // Mirror in origin
    facet2.AddVertex(-1 * coord.first, -1 * coord.second);
  }

  // Calculate pixel position of facet vertices in main image
  facet1.CalculatePixels(phase_centre_ra, phase_centre_dec, dl, dm, image_width,
                         image_height, shift_l, shift_m, kPadding, kAlign,
                         false);
  facet2.CalculatePixels(phase_centre_ra, phase_centre_dec, dl, dm, image_width,
                         image_height, shift_l, shift_m, kPadding, kAlign,
                         true);

  std::vector<std::vector<float>> output_images;
  std::vector<float*> output_ptrs;
  for (size_t term = 0; term < kNTerms; ++term) {
    output_images.emplace_back(image_width * image_height, 0.0f);
    output_ptrs.push_back(output_images.back().data());
  }

  FacetImage facet_image(image_width, image_height, kNTerms);

  facet_image.SetFacet(facet1, false);
  // Initialize facet1 with odd numbers.
  for (size_t term = 0; term < kNTerms; ++term) {
    FillFacetData(facet_image.Data(term), facet1, 1 + term * 2);
  }
  facet_image.AddToImage(output_ptrs);

  facet_image.SetFacet(facet2, false);
  // Initialize facet2 with even numbers.
  for (size_t term = 0; term < kNTerms; ++term) {
    FillFacetData(facet_image.Data(term), facet2, 2 + term * 2);
  }
  facet_image.AddToImage(output_ptrs);

  // Create unpadded bounding boxes for the facets: AddToImage should not use
  // data in the padding area.
  const BoundingBox box1(facet1.GetPixels());
  const BoundingBox box2(facet2.GetPixels());

  for (size_t term = 0; term < kNTerms; ++term) {
    for (int y = 0; y < static_cast<int>(image_height); ++y) {
      for (int x = 0; x < static_cast<int>(image_width); ++x) {
        const float result_pixel = output_images[term][y * image_width + x];
        if (y >= box1.Min().y && y < box1.Max().y && x >= box1.Min().x &&
            x < box1.Max().x) {
          // Check if "facet1" value was assigned.
          BOOST_CHECK_CLOSE(result_pixel, 1 + term * 2, 1e-8);
        } else if (y >= box2.Min().y && y < box2.Max().y && x >= box2.Min().x &&
                   x < box2.Max().x) {
          // Check if "facet2" value was assigned.
          BOOST_CHECK_CLOSE(result_pixel, 2 + term * 2, 1e-8);
        } else {
          // Check if the value is still zero.
          BOOST_CHECK_CLOSE(result_pixel, 0.0f, 1e-8);
        }
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(multiply_image_inside_facet) {
  // Properties of the main image:
  const size_t image_width = kImageSize * 2;
  const size_t image_height = kImageSize;
  const double dl = 0.002;
  const double dm = dl;
  const double phase_centre_ra = 0.0;
  const double phase_centre_dec = 0.0;
  const double shift_l = 0.0;
  const double shift_m = 0.0;

  // Make a facet in the lower left corner and upper right corner
  // (please note coord system conventions!)
  Facet facet1;
  Facet facet2;

  std::vector<std::pair<double, double>> coords{
      {0.1, -0.05}, {0.05, -0.05}, {0.05, -0.03}, {0.1, -0.03}};

  for (const auto& coord : coords) {
    facet1.AddVertex(coord.first, coord.second);
    // Mirror in origin
    facet2.AddVertex(-1 * coord.first, -1 * coord.second);
  }

  // Calculate pixel position of facet vertices in main image
  facet1.CalculatePixels(phase_centre_ra, phase_centre_dec, dl, dm, image_width,
                         image_height, shift_l, shift_m, kPadding, kAlign,
                         false);
  facet2.CalculatePixels(phase_centre_ra, phase_centre_dec, dl, dm, image_width,
                         image_height, shift_l, shift_m, kPadding, kAlign,
                         true);

  std::vector<std::vector<float>> output_images;
  std::vector<float*> output_ptrs;
  for (size_t term = 0; term < kNTerms; ++term) {
    output_images.emplace_back(image_width * image_height, 1.0f);
    output_ptrs.push_back(output_images.back().data());
  }

  FacetImage facet_image(image_width, image_height, kNTerms);

  BOOST_CHECK_THROW(facet_image.MultiplyImageInsideFacet(output_ptrs, 1.0f),
                    std::runtime_error);

  facet_image.SetFacet(facet1, false);
  BOOST_CHECK_NO_THROW(facet_image.MultiplyImageInsideFacet(output_ptrs, 2.0f));

  facet_image.SetFacet(facet2, false);
  BOOST_CHECK_NO_THROW(facet_image.MultiplyImageInsideFacet(output_ptrs, 4.0f));

  // Create unpadded bounding boxes for the facets: AddToImage should not use
  // data in the padding area.
  const BoundingBox box1(facet1.GetPixels());
  const BoundingBox box2(facet2.GetPixels());

  for (size_t term = 0; term < kNTerms; ++term) {
    for (int y = 0; y < static_cast<int>(image_height); ++y) {
      for (int x = 0; x < static_cast<int>(image_width); ++x) {
        const float result_pixel = output_images[term][y * image_width + x];
        if (y >= box1.Min().y && y < box1.Max().y && x >= box1.Min().x &&
            x < box1.Max().x) {
          // Check if "facet1" value was assigned.
          BOOST_CHECK_CLOSE(result_pixel, 2.0f, 1e-8);
        } else if (y >= box2.Min().y && y < box2.Max().y && x >= box2.Min().x &&
                   x < box2.Max().x) {
          // Check if "facet2" value was assigned.
          BOOST_CHECK_CLOSE(result_pixel, 4.0f, 1e-8);
        } else {
          // Check if the value is still one.
          BOOST_CHECK_CLOSE(result_pixel, 1.0f, 1e-8);
        }
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
