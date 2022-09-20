// Copyright (C) 2020 ASTRON (Netherlands Institute for Radio Astronomy)
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCHAAPCOMMON_FACETS_DS9FACETFILE_H
#define SCHAAPCOMMON_FACETS_DS9FACETFILE_H

#include "facet.h"

#include <cmath>
#include <fstream>
#include <vector>

namespace schaapcommon {
namespace facets {

/**
 * Class for reading facets from a DS9 region file.
 *
 * Facets are specified using "polygons", where the (ra,dec)
 * coordinates of the polygon vertices are specified sequentially, i.e.
 *
 * \code{.unparsed}
 * polygon(ra_0, dec_0, ..., ..., ra_n, dec_n)
 * \endcode
 *
 * Note that the (ra,dec) coordinates should be given in degrees!
 *
 * In addition, each polygon can be equipped with text labels and/or a specific
 * point of interest. The text label should be specified on the same line as the
 * \c polygon to which it should be attached and should be preceded by a \c #,
 * e.g.:
 *
 * \code{.unparsed}
 * polygon(ra_0, dec_0, ..., ..., ra_n, dec_n) # text="ABCD"
 * \endcode
 *
 * A \c point can be attached to a polygon to mark a specific point of interest.
 * Similar to the polygon definition, the coordinates are provided in (ra,dec)
 * in degrees. A point should be placed on a new line, following the polygon
 * definition, i.e.:
 *
 * \code{.unparsed}
 * polygon(ra_0, dec_0, ..., ..., ra_n, dec_n) # text="ABCD"
 * point(ra_A,dec_A)
 * \endcode
 *
 * Only one point can be attached per polygon. In case multiple points were
 * specified, the last one will be used. So in the following example:
 *
 * \code{.unparsed}
 * polygon(ra_0, dec_0, ..., ..., ra_n, dec_n) # text="ABCD"
 * point(ra_A,dec_A)
 * point(ra_B,dec_B)
 * \endcode
 * the \c point(ra_A,dec_A) is ignored and \c point(ra_B,dec_B) will be used.
 */
class DS9FacetFile {
 public:
  enum class TokenType { kEmpty, kWord, kNumber, kSymbol, kComment };

  /**
   * @brief Construct a new DS9FacetFile object
   *
   * @param filename path to DS9 region file
   */
  DS9FacetFile(const std::string& filename)
      : file_(filename), has_char_(false) {
    if (!file_) {
      throw std::runtime_error("Error reading " + filename);
    }
    Skip();
  }

  /**
   * Read the facets from the file. Be aware that this does not
   * set the pixel values (x, y) of the vertices, see
   * @ref Facet::CalculatePixelPositions().
   */
  std::vector<Facet> Read() {
    std::vector<Facet> facets;
    while (Type() != TokenType::kEmpty) {
      std::string t = Token();
      if (t == "global" || t == "fk5") {
        SkipLine();
      } else if (Type() == TokenType::kComment) {
        Skip();
      } else if (Type() == TokenType::kWord) {
        Skip();

        if (t == "polygon") {
          facets.emplace_back();
          ReadPolygon(facets.back());
        } else if (t == "point" && !facets.empty()) {
          ReadPoint(facets.back());
        }
      }
    }
    return facets;
  }

  std::vector<std::shared_ptr<Facet>> ReadShared() {
    std::vector<std::shared_ptr<Facet>> facets;
    while (Type() != TokenType::kEmpty) {
      std::string t = Token();
      if (t == "global" || t == "fk5") {
        SkipLine();
      } else if (Type() == TokenType::kComment) {
        Skip();
      } else if (Type() == TokenType::kWord) {
        Skip();

        if (t == "polygon") {
          facets.emplace_back(std::make_shared<Facet>());
          ReadPolygon(*facets.back());
        } else if (t == "point" && !facets.empty()) {
          ReadPoint(*facets.back());
        }
      }
    }
    return facets;
  }

