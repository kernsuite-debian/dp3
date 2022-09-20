// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCHAAPCOMMON_FACETS_FACETIMAGE_H
#define SCHAAPCOMMON_FACETS_FACETIMAGE_H

#include "facet.h"

#include <aocommon/uvector.h>

namespace schaapcommon {
namespace facets {

/**
 * This class allows copying image data between a main image and facets of that
 * main image. It has an internal data buffer for storing the facet image.
 */
class FacetImage {
 public:
  /**
   * @brief Construct a new Facet Image object.
   *
   * @param image_width Width of the main image, in pixels.
   * @param image_height Height of the main image, in pixels.
   * @param nr_spectral_terms Number of spectral terms to use.
   * @throw std::invalid_argument If image_width, image_height or
   *        nr_spectral_terms is zero.
   */
  FacetImage(size_t image_width, size_t image_height, size_t nr_spectral_terms);

  /**
   * Update the facet geometry.
   * This function initializes the FacetImage for using the given facet.
   * Call this function before calling any other functions.
   *
   * @param facet The facet geometry. This facet should have a properly padded
   *        and aligned bounding box. This bounding box may extend beyond the
   *        borders of the main image due to padding or alignment.
   *        All horizontal intersections should be within the mainimage, though.
   * @param trimmed True: Use trimmed facet bounding box. False: Use untrimmed
   *        facet bounding box.
   */
  void SetFacet(const Facet& facet, bool trimmed);

  /**
   * @return The width of the facet.
   */
  size_t Width() const { return box_.Width(); }

  /**
   * @return The height of the facet.
   */
  size_t Height() const { return box_.Height(); }

  /**
   * @return The number of pixels that this facet is displaced in X direction,
   * relative to origin of main image.
   */
  int OffsetX() const { return box_.Min().x; }

  /**
   * @return The number of pixels that this facet is displaced in Y direction,
   * relative to origin of main image.
   */
  int OffsetY() const { return box_.Min().y; }

  /**
   * @return The facet image data for the given spectral frequency term.
   */
  const float* Data(size_t spectral_term) const {
    return data_[spectral_term].data();
  }

  /**
   * @return The facet image data for the given spectral frequency term.
   */
  float* Data(size_t spectral_term) { return data_[spectral_term].data(); }

  /**
   * @brief Multiply facet image data for all spectral terms with a scalar.
   */
  FacetImage& operator*=(float factor);

  /**
   * @brief Fill the facet buffer with image data.
   *
   * @param images Vector of images.
   *        The outer vector length should match the number of spectral terms.
   *        The inner vector size should should match the image width and
   *        image height values passed to the constructor.
   * @throw std::invalid_argument if 'images' has an incorrect size.
   * @throw std::runtime_error if SetFacet was not called.
   */
  void CopyToFacet(const std::vector<aocommon::UVector<float>>& images);

  /**
   * @brief Fill the facet buffer with the image data given as a buffer
   *
   * @param images Vector of buffered images.
   *        The vector length should match the number of spectral terms.
   *        The buffer is assumed to match the image width and image height
   *        values passed to the constructor
   * @throw std::runtime_error if SetFacet was not called.
   */
  void CopyToFacet(const std::vector<const float*>& images);

  /**
   * @brief Add data from the facet buffer to main images.
   *
   * @param images Vector of images.
   *        The vector length should match the number of spectral terms.
   *        The width and height of each image should match the values passed
   *        to the constructor.
   * @throw std::runtime_error if SetFacet was not called.
   */
  void AddToImage(const std::vector<float*>& images) const;

  /**
   * @brief Multiply the image values enclosed by the facet with scalar valued
   * @param factor
   *
   * @param images Vector of images.
   *         The vector length should match the number of spectral terms.
   *        The width and height of each image should match the values passed
   *        to the constructor.
   * @param factor Scalar used for multiplying the image data inside the facet.
   */
  void MultiplyImageInsideFacet(std::vector<float*>& images,
                                float factor) const;

 private:
  /**
   * @brief Pre-calculate the horizontal intersections (xmin, xmax)
   * of for each y-coordinate row in the facet bounding box with the
   * main image.
   *
   * @param facet Facet
   */
  void SetHorizontalIntersections(const Facet& facet);

  /**
   * A vector with one element for each spectral frequency term.
   * Each inner vector holds the data for a facet image.
   */
  std::vector<std::vector<float>> data_;
  const size_t image_width_;   ///< Width of the main image.
  const size_t image_height_;  ///< Height of the main image.
  BoundingBox box_;  ///< Facet bounding box, including padding/alignment.

  /**
   * For each y value in box_, a vector with start and end x coordinate in the
   * main image.
   */
  std::vector<std::vector<std::pair<int, int>>> horizontal_intersections_;
};
}  // namespace facets
}  // namespace schaapcommon
#endif
