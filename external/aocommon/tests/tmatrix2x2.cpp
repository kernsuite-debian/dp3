#include <boost/test/unit_test.hpp>

#include <aocommon/matrix2x2.h>
#include <aocommon/matrix2x2diag.h>

#include <iostream>

using aocommon::Matrix2x2;
using aocommon::MC2x2;
using aocommon::MC2x2F;

BOOST_AUTO_TEST_SUITE(matrix2x2)

BOOST_AUTO_TEST_CASE(from_diagonal) {
  const aocommon::MC2x2Diag a({1.0, 2.0}, {3.0, 4.0});
  MC2x2 b(a);
  BOOST_CHECK_CLOSE(b[0].real(), 1, 1e-6);
  BOOST_CHECK_CLOSE(b[0].imag(), 2, 1e-6);
  BOOST_CHECK_CLOSE(b[1].real(), 0, 1e-6);
  BOOST_CHECK_CLOSE(b[1].imag(), 0, 1e-6);
  BOOST_CHECK_CLOSE(b[2].real(), 0, 1e-6);
  BOOST_CHECK_CLOSE(b[2].imag(), 0, 1e-6);
  BOOST_CHECK_CLOSE(b[3].real(), 3, 1e-6);
  BOOST_CHECK_CLOSE(b[3].imag(), 4, 1e-6);
}

BOOST_AUTO_TEST_CASE(complex_times_real) {
  MC2x2 a({1.0, 2.0}, {0, 0}, {0, 0}, {3.0, 4.0});
  // Flattened real 2x2 array
  double r[4] = {10, 20, 30, 40};

  // Multiply
  MC2x2 b = a * r;
  BOOST_CHECK_CLOSE(b[0].real(), 10, 1e-6);
  BOOST_CHECK_CLOSE(b[0].imag(), 20, 1e-6);
  BOOST_CHECK_CLOSE(b[1].real(), 20, 1e-6);
  BOOST_CHECK_CLOSE(b[1].imag(), 40, 1e-6);
  BOOST_CHECK_CLOSE(b[2].real(), 90, 1e-6);
  BOOST_CHECK_CLOSE(b[2].imag(), 120, 1e-6);
  BOOST_CHECK_CLOSE(b[3].real(), 120, 1e-6);
  BOOST_CHECK_CLOSE(b[3].imag(), 160, 1e-6);

  // Multiply-assign
  a *= r;
  BOOST_CHECK_CLOSE(a[0].real(), 10, 1e-6);
  BOOST_CHECK_CLOSE(a[0].imag(), 20, 1e-6);
  BOOST_CHECK_CLOSE(a[1].real(), 20, 1e-6);
  BOOST_CHECK_CLOSE(a[1].imag(), 40, 1e-6);
  BOOST_CHECK_CLOSE(a[2].real(), 90, 1e-6);
  BOOST_CHECK_CLOSE(a[2].imag(), 120, 1e-6);
  BOOST_CHECK_CLOSE(a[3].real(), 120, 1e-6);
  BOOST_CHECK_CLOSE(a[3].imag(), 160, 1e-6);
}

BOOST_AUTO_TEST_CASE(complex_division_with_real) {
  MC2x2F a({4.0, 2.0}, {0, 40}, {12, 16}, {8.0, 4.0});

  // Divide and assign
  a /= 4.0f;
  BOOST_CHECK_CLOSE(a[0].real(), 1, 1e-6);
  BOOST_CHECK_CLOSE(a[0].imag(), 0.5, 1e-6);
  BOOST_CHECK_CLOSE(a[1].real(), 0, 1e-6);
  BOOST_CHECK_CLOSE(a[1].imag(), 10, 1e-6);
  BOOST_CHECK_CLOSE(a[2].real(), 3, 1e-6);
  BOOST_CHECK_CLOSE(a[2].imag(), 4, 1e-6);
  BOOST_CHECK_CLOSE(a[3].real(), 2, 1e-6);
  BOOST_CHECK_CLOSE(a[3].imag(), 1, 1e-6);
}

