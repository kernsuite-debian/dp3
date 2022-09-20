// Copyright (C) 2021 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#include "facetimage.h"

#include <algorithm>
#include <functional>

namespace schaapcommon {
namespace facets {

FacetImage::FacetImage(size_t image_width, size_t image_height,
                       size_t nr_spectral_terms)
    : data_(nr_spectral_terms),
      image_width_(image_width),
      image_height_(image_height),
      box_(),
      horizontal_intersections_() {
  if (nr_spectral_terms == 0 || image_width == 0 || image_height == 0) {
    throw std::invalid_argument("Zero terms/width/height in FacetImage()");
  }
}

void FacetImage::SetFacet(const Facet& facet, bool trimmed) {
  box_ =
      trimmed ? facet.GetTrimmedBoundingBox() : facet.GetUntrimmedBoundingBox();

  const size_t data_size = box_.Width() * box_.Height();
  for (std::vector<float>& values : data_) {
    values.assign(data_size, 0.0);
  }

  SetHorizontalIntersections(facet);
}

FacetImage& FacetImage::operator*=(float factor) {
  for (size_t term = 0; term != data_.size(); ++term) {
    for (auto& val : data_[term]) val *= factor;
  }
  return *this;
}

void FacetImage::CopyToFacet(
    const std::vector<aocommon::UVector<float>>& images) {
  std::vector<const float*> images_ptr;
  for (const aocommon::UVector<float>& image : images) {
    if (image.size() != image_width_ * image_height_) {
      throw std::invalid_argument(
          "Image size does not match the specified width and height.");
    }
    images_ptr.push_back(image.data());
  }
  CopyToFacet(images_ptr);
}

void FacetImage::CopyToFacet(const std::vector<const float*>& images) {
  if (images.size() != data_.size()) {
    throw std::invalid_argument(
        "Image term count does not match facet term count.");
  }

  if (data_.front().empty()) {
    throw std::runtime_error(
        "Facet data buffer is not initialized. Call SetFacet() first.");
  }

  for (size_t term = 0; term != images.size(); ++term) {
    float* data_y = data_[term].data();
    const float* image_y = images[term] + OffsetY() * image_width_;

    for (const std::vector<std::pair<int, int>>& intersections :
         horizontal_intersections_) {
      for (const std::pair<int, int>& intersection : intersections) {
        std::copy_n(image_y + intersection.first,
                    intersection.second - intersection.first,
                    data_y + intersection.first - OffsetX());
      }
      data_y += Width();
      image_y += image_width_;
    }
  }
}

void FacetImage::AddToImage(const std::vector<float*>& images) const {
  if (images.size() != data_.size()) {
    throw std::invalid_argument(
        "Image term count does not match facet term count.");
  }

  if (data_.front().empty()) {
    throw std::runtime_error(
        "Facet data buffer is not initialized. Call SetFacet() first.");
  }

  for (size_t term = 0; term != data_.size(); ++term) {
    const float* data_y = data_[term].data();
    float* image_y = images[term] + OffsetY() * image_width_;

    for (const std::vector<std::pair<int, int>>& intersections :
         horizontal_intersections_) {
      for (const std::pair<int, int>& intersection : intersections) {
        const float* facet_begin = data_y + intersection.first - OffsetX();
        const float* facet_end = data_y + intersection.second - OffsetX();
        float* image_begin = image_y + intersection.first;

        // Do an addition-assignment
        std::transform(facet_begin, facet_end, image_begin, image_begin,
                       std::plus<float>());
      }
      data_y += Width();
      image_y += image_width_;
    }
  }
}

void FacetImage::MultiplyImageInsideFacet(std::vector<float*>& images,
                                          float factor) const {
  if (images.size() != data_.size()) {
    throw std::invalid_argument(
        "Image term count does not match facet term count.");
  }

  if (horizontal_intersections_.empty()) {
    throw std::runtime_error(
        "Horizontal intersections are not initialized. Call SetFacet() first.");
  }

  for (size_t term = 0; term != data_.size(); ++term) {
    float* image_y = images[term] + OffsetY() * image_width_;

    for (const std::vector<std::pair<int, int>>& intersections :
         horizontal_intersections_) {
      for (const std::pair<int, int>& intersection : intersections) {
        float* image_begin = image_y + intersection.first;
        float* image_end = image_y + intersection.second;
        // Multiply range with factor
        std::for_each(image_begin, image_end,
                      [factor](float& pixel) { pixel *= factor; });
      }
      image_y += image_width_;
    }
  }
}

void FacetImage::SetHorizontalIntersections(const Facet& facet) {
  // Pre-calculate horizontal intersections for each y value.
  // Check that all intersections are within the main image.
  horizontal_intersections_.clear();
  horizontal_intersections_.reserve(box_.Height());
  for (int y = box_.Min().y; y != box_.Max().y; ++y) {
    auto isects = facet.HorizontalIntersections(y);
    for (const std::pair<int, int>& isect : isects) {
      if (isect.first < 0 || isect.second > static_cast<int>(image_width_) ||
          y < 0 || y > static_cast<int>(image_height_)) {
        throw std::invalid_argument("Facet does not fit in main image.");
      }
    }
    horizontal_intersections_.push_back(std::move(isects));
  }
}

}  // namespace facets
}  // namespace schaapcommon