  /**
   * Take a comment as input e.g. text={direction} and retrieves direction.
   */
  static std::string ParseDirectionLabel(TokenType type,
                                         const std::string& comment) {
    const std::string classifier = "text=";
    std::string dir = "";

    if (type == TokenType::kComment &&
        comment.find(classifier) != std::string::npos) {
      dir = comment.substr(comment.find(classifier) + classifier.length(),
                           comment.length());
      // Remove trailing parts
      dir = dir.substr(0, dir.find(","))
                .substr(0, dir.find(" "))
                .substr(0, dir.find("\n"));
    }

    return dir;
  }

 private:
  void ReadPolygon(Facet& facet) {
    std::vector<double> vals = ReadNumList();
    if (vals.size() % 2 != 0)
      throw std::runtime_error(
          "Polygon is expecting an even number of numbers in its list");
    std::vector<double>::const_iterator i = vals.begin();
    while (i != vals.end()) {
      const double ra = *i * (M_PI / 180.0);
      ++i;
      const double dec = *i * (M_PI / 180.0);
      ++i;
      facet.AddVertex(ra, dec);
    }

    facet.SetDirectionLabel(ParseDirectionLabel(Type(), Token()));
  }

  void ReadPoint(Facet& facet) {
    std::vector<double> vals = ReadNumList();
    if (vals.size() != 2) {
      throw std::runtime_error(
          "Point is expecting exactly two numbers in its list");
    }
    facet.SetRA(vals[0] * (M_PI / 180.0));
    facet.SetDec(vals[1] * (M_PI / 180.0));
  }

  std::vector<double> ReadNumList() {
    std::vector<double> vals;
    if (Token() != "(")
      throw std::runtime_error("Expecting '(' after polygon keyword");
    Skip();
    while (Token() != ")") {
      if (Type() != TokenType::kNumber)
        throw std::runtime_error("Expected number or ')' after '(' ");
      vals.push_back(atof(Token().c_str()));
      Skip();
      if (Token() == ",") Skip();
    }
    Skip();
    return vals;
  }

  std::string Token() const { return token_; }

  TokenType Type() const { return type_; }

  void SkipLine() {
    char c;
    while (NextChar(c)) {
      if (c == '\n') break;
    }
    Skip();
  }

  void Skip() {
    bool cont = true;
    type_ = TokenType::kEmpty;
    token_ = std::string();
    do {
      char c;
      if (NextChar(c)) {
        switch (type_) {
          case TokenType::kEmpty:
            if (IsAlpha(c)) {
              type_ = TokenType::kWord;
              token_ += c;
            } else if (IsWhiteSpace(c)) {
            } else if (IsNumeric(c)) {
              type_ = TokenType::kNumber;
              token_ += c;
            } else if (c == '(' || c == ')' || c == ',') {
              type_ = TokenType::kSymbol;
              token_ += c;
              cont = false;
            } else if (c == '#') {
              type_ = TokenType::kComment;
            }
            break;
          case TokenType::kWord:
            if (IsAlpha(c) || (c >= '0' && c <= '9')) {
              token_ += c;
            } else {
              cont = false;
              PushChar(c);
            }
            break;
          case TokenType::kNumber:
            if (IsNumeric(c)) {
              token_ += c;
            } else {
              cont = false;
              PushChar(c);
            }
            break;
          case TokenType::kSymbol:
            PushChar(c);
            cont = false;
            break;
          case TokenType::kComment:
            if (c == '\n') {
              cont = false;
            } else {
              token_ += c;
            }
            break;
        }
      } else {
        cont = false;
      }
    } while (cont);
  }

  bool NextChar(char& c) {
    if (has_char_) {
      has_char_ = false;
      c = char_;
      return true;
    } else {
      file_.read(&c, 1);
      return file_.good();
    }
  }
  void PushChar(char c) {
    has_char_ = true;
    char_ = c;
  }

  std::ifstream file_;
  std::string token_;
  TokenType type_;
  bool has_char_;
  char char_;

  constexpr static bool IsAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
  }
  constexpr static bool IsWhiteSpace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
  }
  constexpr static bool IsNumeric(char c) {
    return (c >= '0' && c <= '9') || c == '-' || c == '.';
  }
};
}  // namespace facets
}  // namespace schaapcommon

#endif  // SCHAAPCOMMON_FACETS_DS9FACETFILE_H
