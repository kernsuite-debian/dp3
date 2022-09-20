#include <boost/test/unit_test.hpp>

#include <aocommon/matrix2x2diag.h>

#include <iostream>

using aocommon::MC2x2;
using aocommon::MC2x2Diag;

BOOST_AUTO_TEST_SUITE(matrix2x2diag)

BOOST_AUTO_TEST_CASE(construct_default) {
  const MC2x2Diag m;
  BOOST_CHECK_CLOSE(m[0].real(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(m[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(m[1].real(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(m[1].imag(), 0.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(construct_from_array) {
  double unit[2] = {1.0, 1.0};
  const MC2x2Diag m(unit);
  BOOST_CHECK_CLOSE(m[0].real(), 1.0, 1e-6);
  BOOST_CHECK_CLOSE(m[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(m[1].real(), 1.0, 1e-6);
  BOOST_CHECK_CLOSE(m[1].imag(), 0.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(copy_construct) {
  const MC2x2Diag source(std::complex<double>(3.0, 4.0),
                         std::complex<double>(5.0, 6.0));
  const MC2x2Diag copy(source);
  BOOST_CHECK_CLOSE(copy[0].real(), 3.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[0].imag(), 4.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[1].real(), 5.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[1].imag(), 6.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(copy_assign) {
  const MC2x2Diag source(std::complex<double>(3.0, 4.0),
                         std::complex<double>(5.0, 6.0));
  MC2x2Diag copy(MC2x2Diag::Zero());
  copy = source;
  BOOST_CHECK_CLOSE(copy[0].real(), 3.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[0].imag(), 4.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[1].real(), 5.0, 1e-6);
  BOOST_CHECK_CLOSE(copy[1].imag(), 6.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(add) {
  MC2x2Diag lhs(std::complex<double>(3.0, 4.0), std::complex<double>(5.0, 6.0));
  const MC2x2Diag rhs(std::complex<double>(7.0, 8.0),
                      std::complex<double>(9.0, 10.0));
  lhs += rhs;
  lhs += MC2x2Diag::Zero();
  BOOST_CHECK_CLOSE(lhs[0].real(), 10.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[0].imag(), 12.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[1].real(), 14.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[1].imag(), 16.0, 1e-6);
  lhs = lhs + lhs + MC2x2Diag::Zero();
  BOOST_CHECK_CLOSE(lhs[0].real(), 20.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[0].imag(), 24.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[1].real(), 28.0, 1e-6);
  BOOST_CHECK_CLOSE(lhs[1].imag(), 32.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(multiply) {
  const std::complex<double> a(1.0, 2.0);
  const std::complex<double> b(3.0, 4.0);
  const std::complex<double> c(5.0, 6.0);
  const std::complex<double> d(7.0, 8.0);
  const MC2x2Diag lhs(a, b);
  const MC2x2Diag rhs(c, d);
  MC2x2Diag result = lhs * rhs * MC2x2Diag::Unity();
  result *= MC2x2Diag::Unity() * MC2x2Diag(2, 3);
  BOOST_CHECK_CLOSE(result[0].real(), 2.0 * (a * c).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[0].imag(), 2.0 * (a * c).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].real(), 3.0 * (b * d).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].imag(), 3.0 * (b * d).imag(), 1e-6);
}

BOOST_AUTO_TEST_CASE(scalar_operations) {
  MC2x2Diag a(2.0, 4.0);
  a *= 4.0;
  BOOST_CHECK_CLOSE(a[0].real(), 8.0, 1e-6);
  BOOST_CHECK_CLOSE(a[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(a[1].real(), 16.0, 1e-6);
  BOOST_CHECK_CLOSE(a[1].imag(), 0.0, 1e-6);
  a /= 2.0;
  BOOST_CHECK_CLOSE(a[0].real(), 4.0, 1e-6);
  BOOST_CHECK_CLOSE(a[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(a[1].real(), 8.0, 1e-6);
  BOOST_CHECK_CLOSE(a[1].imag(), 0.0, 1e-6);

  MC2x2Diag b = a * 2.0;
  BOOST_CHECK_CLOSE(b[0].real(), 8.0, 1e-6);
  BOOST_CHECK_CLOSE(b[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(b[1].real(), 16.0, 1e-6);
  BOOST_CHECK_CLOSE(b[1].imag(), 0.0, 1e-6);

  MC2x2Diag c = b / 2.0;
  BOOST_CHECK_CLOSE(c[0].real(), 4.0, 1e-6);
  BOOST_CHECK_CLOSE(c[0].imag(), 0.0, 1e-6);
  BOOST_CHECK_CLOSE(c[1].real(), 8.0, 1e-6);
  BOOST_CHECK_CLOSE(c[1].imag(), 0.0, 1e-6);

  MC2x2Diag d(std::complex<double>{1.0, 2.0}, std::complex<double>{3.0, 4.0});
  d *= std::complex<double>{1.0, 2.0};
  BOOST_CHECK_CLOSE(d[0].real(), -3.0, 1e-6);
  BOOST_CHECK_CLOSE(d[0].imag(), 4.0, 1e-6);
  BOOST_CHECK_CLOSE(d[1].real(), -5.0, 1e-6);
  BOOST_CHECK_CLOSE(d[1].imag(), 10.0, 1e-6);

  MC2x2Diag e = d * std::complex<double>{2.0, 1.0};
  BOOST_CHECK_CLOSE(e[0].real(), -10.0, 1e-6);
  BOOST_CHECK_CLOSE(e[0].imag(), 5.0, 1e-6);
  BOOST_CHECK_CLOSE(e[1].real(), -20.0, 1e-6);
  BOOST_CHECK_CLOSE(e[1].imag(), 15.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(herm_transpose) {
  const MC2x2Diag m(std::complex<double>(3.0, 4.0),
                    std::complex<double>(5.0, 6.0));
  const MC2x2Diag result = m.HermTranspose();
  BOOST_CHECK_CLOSE(result[0].real(), 3.0, 1e-6);
  BOOST_CHECK_CLOSE(result[0].imag(), -4.0, 1e-6);
  BOOST_CHECK_CLOSE(result[1].real(), 5.0, 1e-6);
  BOOST_CHECK_CLOSE(result[1].imag(), -6.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(diagonal) {
  const std::complex<double> a(1.0, 2.0);
  const std::complex<double> b(3.0, 4.0);
  const std::complex<double> c(5.0, 6.0);
  const std::complex<double> d(7.0, 8.0);
  const MC2x2 m(a, b, c, d);
  const MC2x2Diag dm = Diagonal(m);
  BOOST_CHECK_CLOSE(dm[0].real(), a.real(), 1e-6);
  BOOST_CHECK_CLOSE(dm[0].imag(), a.imag(), 1e-6);
  BOOST_CHECK_CLOSE(dm[1].real(), d.real(), 1e-6);
  BOOST_CHECK_CLOSE(dm[1].imag(), d.imag(), 1e-6);
}

BOOST_AUTO_TEST_CASE(diag_nondiag_multiply) {
  const std::complex<double> a(1.0, 2.0);
  const std::complex<double> b(3.0, 4.0);
  const std::complex<double> c(5.0, 6.0);
  const std::complex<double> d(7.0, 8.0);
  const std::complex<double> e(8.0, 9.0);
  const std::complex<double> f(10.0, 11.0);
  const MC2x2 m(a, b, c, d);
  const MC2x2Diag dm(e, f);
  const MC2x2 result = m * dm;
  BOOST_CHECK_CLOSE(result[0].real(), (a * e).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[0].imag(), (a * e).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].real(), (b * f).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].imag(), (b * f).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].real(), (c * e).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].imag(), (c * e).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].real(), (d * f).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].imag(), (d * f).imag(), 1e-6);
}

BOOST_AUTO_TEST_CASE(nondiag_diag_multiply) {
  const std::complex<double> a(1.0, 2.0);
  const std::complex<double> b(3.0, 4.0);
  const std::complex<double> c(5.0, 6.0);
  const std::complex<double> d(7.0, 8.0);
  const std::complex<double> e(8.0, 9.0);
  const std::complex<double> f(10.0, 11.0);
  const MC2x2Diag dm(a, b);
  const MC2x2 m(c, d, e, f);
  const MC2x2 result = dm * m;
  BOOST_CHECK_CLOSE(result[0].real(), (a * c).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[0].imag(), (a * c).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].real(), (a * d).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[1].imag(), (a * d).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].real(), (b * e).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[2].imag(), (b * e).imag(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].real(), (b * f).real(), 1e-6);
  BOOST_CHECK_CLOSE(result[3].imag(), (b * f).imag(), 1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