BOOST_AUTO_TEST_CASE(assign_to) {
  MC2x2 a({1.0, 2.0}, {0, 0}, {0, 0}, {3.0, 4.0});
  std::complex<double> r1[4];

  // Assign to complex double buffer
  a.AssignTo(r1);
  BOOST_CHECK_CLOSE(r1[0].real(), 1, 1e-6);
  BOOST_CHECK_CLOSE(r1[0].imag(), 2, 1e-6);
  BOOST_CHECK_CLOSE(r1[3].real(), 3, 1e-6);
  BOOST_CHECK_CLOSE(r1[3].imag(), 4, 1e-6);

  // Assign to complex float buffer.
  std::complex<float> r2[4];
  a.AssignTo(r2);
  BOOST_CHECK_CLOSE(r2[0].real(), 1, 1e-6);
  BOOST_CHECK_CLOSE(r2[0].imag(), 2, 1e-6);
  BOOST_CHECK_CLOSE(r2[3].real(), 3, 1e-6);
  BOOST_CHECK_CLOSE(r2[3].imag(), 4, 1e-6);
}

BOOST_AUTO_TEST_CASE(eigenvalue1) {
  double unit[4] = {1.0, 0.0, 0.0, 1.0};
  double e1, e2;
  Matrix2x2::EigenValues(unit, e1, e2);
  BOOST_CHECK_CLOSE(e1, 1.0, 1e-6);
  BOOST_CHECK_CLOSE(e2, 1.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(eigenvalue2) {
  double unit[4] = {0.0, 1.0, -2.0, -3.0};
  double e1, e2;
  Matrix2x2::EigenValues(unit, e1, e2);
  if (e1 < e2) std::swap(e1, e2);
  BOOST_CHECK_CLOSE(e1, -1.0, 1e-6);
  BOOST_CHECK_CLOSE(e2, -2.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(eigenvalue3) {
  double unit[4] = {0.0, -2.0, 1.0, -3.0};
  double e1, e2;
  Matrix2x2::EigenValues(unit, e1, e2);
  if (e1 < e2) std::swap(e1, e2);
  BOOST_CHECK_CLOSE(e1, -1.0, 1e-6);
  BOOST_CHECK_CLOSE(e2, -2.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(eigenvalue4) {
  double unit[4] = {0.0, 1.0, -1.0, 0.0};
  double e1, e2;
  Matrix2x2::EigenValues(unit, e1, e2);
  if (e1 < e2) std::swap(e1, e2);
  BOOST_CHECK(!std::isfinite(e1));
  BOOST_CHECK(!std::isfinite(e2));
}

BOOST_AUTO_TEST_CASE(eigenvector2) {
  double unit[4] = {0.0, 1.0, -2.0, -3.0};
  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(unit, e1, e2, vec1, vec2);
  if (e1 < e2) {
    std::swap(e1, e2);
    std::swap(vec1, vec2);
  }
  BOOST_CHECK_CLOSE(e1, -1.0, 1e-6);
  BOOST_CHECK_CLOSE(vec1[0] / vec1[1], -1.0, 1e-6);  // vec1 = c [-1, 1]
  BOOST_CHECK_CLOSE(e2, -2.0, 1e-6);
  BOOST_CHECK_CLOSE(vec2[0] / vec2[1], -0.5, 1e-6);  // vec2 = c [-1, 2]
}

BOOST_AUTO_TEST_CASE(eigenvector3) {
  double unit[4] = {0.0, -2.0, 1.0, -3.0};
  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(unit, e1, e2, vec1, vec2);
  if (e1 < e2) {
    std::swap(e1, e2);
    std::swap(vec1, vec2);
  }
  BOOST_CHECK_CLOSE(e1, -1.0, 1e-6);
  BOOST_CHECK_CLOSE(vec1[0] / vec1[1], 2.0, 1e-6);  // vec1 = c [2, 1]
  BOOST_CHECK_CLOSE(e2, -2.0, 1e-6);
  BOOST_CHECK_CLOSE(vec2[0] / vec2[1], 1.0, 1e-6);  // vec2 = c [1, 1]
}

BOOST_AUTO_TEST_CASE(eigenvector4) {
  double unit[4] = {1.0, 2.0, 3.0, -4.0};
  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(unit, e1, e2, vec1, vec2);
  if (e1 < e2) {
    std::swap(e1, e2);
    std::swap(vec1, vec2);
  }
  BOOST_CHECK_CLOSE(e1, 2.0, 1e-6);
  BOOST_CHECK_CLOSE(vec1[0] / vec1[1], 2.0, 1e-6);  // vec1 = c [2, 1]
  BOOST_CHECK_CLOSE(e2, -5.0, 1e-6);
  BOOST_CHECK_CLOSE(vec2[1] / vec2[0], -3.0, 1e-6);  // vec2 = c [-2, 6]
}

BOOST_AUTO_TEST_CASE(eigenvector5) {
  double m[4] = {1.0, 0.0, 0.0, 0.5};
  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);
  if (e1 < e2) {
    std::swap(e1, e2);
    std::swap(vec1, vec2);
  }
  BOOST_CHECK_CLOSE(e1, 1.0, 1e-6);
  BOOST_CHECK_CLOSE(vec1[1] / vec1[0], 0.0, 1e-6);
  BOOST_CHECK_CLOSE(e2, 0.5, 1e-6);
  BOOST_CHECK_CLOSE(vec2[0] / vec2[1], 0.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(cholesky_real) {
  std::complex<double> matrixA[4] = {1., 2., 2., 13.};
  std::complex<double> matrixB[4] = {1., 2., 2., 13.};
  const std::complex<double> answer[4] = {1., 0., 2., 3.};

  BOOST_CHECK(Matrix2x2::Cholesky(matrixA));
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(matrixA[i].real(), answer[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(matrixA[i].imag(), answer[i].imag(), 1e-6);
  }

  Matrix2x2::UncheckedCholesky(matrixB);
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(matrixB[i].real(), answer[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(matrixB[i].imag(), answer[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(cholesky_complex) {
  std::complex<double> matrixA[4] = {{1., 0.}, {2., -5.}, {2., 5.}, {38., 0.}};
  std::complex<double> matrixB[4] = {{1., 0.}, {2., -5.}, {2., 5.}, {38., 0.}};
  std::complex<double> answer[4] = {{1., 0.}, {0., 0.}, {2., 5.}, {3., 0.}};
  BOOST_CHECK(Matrix2x2::CheckedCholesky(matrixA));
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(matrixA[i].real(), answer[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(matrixA[i].imag(), answer[i].imag(), 1e-6);
  }

  Matrix2x2::UncheckedCholesky(matrixB);
  for (size_t i = 0; i != 4; ++i) {
    BOOST_CHECK_CLOSE(matrixB[i].real(), answer[i].real(), 1e-6);
    BOOST_CHECK_CLOSE(matrixB[i].imag(), answer[i].imag(), 1e-6);
  }
}

BOOST_AUTO_TEST_CASE(cholesky_not_positive) {
  std::complex<double> diag_not_positive[4] = {
      {0., 0.}, {0., 0.}, {0., 0.}, {1., 0.}};  // diagonal not positive
  BOOST_CHECK(!Matrix2x2::CheckedCholesky(diag_not_positive));
  std::complex<double> diag_not_real[4] = {
      {1., 0.}, {0., 0.}, {0., 0.}, {1., 1.}};  // diagonal not real
  BOOST_CHECK(!Matrix2x2::CheckedCholesky(diag_not_real));
  std::complex<double> not_hermitian[4] = {
      {1., 0.}, {1., 0.}, {2., 0.}, {1., 0.}};  // not hermitian
  BOOST_CHECK(!Matrix2x2::CheckedCholesky(not_hermitian));
}

BOOST_AUTO_TEST_CASE(eigen_value_and_vectors_real) {
  double m[] = {4.0, 1.0, 0.0, 4.0};

  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);

  BOOST_CHECK_CLOSE(e1, 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e2, 4.0, 1e-5);

  BOOST_CHECK_CLOSE(vec1[0], -1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1], 0.0, 1e-5);

  BOOST_CHECK_CLOSE(vec2[0], -1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1], 0.0, 1e-5);

  // Of course this is no longer necessary when the above checks
  // are already done, but e.g. signs are actually ambiguous in
  // above equations, so this is the real equation that should hold:
  BOOST_CHECK_CLOSE(m[0] * vec1[0] + m[1] * vec1[1], e1 * vec1[0], 1e-5);
  BOOST_CHECK_CLOSE(m[2] * vec1[0] + m[3] * vec1[1], e1 * vec1[1], 1e-5);
}

BOOST_AUTO_TEST_CASE(eigen_value_and_vectors_complex) {
  std::complex<double> m[] = {
      std::complex<double>(4.0, 1.0), std::complex<double>(1.0, 0.0),
      std::complex<double>(0.0, 0.0), std::complex<double>(4.0, 1.0)};

  std::complex<double> e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);

  BOOST_CHECK_CLOSE(e1.real(), 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e1.imag(), 1.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.real(), 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.imag(), 1.0, 1e-5);

  BOOST_CHECK_CLOSE(vec1[0].real(), -1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[0].imag(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1].real(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1].imag(), 0.0, 1e-5);

  BOOST_CHECK_CLOSE(vec2[0].real(), -1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[0].imag(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1].real(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1].imag(), 0.0, 1e-5);

  BOOST_CHECK_LT(std::abs(m[0] * vec1[0] + m[1] * vec1[1] - e1 * vec1[0]),
                 1e-5);
  BOOST_CHECK_LT(std::abs(m[2] * vec1[0] + m[3] * vec1[1] - e1 * vec1[1]),
                 1e-5);
}

BOOST_AUTO_TEST_CASE(eigen_value_order_real) {
  // Test a specific case for which the eigen vector order
  // is "ambiguous". vec1 should always be associated with
  // e1, and vec2 with e2.
  // vec1 = { 0 , 1 }
  // vec2 = { 1 , 0 }
  // e1 = 4, e2 = 3
  // m {0, 1}^T = {0, 4} and m {1, 0}^T = {3, 0}
  // m = [ 3 0 ; 0 4 ]
  double m[] = {3.0, 0.0, 0.0, 4.0};

  double e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);

  BOOST_CHECK_CLOSE(e1, 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e2, 3.0, 1e-5);

  BOOST_CHECK_CLOSE(vec1[0], 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1], 1.0, 1e-5);

  BOOST_CHECK_CLOSE(vec2[0], 1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1], 0.0, 1e-5);

  BOOST_CHECK_CLOSE(m[0] * vec1[0] + m[1] * vec1[1], e1 * vec1[0], 1e-5);
  BOOST_CHECK_CLOSE(m[2] * vec1[0] + m[3] * vec1[1], e1 * vec1[1], 1e-5);
}

BOOST_AUTO_TEST_CASE(eigen_value_order1_complex) {
  // Test a specific case for which the eigen vector order
  // is "ambiguous". vec1 should always be associated with
  // e1, and vec2 with e2.
  // vec1 = { 0 , 1 }
  // vec2 = { 1 , 0 }
  // e1 = 4 + i, e2 = 3 + i
  // m {0, 1}^T = {0, 4+i} and m {1, 0}^T = {3+i, 0}
  // m = [ 3+i 0 ; 0 4+i ]
  std::complex<double> m[] = {
      std::complex<double>(3.0, 1.0), std::complex<double>(0.0, 0.0),
      std::complex<double>(0.0, 0.0), std::complex<double>(4.0, 1.0)};

  std::complex<double> e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);

  BOOST_CHECK_CLOSE(e1.real(), 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e1.imag(), 1.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.real(), 3.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.imag(), 1.0, 1e-5);

  BOOST_CHECK_CLOSE(vec1[0].real(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1].real(), 1.0, 1e-5);

  BOOST_CHECK_CLOSE(vec2[0].real(), 1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1].real(), 0.0, 1e-5);

  const std::complex<double> lhs1 = m[0] * vec1[0] + m[1] * vec1[1],
                             rhs1 = e1 * vec1[0],
                             lhs2 = m[2] * vec1[0] + m[3] * vec1[1],
                             rhs2 = e1 * vec1[1];
  BOOST_CHECK_LT(std::abs(lhs1 - rhs1), 1e-5);
  BOOST_CHECK_LT(std::abs(lhs2 - rhs2), 1e-5);
}

BOOST_AUTO_TEST_CASE(eigen_value_order2_complex) {
  // vec1 = { 1 , 0 }
  // vec2 = { 0 , 1 }
  // e1 = 4 + i, e2 = 3 + i
  // m {1, 0}^T = {4+i, 0} and m {0, 1}^T = {0, 3+i}
  // m = [ 4+i 0 ; 0 3+i ]
  std::complex<double> m[] = {
      std::complex<double>(4.0, 1.0), std::complex<double>(0.0, 0.0),
      std::complex<double>(0.0, 0.0), std::complex<double>(3.0, 1.0)};

  std::complex<double> e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(m, e1, e2, vec1, vec2);

  BOOST_CHECK_CLOSE(e1.real(), 4.0, 1e-5);
  BOOST_CHECK_CLOSE(e1.imag(), 1.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.real(), 3.0, 1e-5);
  BOOST_CHECK_CLOSE(e2.imag(), 1.0, 1e-5);

  BOOST_CHECK_CLOSE(vec1[0].real(), 1.0, 1e-5);
  BOOST_CHECK_CLOSE(vec1[1].real(), 0.0, 1e-5);

  BOOST_CHECK_CLOSE(vec2[0].real(), 0.0, 1e-5);
  BOOST_CHECK_CLOSE(vec2[1].real(), 1.0, 1e-5);

  const std::complex<double> lhs1 = m[0] * vec1[0] + m[1] * vec1[1],
                             rhs1 = e1 * vec1[0],
                             lhs2 = m[2] * vec1[0] + m[3] * vec1[1],
                             rhs2 = e1 * vec1[1];
  BOOST_CHECK_LT(std::abs(lhs1 - rhs1), 1e-5);
  BOOST_CHECK_LT(std::abs(lhs2 - rhs2), 1e-5);
}

BOOST_AUTO_TEST_CASE(evdecomposition) {
  MC2x2 a(1, 2, 3, 4), b(5, 6, 7, 8);
  MC2x2 jones = a.MultiplyHerm(b) + b.MultiplyHerm(a);
  MC2x2 r = jones;
  r *= r.HermTranspose();
  std::complex<double> e1, e2, vec1[2], vec2[2];
  Matrix2x2::EigenValuesAndVectors(r.Data(), e1, e2, vec1, vec2);
  double v1norm = std::norm(vec1[0]) + std::norm(vec1[1]);
  vec1[0] /= sqrt(v1norm);
  vec1[1] /= sqrt(v1norm);
  double v2norm = std::norm(vec2[0]) + std::norm(vec2[1]);
  vec2[0] /= sqrt(v2norm);
  vec2[1] /= sqrt(v2norm);

  MC2x2 u(vec1[0], vec2[0], vec1[1], vec2[1]), e(e1, 0, 0, e2);
  MC2x2 res = u.Multiply(e).MultiplyHerm(u);
  for (size_t i = 0; i != 4; ++i)
    BOOST_CHECK_CLOSE(res[i].real(), r[i].real(), 1e-6);

  MC2x2 decomposed = r.DecomposeHermitianEigenvalue();
  decomposed *= decomposed.HermTranspose();
  for (size_t i = 0; i != 4; ++i)
    BOOST_CHECK_CLOSE(decomposed[i].real(), r[i].real(), 1e-6);
}

BOOST_AUTO_TEST_CASE(herm_transpose) {
  const std::complex<double> a(1, 2);
  const std::complex<double> b(3, 4);
  const std::complex<double> c(5, 6);
  const std::complex<double> d(7, 8);
  const MC2x2 m(a, b, c, d);
  MC2x2 result = m.HermTranspose();
  BOOST_CHECK_CLOSE(result[0].real(), a.real(), 1e-6);
  BOOST_CHECK_CLOSE(result[0].imag(), -a.imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].real(), c.real(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].imag(), -c.imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].real(), b.real(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].imag(), -b.imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].real(), d.real(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].imag(), -d.imag(), 1e-6);
  result -= HermTranspose(m);
  for (size_t i = 0; i != 4; ++i) BOOST_CHECK_LT(std::norm(result[i]), 1e-6);
}

BOOST_AUTO_TEST_CASE(conjugate, *boost::unit_test::tolerance(1e8)) {
  const std::complex<double> a(1, 2);
  const std::complex<double> b(3, 4);
  const std::complex<double> c(5, 6);
  const std::complex<double> d(7, 8);

  const MC2x2 m(a, b, c, d);
  const MC2x2 m_conj = m.Conjugate();
  BOOST_TEST(m_conj[0] == std::conj(a));
  BOOST_TEST(m_conj[1] == std::conj(b));
  BOOST_TEST(m_conj[2] == std::conj(c));
  BOOST_TEST(m_conj[3] == std::conj(d));
}

BOOST_AUTO_TEST_CASE(double_dot) {
  const std::complex<double> a(1, 2);
  const std::complex<double> b(3, 4);
  const std::complex<double> c(5, 6);
  const std::complex<double> d(7, 8);
  const MC2x2 m(a, b, c, d);

  // Double contraction with conjugate of itself should equal the matrix norm
  const std::complex<double> result0 = m.DoubleDot(m.Conjugate());
  BOOST_CHECK_CLOSE(result0.real(), aocommon::Norm(m), 1e-8);
  BOOST_CHECK_CLOSE(result0.imag(), 0.0, 1e-8);

  const std::complex<double> result1 = m.DoubleDot(m);
  const std::complex<double> result_ref = a * a + b * b + c * c + d * d;
  BOOST_CHECK_CLOSE(result1.real(), result_ref.real(), 1e-8);
  BOOST_CHECK_CLOSE(result1.imag(), result_ref.imag(), 1e-8);
}

BOOST_AUTO_TEST_CASE(trace) {
  const std::complex<double> a(1, 2);
  const std::complex<double> b(3, 4);
  const std::complex<double> c(5, 6);
  const std::complex<double> d(7, 8);
  const MC2x2 m(a, b, c, d);
  BOOST_CHECK_CLOSE(Trace(m).real(), (a + d).real(), 1e-6);
  BOOST_CHECK_CLOSE(Trace(m).imag(), (a + d).imag(), 1e-6);
  BOOST_CHECK_CLOSE((Trace(m) * 0.0).real(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE((Trace(m) * 0.0).imag(), 0.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(norm) {
  const std::complex<double> a(1, 2);
  const std::complex<double> b(3, 4);
  const std::complex<double> c(5, 6);
  const std::complex<double> d(7, 8);
  const MC2x2 m(a, b, c, d);
  double norm_result =
      1 * 1 + 2 * 2 + 3 * 3 + 4 * 4 + 5 * 5 + 6 * 6 + 7 * 7 + 8 * 8;
  BOOST_CHECK_CLOSE(Norm(m), norm_result, 1e-6);
  BOOST_CHECK_CLOSE(Norm(m * std::complex<double>(0.0, 0.0)), 0.0, 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
