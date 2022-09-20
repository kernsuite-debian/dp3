#ifndef TEST_MATRIX4X4_H
#define TEST_MATRIX4X4_H

#include <boost/test/unit_test.hpp>

#include <aocommon/matrix4x4.h>

using aocommon::Matrix4x4;
using aocommon::MC2x2;
using aocommon::MC4x4;
using aocommon::Vector4;

BOOST_AUTO_TEST_SUITE(matrix4x4)

static void CheckMatrix(const Matrix4x4& result, const Matrix4x4& groundtruth) {
  for (size_t i = 0; i != 16; ++i) {
    BOOST_CHECK_CLOSE(result[i].real(), groundtruth[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(result[i].imag(), groundtruth[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(construction) {
  CheckMatrix(MC4x4(), MC4x4{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                             0.0, 0.0, 0.0, 0.0, 0.0, 0.0});

  // Constructor must take 16 values if a list is given:
  BOOST_CHECK_THROW(MC4x4({3, 4}), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(unit) {
  MC4x4 unit = MC4x4::Unit();
  MC4x4 ref{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
  CheckMatrix(unit, ref);
}

BOOST_AUTO_TEST_CASE(inversion) {
  MC4x4 m1(MC4x4::Unit());
  BOOST_CHECK(m1.Invert());
  CheckMatrix(m1, MC4x4::Unit());

  MC4x4 m2(MC4x4::Unit() * 2);
  BOOST_CHECK(m2.Invert());
  CheckMatrix(m2, MC4x4::Unit() * 0.5);
  BOOST_CHECK(m2.Invert());
  CheckMatrix(m2, MC4x4::Unit() * 2.0);

  MC4x4 m3;
  BOOST_CHECK(!m3.Invert());
}

static void checkKroneckerProduct(const MC2x2& a, const MC2x2& x,
                                  const MC2x2& b) {
  Vector4 ref = a.Multiply(x).MultiplyHerm(b).Vec();
  MC4x4 product = MC4x4::KroneckerProduct(b.HermTranspose().Transpose(), a);
  Vector4 v = product * x.Vec();
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(v[i].real(), ref[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(v[i].imag(), ref[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(kronecker_product) {
  checkKroneckerProduct(MC2x2::Unity(), MC2x2::Unity(), MC2x2::Unity());

  MC2x2 a1{1.0, 2.0, 2.0, 4.0}, x1(MC2x2::Unity()), b1{1.0, 2.0, 2.0, 4.0};
  checkKroneckerProduct(a1, x1, b1);

  MC2x2 a2{0.0, 1.0, 2.0, 3.0}, x2(MC2x2::Unity()), b2{0.0, 1.0, 2.0, 3.0};
  checkKroneckerProduct(a2, x2, b2);

  MC2x2 a3{0.0, 1.0, 2.0, 3.0}, x3{0.0, 1.0, 2.0, 3.0}, b3{0.0, 1.0, 2.0, 3.0};
  checkKroneckerProduct(a3, x3, b3);

  std::complex<double> x(8, 2), y(6, 3);
  MC2x2 a4{0.0, 1.0 * y, 2.0 * x, 3.0 * y},
      x4{1.0 * y, 2.0 * x, 3.0 * x, 4.0 * y},
      b4{1.0 * x, 2.0 * x, 3.0 * x, 4.0 * y};
  checkKroneckerProduct(a4, x4, b4);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
