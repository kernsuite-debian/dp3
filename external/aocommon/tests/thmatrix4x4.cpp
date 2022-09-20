#ifndef TEST_HMATRIX4X4_H
#define TEST_HMATRIX4X4_H

#include <boost/test/unit_test.hpp>

#include <aocommon/hmatrix4x4.h>
#include <aocommon/matrix2x2.h>
#include <aocommon/matrix4x4.h>

using aocommon::HMC4x4;
using aocommon::MC2x2;
using aocommon::MC4x4;
using aocommon::Vector4;

#define CHECK_CLOSE_MESSAGE(VAL, REF, MSG)         \
  BOOST_CHECK_MESSAGE(std::fabs(VAL - REF) < 1e-6, \
                      MSG << " is " << VAL << ", should be " << REF);

BOOST_AUTO_TEST_SUITE(hmatrix4x4)

template <typename Matrix>
static void CheckMatrix(const Matrix& result, const Matrix& groundtruth) {
  for (size_t i = 0; i != 16; ++i) {
    BOOST_CHECK_CLOSE(result[i].real(), groundtruth[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(result[i].imag(), groundtruth[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(zero) {
  HMC4x4 zero = HMC4x4::Zero();
  HMC4x4 ref{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
             0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  CheckMatrix(zero, ref);
  CheckMatrix(zero.ToMatrix(), MC4x4::Zero());
}

BOOST_AUTO_TEST_CASE(unit) {
  HMC4x4 unit = HMC4x4::Unit();
  HMC4x4 ref{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
             0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
  CheckMatrix(unit, ref);
  CheckMatrix(unit.ToMatrix(), MC4x4::Unit());
}

BOOST_AUTO_TEST_CASE(buffer) {
  HMC4x4 ref = HMC4x4::Unit();
  std::array<double, 16> buffer;
  for (size_t i = 0; i != buffer.size(); ++i) {
    buffer[i] = ref.Data(i);
  }
  HMC4x4 unit(buffer.data());
  CheckMatrix(unit, ref);
  CheckMatrix(unit.ToMatrix(), MC4x4::Unit());
}

BOOST_AUTO_TEST_CASE(inversion) {
  HMC4x4 m1(HMC4x4::Unit());
  BOOST_CHECK(m1.Invert());
  CheckMatrix(m1, HMC4x4::Unit());

  HMC4x4 m2(HMC4x4::Unit() * 2);
  BOOST_CHECK(m2.Invert());
  CheckMatrix(m2, HMC4x4::Unit() * 0.5);
  BOOST_CHECK(m2.Invert());
  CheckMatrix(m2, HMC4x4::Unit() * 2.0);

  HMC4x4 m3;
  BOOST_CHECK(!m3.Invert());
}

BOOST_AUTO_TEST_CASE(from_data) {
  // Identity matrix
  HMC4x4 m = HMC4x4::FromData({1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
                               0.0, 0.0, 0.0, 0.0, 0.0, 1.0});
  CheckMatrix(m, HMC4x4::Unit());
}

BOOST_AUTO_TEST_CASE(indexing1) {
  HMC4x4 m{1.0, 2.0, 4.0, 7.0, 2.0, 3.0, 5.0, 8.0,
           4.0, 5.0, 6.0, 9.0, 7.0, 8.0, 9.0, 10.0};
  const double vals[16] = {1.0, 2.0, 4.0, 7.0, 2.0, 3.0, 5.0, 8.0,
                           4.0, 5.0, 6.0, 9.0, 7.0, 8.0, 9.0, 10.0};
  for (size_t i = 0; i != 16; ++i) {
    BOOST_CHECK_CLOSE(m[i].real(), vals[i], 1e-6);
    BOOST_CHECK_CLOSE(m[i].imag(), 0.0, 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(indexing2) {
  std::complex<double> j(0.0, 1.0);
  HMC4x4 m{1.0, 2.0 - j, 4.0, 7.0, 2.0 + j, 3.0,     5.0, 8.0 + j,
           4.0, 5.0,     6.0, 9.0, 7.0,     8.0 - j, 9.0, 10.0};
  const std::complex<double> vals[16] = {1.0, 2.0 - j, 4.0, 7.0, 2.0 + j, 3.0,
                                         5.0, 8.0 + j, 4.0, 5.0, 6.0,     9.0,
                                         7.0, 8.0 - j, 9.0, 10.0};
  for (size_t i = 0; i != 16; ++i) {
    CHECK_CLOSE_MESSAGE(m[i].real(), vals[i].real(), "Real element " << i);
    CHECK_CLOSE_MESSAGE(m[i].imag(), vals[i].imag(), "Imag element " << i);
  }
}

BOOST_AUTO_TEST_CASE(scalar_product) {
  HMC4x4 ref = HMC4x4{2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0,
                      0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0};
  // scalar multiplication
  CheckMatrix(HMC4x4::Unit() * 2.0, ref);
  // scalar multiplication-assignment
  HMC4x4 m = HMC4x4::Unit();
  m *= 2.0;
  CheckMatrix(m, ref);
}

BOOST_AUTO_TEST_CASE(scalar_division) {
  HMC4x4 m = HMC4x4{2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0,
                    0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0};
  // scalar division-assignment
  m /= 2.0;
  CheckMatrix(m, HMC4x4::Unit());
}

BOOST_AUTO_TEST_CASE(product_with_vector4) {
  Vector4 v1(2.0, 2.0, 2.0, 2.0);
  Vector4 res = HMC4x4::Unit() * v1;
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(res[i].real(), 2.0, 1e-6);
    BOOST_CHECK_CLOSE(res[i].imag(), 0.0, 1e-6);
  }
  Vector4 v2(std::complex<double>(2.0, 3.0), std::complex<double>(4.0, 5.0),
             std::complex<double>(5.0, 6.0), std::complex<double>(7.0, 8.0));
  res = HMC4x4::Unit() * 0.5 * v2;
  Vector4 ref = MC4x4::Unit() * 0.5 * v2;
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(res[i].real(), ref[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(res[i].imag(), ref[i].imag(), 1e-6);
  }
  std::complex<double> j(0.0, 1.0);
  MC4x4 m{1.0,           2.0 + 1.0 * j, 3.0 + 2.0 * j, 4.0 + 3.0 * j,
          2.0 - 1.0 * j, 2.0,           3.0 + 2.0 * j, 4.0 + 2.0 * j,
          3.0 - 2.0 * j, 3.0 - 2.0 * j, 3.0,           4.0 - 3.0 * j,
          4.0 - 3.0 * j, 4.0 - 2.0 * j, 4.0 + 3.0 * j, 4.0};

  res = HMC4x4(m) * v2;
  ref = m * v2;
  for (size_t i = 0; i != 4; ++i) {
    CHECK_CLOSE_MESSAGE(res[i].real(), ref[i].real(), "Element " << i);
    CHECK_CLOSE_MESSAGE(res[i].imag(), ref[i].imag(), "Element " << i);
  }
}

static void checkKroneckerProduct(const MC2x2& a, const MC2x2& x,
                                  const MC2x2& b) {
  Vector4 ref = a.Multiply(x).MultiplyHerm(b).Vec();
  HMC4x4 product = HMC4x4::KroneckerProduct(b.HermTranspose().Transpose(), a);
  Vector4 v = product * x.Vec();
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(v[i].real(), ref[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(v[i].imag(), ref[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(kronecker_product_a) {
  checkKroneckerProduct(MC2x2::Unity(), MC2x2::Unity(), MC2x2::Unity());
}

BOOST_AUTO_TEST_CASE(kronecker_product_b) {
  MC2x2 a1{1.0, 2.0, 2.0, 4.0}, x1(MC2x2::Unity()), b1{1.0, 2.0, 2.0, 4.0};
  checkKroneckerProduct(a1, x1, b1);
}

BOOST_AUTO_TEST_CASE(kronecker_product_c) {
  MC2x2 a3{0.0, 1.0, 1.0, 3.0}, x3{0.0, 1.0, 2.0, 3.0}, b3{0.0, 1.0, 1.0, 3.0};
  checkKroneckerProduct(a3, x3, b3);
}

BOOST_AUTO_TEST_CASE(kronecker_product_d) {
  std::complex<double> x(8, 2), y(6, 3), xc = std::conj(x), yc = std::conj(y);
  MC2x2 a4{0.0, 2.0 * y, 2.0 * yc, 3.0}, x4{1.0, 2.0 * xc, 2.0 * x, 4.0},
      b4{1.0, 3.0 * x, 3.0 * xc, 4.0};
  checkKroneckerProduct(a4, x4, b4);
}

BOOST_AUTO_TEST_CASE(norm) {
  BOOST_CHECK_CLOSE((HMC4x4::Unit() * 2.0).Norm(), (MC4x4::Unit() * 2.0).Norm(),
                    1e-6);
  std::complex<double> j(0, 1);
  HMC4x4 m{
      1.0,  2.0 + 3.0 * j,   4.0 - 5.0 * j,   6.0 + 7.0 * j,   2.0 - 3.0 * j,
      8.0,  9.0 + 10.0 * j,  11.0 - 12.0 * j, 4.0 + 5.0 * j,   9.0 - 10.0 * j,
      13.0, 14.0 + 15.0 * j, 6.0 - 7.0 * j,   11.0 + 12.0 * j, 11.0 + 12.0 * j,
      16.0};
  BOOST_CHECK_CLOSE(m.Norm(), m.ToMatrix().Norm(), 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
