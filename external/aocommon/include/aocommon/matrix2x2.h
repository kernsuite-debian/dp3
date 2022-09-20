#ifndef MATRIX_2X2_H
#define MATRIX_2X2_H

#include <cmath>
#include <complex>
#include <limits>
#include <sstream>

namespace aocommon {

template <typename T>
class MC2x2DiagBase;

/**
 * Class implements a vector with length 4 of complex-valued doubles.
 *
 */
class Vector4 {
 public:
  /**
   * Default constructor
   *
   */
  Vector4(){};

  /**
   * Construct a Vector4 object from four complex-valued numbers.
   *
   */
  Vector4(std::complex<double> a, std::complex<double> b,
          std::complex<double> c, std::complex<double> d) {
    _data[0] = a;
    _data[1] = b;
    _data[2] = c;
    _data[3] = d;
  };

  /**
   * Index Vector4 object
   */
  std::complex<double>& operator[](size_t i) { return _data[i]; }
  const std::complex<double>& operator[](size_t i) const { return _data[i]; }

  /**
   * Get pointer to underlying data
   */
  std::complex<double>* data() { return _data; }
  const std::complex<double>* data() const { return _data; }

 private:
  std::complex<double> _data[4];
};

/**
 * Class wraps functionality around a size 4 pointer
 * as if it were a 2x2 matrix.
 *
 */
class Matrix2x2 {
 public:
  /**
   * Copy complex-valued source buffer to complex-valued dest buffer.
   *
   * TODO: seems redundant?
   */
  template <typename LHS_T, typename RHS_T>
  static void Assign(std::complex<LHS_T>* dest,
                     const std::complex<RHS_T>* source) {
    for (size_t p = 0; p != 4; ++p) dest[p] = source[p];
  }

  /**
   * Copy source buffer to dest buffer.
   *
   */
  template <typename LHS_T, typename RHS_T>
  static void Assign(LHS_T* dest, const RHS_T* source) {
    for (size_t p = 0; p != 4; ++p) dest[p] = source[p];
  }

  /**
   * Add assign rhs buffer to complex-valued dest buffer.
   */
  template <typename T, typename RHS_T>
  static void Add(std::complex<T>* dest, const RHS_T* rhs) {
    for (size_t p = 0; p != 4; ++p) dest[p] += rhs[p];
  }

  /**
   * Subtract assign complex-valued rhs buffer to complex-valued dest buffer.
   * Assumes that T and RHS_T admit an implicit conversion.
   *
   */
  template <typename T>
  static void Subtract(std::complex<T>* dest, const std::complex<T>* rhs) {
    for (size_t p = 0; p != 4; ++p) dest[p] -= rhs[p];
  }

  /**
   * Check if all entries in matrix are finite
   */
  template <typename T>
  static bool IsFinite(const std::complex<T>* matrix) {
    return std::isfinite(matrix[0].real()) && std::isfinite(matrix[0].imag()) &&
           std::isfinite(matrix[1].real()) && std::isfinite(matrix[1].imag()) &&
           std::isfinite(matrix[2].real()) && std::isfinite(matrix[2].imag()) &&
           std::isfinite(matrix[3].real()) && std::isfinite(matrix[3].imag());
  }

  /**
   * Scalar multiplication of matrix.
   */
  template <typename LHS_T, typename RHS_T>
  static void ScalarMultiply(LHS_T* dest, RHS_T factor) {
    for (size_t p = 0; p != 4; ++p) dest[p] *= factor;
  }

  /**
   * Multiply rhs matrix with factor, then add assign to lhs matrix
   */
  template <typename T, typename RHS, typename FactorType>
  static void MultiplyAdd(std::complex<T>* dest, const RHS* rhs,
                          FactorType factor) {
    for (size_t p = 0; p != 4; ++p) dest[p] += rhs[p] * factor;
  }

  /**
   * Matrix multiplication
   */
  template <typename ComplType, typename LHS_T, typename RHS_T>
  static void ATimesB(std::complex<ComplType>* dest, const LHS_T* lhs,
                      const RHS_T* rhs) {
    dest[0] = lhs[0] * rhs[0] + lhs[1] * rhs[2];
    dest[1] = lhs[0] * rhs[1] + lhs[1] * rhs[3];
    dest[2] = lhs[2] * rhs[0] + lhs[3] * rhs[2];
    dest[3] = lhs[2] * rhs[1] + lhs[3] * rhs[3];
  }

  /**
   * Add assign matrix multiplication to destination buffer
   *
   * TODO: use templated type?
   */
  static void PlusATimesB(std::complex<double>* dest,
                          const std::complex<double>* lhs,
                          const std::complex<double>* rhs) {
    dest[0] += lhs[0] * rhs[0] + lhs[1] * rhs[2];
    dest[1] += lhs[0] * rhs[1] + lhs[1] * rhs[3];
    dest[2] += lhs[2] * rhs[0] + lhs[3] * rhs[2];
    dest[3] += lhs[2] * rhs[1] + lhs[3] * rhs[3];
  }

  /**
   * Matrix multiplication of matrix a with the Hermitian transpose of matrix B,
   * , i.e. A * B^H
   *
   * TODO: seems unnecessary to use three templated types
   */
  template <typename ComplType, typename LHS_T, typename RHS_T>
  static void ATimesHermB(std::complex<ComplType>* dest, const LHS_T* lhs,
                          const RHS_T* rhs) {
    dest[0] = lhs[0] * std::conj(rhs[0]) + lhs[1] * std::conj(rhs[1]);
    dest[1] = lhs[0] * std::conj(rhs[2]) + lhs[1] * std::conj(rhs[3]);
    dest[2] = lhs[2] * std::conj(rhs[0]) + lhs[3] * std::conj(rhs[1]);
    dest[3] = lhs[2] * std::conj(rhs[2]) + lhs[3] * std::conj(rhs[3]);
  }

  /**
   * Add assign matrix multiplication of matrix a with the
   * Hermitian transpose of matrix B, i.e. A * B^H
   *
   * TODO: seems unnecessary to use three templated types
   */
  template <typename ComplType, typename LHS_T, typename RHS_T>
  static void PlusATimesHermB(std::complex<ComplType>* dest, const LHS_T* lhs,
                              const RHS_T* rhs) {
    dest[0] += lhs[0] * std::conj(rhs[0]) + lhs[1] * std::conj(rhs[1]);
    dest[1] += lhs[0] * std::conj(rhs[2]) + lhs[1] * std::conj(rhs[3]);
    dest[2] += lhs[2] * std::conj(rhs[0]) + lhs[3] * std::conj(rhs[1]);
    dest[3] += lhs[2] * std::conj(rhs[2]) + lhs[3] * std::conj(rhs[3]);
  }

  /**
   * Matrix multiplication of the Hermitian transpose of matrix A with matrix B,
   * i.e. A^H * B
   */
  template <typename ComplType, typename LHS_T, typename RHS_T>
  static void HermATimesB(std::complex<ComplType>* dest, const LHS_T* lhs,
                          const RHS_T* rhs) {
    dest[0] = std::conj(lhs[0]) * rhs[0] + std::conj(lhs[2]) * rhs[2];
    dest[1] = std::conj(lhs[0]) * rhs[1] + std::conj(lhs[2]) * rhs[3];
    dest[2] = std::conj(lhs[1]) * rhs[0] + std::conj(lhs[3]) * rhs[2];
    dest[3] = std::conj(lhs[1]) * rhs[1] + std::conj(lhs[3]) * rhs[3];
  }

  /**
   * Matrix multiplication of the Hermitian transpose of matrix A with Hermitian
   * transpose of B, i.e. A^H * B^H
   */
  static void HermATimesHermB(std::complex<double>* dest,
                              const std::complex<double>* lhs,
                              const std::complex<double>* rhs) {
    dest[0] = std::conj(lhs[0]) * std::conj(rhs[0]) +
              std::conj(lhs[2]) * std::conj(rhs[1]);
    dest[1] = std::conj(lhs[0]) * std::conj(rhs[2]) +
              std::conj(lhs[2]) * std::conj(rhs[3]);
    dest[2] = std::conj(lhs[1]) * std::conj(rhs[0]) +
              std::conj(lhs[3]) * std::conj(rhs[1]);
    dest[3] = std::conj(lhs[1]) * std::conj(rhs[2]) +
              std::conj(lhs[3]) * std::conj(rhs[3]);
  }

  /**
   * Add assign matrix multiplication A^H * B to the destination buffer.
   *
   * TODO: seems redundant to template three types
   */
  template <typename ComplType, typename LHS_T, typename RHS_T>
  static void PlusHermATimesB(std::complex<ComplType>* dest, const LHS_T* lhs,
                              const RHS_T* rhs) {
    dest[0] += std::conj(lhs[0]) * rhs[0] + std::conj(lhs[2]) * rhs[2];
    dest[1] += std::conj(lhs[0]) * rhs[1] + std::conj(lhs[2]) * rhs[3];
    dest[2] += std::conj(lhs[1]) * rhs[0] + std::conj(lhs[3]) * rhs[2];
    dest[3] += std::conj(lhs[1]) * rhs[1] + std::conj(lhs[3]) * rhs[3];
  }

  /**
   * Compute matrix inverse
   */
  template <typename T>
  static bool Invert(T* matrix) {
    T d = ((matrix[0] * matrix[3]) - (matrix[1] * matrix[2]));
    if (d == T(0.0)) return false;
    T determinant_reciprocal = T(1.0) / d;
    T temp;
    temp = matrix[3] * determinant_reciprocal;
    matrix[1] = -matrix[1] * determinant_reciprocal;
    matrix[2] = -matrix[2] * determinant_reciprocal;
    matrix[3] = matrix[0] * determinant_reciprocal;
    matrix[0] = temp;
    return true;
  }

  /**
   * Compute conjugate transpose (a.k.a. Hermitian transpose) of matrix
   */
  template <typename T>
  static void ConjugateTranspose(T* matrix) {
    matrix[0] = std::conj(matrix[0]);
    T temp = matrix[1];
    matrix[1] = std::conj(matrix[2]);
    matrix[2] = std::conj(temp);
    matrix[3] = std::conj(matrix[3]);
  }

  /**
   * Multiply lhs buffer with inverse of rhs buffer. Returns false if
   * rhs not invertible.
   */
  static bool MultiplyWithInverse(std::complex<double>* lhs,
                                  const std::complex<double>* rhs) {
    std::complex<double> d = ((rhs[0] * rhs[3]) - (rhs[1] * rhs[2]));
    if (d == 0.0) return false;
    std::complex<double> determinant_reciprocal = 1.0 / d;
    std::complex<double> temp[4];
    temp[0] = rhs[3] * determinant_reciprocal;
    temp[1] = -rhs[1] * determinant_reciprocal;
    temp[2] = -rhs[2] * determinant_reciprocal;
    temp[3] = rhs[0] * determinant_reciprocal;

    std::complex<double> temp2 = lhs[0];
    lhs[0] = lhs[0] * temp[0] + lhs[1] * temp[2];
    lhs[1] = temp2 * temp[1] + lhs[1] * temp[3];

    temp2 = lhs[2];
    lhs[2] = lhs[2] * temp[0] + lhs[3] * temp[2];
    lhs[3] = temp2 * temp[1] + lhs[3] * temp[3];
    return true;
  }

  /**
   * Compute singular values of the matrix buffer
   */
  static void SingularValues(const std::complex<double>* matrix, double& e1,
                             double& e2) {
    // This is not the ultimate fastest method, since we
    // don't need to calculate the imaginary values of b,c at all.
    // Calculate M M^H
    std::complex<double> temp[4] = {
        matrix[0] * std::conj(matrix[0]) + matrix[1] * std::conj(matrix[1]),
        matrix[0] * std::conj(matrix[2]) + matrix[1] * std::conj(matrix[3]),
        matrix[2] * std::conj(matrix[0]) + matrix[3] * std::conj(matrix[1]),
        matrix[2] * std::conj(matrix[2]) + matrix[3] * std::conj(matrix[3])};
    // Use quadratic formula, with a=1.
    double b = -temp[0].real() - temp[3].real(),
           c = temp[0].real() * temp[3].real() - (temp[1] * temp[2]).real(),
           d = b * b - (4.0 * 1.0) * c, sqrtd = std::sqrt(d);

    e1 = std::sqrt((-b + sqrtd) * 0.5);
    e2 = std::sqrt((-b - sqrtd) * 0.5);
  }

  /**
   * Compute eigen values of input matrix buffer. It assumes that the
   * determinant > 0, so that eigenvalues are real.
   */
  static void EigenValues(const double* matrix, double& e1, double& e2) {
    double tr = matrix[0] + matrix[3];
    double d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    double term = std::sqrt(tr * tr * 0.25 - d);
    double trHalf = tr * 0.5;
    e1 = trHalf + term;
    e2 = trHalf - term;
  }

  /**
   * Compute the eigen values of a complex matrix.
   *
   * TODO: can probably be merged with previous method.
   */
  template <typename ValType>
  static void EigenValues(const std::complex<ValType>* matrix,
                          std::complex<ValType>& e1,
                          std::complex<ValType>& e2) {
    std::complex<ValType> tr = matrix[0] + matrix[3];
    std::complex<ValType> d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    std::complex<ValType> term = std::sqrt(tr * tr * ValType(0.25) - d);
    std::complex<ValType> trHalf = tr * ValType(0.5);
    e1 = trHalf + term;
    e2 = trHalf - term;
  }

  /**
   * Compute eigen values and vectors for real matrix. Assumes
   * the determinant > 0.
   */
  static void EigenValuesAndVectors(const double* matrix, double& e1,
                                    double& e2, double* vec1, double* vec2) {
    double tr = matrix[0] + matrix[3];
    double d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    double term = std::sqrt(tr * tr * 0.25 - d);
    double trHalf = tr * 0.5;
    e1 = trHalf + term;
    e2 = trHalf - term;
    double limit = std::min(std::fabs(e1), std::fabs(e2)) * 1e-6;
    if (std::fabs(matrix[2]) > limit) {
      vec1[0] = matrix[3] - e1;
      vec1[1] = -matrix[2];
      vec2[0] = matrix[3] - e2;
      vec2[1] = -matrix[2];
    } else if (std::fabs(matrix[1]) > limit) {
      vec1[0] = -matrix[1];
      vec1[1] = matrix[0] - e1;
      vec2[0] = -matrix[1];
      vec2[1] = matrix[0] - e2;
    } else {
      // We know that A v = lambda v, and we know that v1 or v2 = [1, 0]:
      double
          // Evaluate for v = [1, 0] and see if the error is smaller for e1 than
          // for e2
          err1_0 = matrix[0] - e1,
          err2_0 = matrix[0] - e2;
      if (err1_0 * err1_0 < err2_0 * err2_0) {
        vec1[0] = 1.0;
        vec1[1] = 0.0;
        vec2[0] = 0.0;
        vec2[1] = 1.0;
      } else {
        vec1[0] = 0.0;
        vec1[1] = 1.0;
        vec2[0] = 1.0;
        vec2[1] = 0.0;
      }
    }
  }

  /**
   * Compute eigen values and vectors for complex-valued matrix. Assumes
   * the determinant > 0.
   *
   * TODO: can probably be merged with previous method
   */
  static void EigenValuesAndVectors(const std::complex<double>* matrix,
                                    std::complex<double>& e1,
                                    std::complex<double>& e2,
                                    std::complex<double>* vec1,
                                    std::complex<double>* vec2) {
    std::complex<double> tr = matrix[0] + matrix[3];
    std::complex<double> d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    std::complex<double> term = std::sqrt(tr * tr * 0.25 - d);
    std::complex<double> trHalf = tr * 0.5;
    e1 = trHalf + term;
    e2 = trHalf - term;
    double limit = std::min(std::abs(e1), std::abs(e2)) * 1e-6;
    if (std::abs(matrix[2]) > limit) {
      vec1[0] = matrix[3] - e1;
      vec1[1] = -matrix[2];
      vec2[0] = matrix[3] - e2;
      vec2[1] = -matrix[2];
    } else if (std::abs(matrix[1]) > limit) {
      vec1[0] = -matrix[1];
      vec1[1] = matrix[0] - e1;
      vec2[0] = -matrix[1];
      vec2[1] = matrix[0] - e2;
    } else {
      // We know that A v = lambda v, and we know that v1 or v2 = [1, 0]:
      auto
          // Evaluate for v = [1, 0] and see if the error is smaller for e1 than
          // for e2
          err1_0 = std::norm(matrix[0] - e1),
          err2_0 = std::norm(matrix[0] - e2);
      if (err1_0 < err2_0) {
        vec1[0] = 1.0;
        vec1[1] = 0.0;
        vec2[0] = 0.0;
        vec2[1] = 1.0;
      } else {
        vec1[0] = 0.0;
        vec1[1] = 1.0;
        vec2[0] = 1.0;
        vec2[1] = 0.0;
      }
    }
  }

  /**
   * Computes the positive square root of a real-valued matrix buffer such that
   * M = R * R. Assumes that determinant > 0. Note that matrix M might have more
   * square roots.
   */
  static void SquareRoot(double* matrix) {
    double tr = matrix[0] + matrix[3];
    double d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    double s = /*+/-*/ std::sqrt(d);
    double t = /*+/-*/ std::sqrt(tr + 2.0 * s);
    if (t != 0.0) {
      matrix[0] = (matrix[0] + s) / t;
      matrix[1] = (matrix[1] / t);
      matrix[2] = (matrix[2] / t);
      matrix[3] = (matrix[3] + s) / t;
    } else {
      if (matrix[0] == 0.0 && matrix[1] == 0.0 && matrix[2] == 0.0 &&
          matrix[3] == 0.0) {
        // done: it's the zero matrix
      } else {
        for (size_t i = 0; i != 4; ++i)
          matrix[i] = std::numeric_limits<double>::quiet_NaN();
      }
    }
  }

  /**
   * Computes the positive square root of a complex-valued matrix buffer,
   * such that M = R * R. Assumes that determinant > 0.
   * Note that matrix M might have more square roots.
   */
  static void SquareRoot(std::complex<double>* matrix) {
    std::complex<double> tr = matrix[0] + matrix[3];
    std::complex<double> d = matrix[0] * matrix[3] - matrix[1] * matrix[2];
    std::complex<double> s = /*+/-*/ std::sqrt(d);
    std::complex<double> t = /*+/-*/ std::sqrt(tr + 2.0 * s);
    if (t != 0.0) {
      matrix[0] = (matrix[0] + s) / t;
      matrix[1] = (matrix[1] / t);
      matrix[2] = (matrix[2] / t);
      matrix[3] = (matrix[3] + s) / t;
    } else {
      if (matrix[0] == 0.0 && matrix[1] == 0.0 && matrix[2] == 0.0 &&
          matrix[3] == 0.0) {
        // done: it's the zero matrix
      } else {
        for (size_t i = 0; i != 4; ++i)
          matrix[i] =
              std::complex<double>(std::numeric_limits<double>::quiet_NaN(),
                                   std::numeric_limits<double>::quiet_NaN());
      }
    }
  }

  /**
   * Calculates L, the lower triangle of the Cholesky decomposition, such that
   * L L^H = M. The result is undefined when the matrix is not positive
   * definite.
   */
  static void UncheckedCholesky(std::complex<double>* matrix) {
    // solve:
    // ( a 0 ) ( a* b* ) = ( aa* ;    ab*    )
    // ( b c ) ( 0  c* )   ( a*b ; bb* + cc* )
    // With a and c necessarily real.
    double a = std::sqrt(matrix[0].real());
    std::complex<double> b = std::conj(matrix[1] / a);
    double bbConj = b.real() * b.real() + b.imag() * b.imag();
    double c = std::sqrt(matrix[3].real() - bbConj);
    matrix[0] = a;
    matrix[1] = 0.0;
    matrix[2] = b;
    matrix[3] = c;
  }

  /**
   * Calculates L, the lower triangle of the Cholesky decomposition, such that
   * L L^H = M. Return false when the result would not be finite.
   */
  static bool Cholesky(std::complex<double>* matrix) {
    if (matrix[0].real() < 0.0) return false;
    double a = std::sqrt(matrix[0].real());
    std::complex<double> b = std::conj(matrix[1] / a);
    double bbConj = b.real() * b.real() + b.imag() * b.imag();
    double cc = matrix[3].real() - bbConj;
    if (cc < 0.0) return false;
    double c = std::sqrt(cc);
    matrix[0] = a;
    matrix[1] = 0.0;
    matrix[2] = b;
    matrix[3] = c;
    return true;
  }

  /**
   * Calculates L, the lower triangle of the Cholesky decomposition, such that
   * L L^H = M. Return false when the matrix was not positive semi-definite.
   */
  static bool CheckedCholesky(std::complex<double>* matrix) {
    if (matrix[0].real() <= 0.0 || matrix[0].imag() != 0.0 ||
        matrix[3].real() <= 0.0 || matrix[3].imag() != 0.0 ||
        matrix[1] != std::conj(matrix[2]))
      return false;
    UncheckedCholesky(matrix);
    return true;
  }

  /**
   * Calculates the rotation angle of a complex-valued matrix.
   */
  template <typename T>
  static T RotationAngle(const std::complex<T>* matrix) {
    return std::atan2((matrix[2].real() - matrix[1].real()) * 0.5,
                      (matrix[0].real() + matrix[3].real()) * 0.5);
  }

  /**
   * Calculates the rotation matrix, given a rotation angle \p alpha.
   */
  template <typename T>
  static void RotationMatrix(std::complex<T>* matrix, double alpha) {
    T cos_alpha = std::cos(alpha), sin_alpha = std::sin(alpha);
    matrix[0] = cos_alpha;
    matrix[1] = -sin_alpha;
    matrix[2] = sin_alpha;
    matrix[3] = cos_alpha;
  }
};

/**
 * Class implements a 2x2 complex-valued matrix.
 */
template <typename ValType>
class MC2x2Base {
 public:
  MC2x2Base() {}

  template <typename OtherValType>
  MC2x2Base(const MC2x2Base<OtherValType>& source) {
    Matrix2x2::Assign(_values, source.Data());
  }

  /**
   * Construct MC2x2Base object from (length 4) data buffer
   */
  template <typename T>
  explicit MC2x2Base(const T source[4]) {
    Matrix2x2::Assign(_values, source);
  }

  /**
   * Construct MC2x2Base object from four input values. Internally, values are
   * converted to complex type.
   */
  MC2x2Base(ValType m00, ValType m01, ValType m10, ValType m11) {
    _values[0] = m00;
    _values[1] = m01;
    _values[2] = m10;
    _values[3] = m11;
  }

  /**
   * Construct MC2x2Base object from four complex-valued input values.
   */
  MC2x2Base(std::complex<ValType> m00, std::complex<ValType> m01,
            std::complex<ValType> m10, std::complex<ValType> m11) {
    _values[0] = m00;
    _values[1] = m01;
    _values[2] = m10;
    _values[3] = m11;
  }

  /**
   * Construct from a diagonal matrix
   */
  MC2x2Base(const MC2x2DiagBase<ValType>& diag);

  /**
   * Copy assignment operator
   */
  MC2x2Base<ValType>& operator=(const MC2x2Base<ValType>& source) {
    Matrix2x2::Assign(_values, source._values);
    return *this;
  }

  /**
   * Addition operator
   */
  MC2x2Base<ValType> operator+(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base<ValType> result(*this);
    Matrix2x2::Add(result._values, rhs._values);
    return result;
  }

  /**
   * Addition assignment operator
   */
  MC2x2Base<ValType>& operator+=(const MC2x2Base<ValType>& rhs) {
    Matrix2x2::Add(_values, rhs._values);
    return *this;
  }

  /**
   * Subtraction assignment operator
   */
  MC2x2Base<ValType>& operator-=(const MC2x2Base<ValType>& rhs) {
    Matrix2x2::Subtract(_values, rhs._values);
    return *this;
  }

  /**
   * Matrix multiplication assignment operator
   */
  MC2x2Base<ValType>& operator*=(const MC2x2Base<ValType>& rhs) {
    MC2x2Base<ValType> lhs(*this);
    Matrix2x2::ATimesB(_values, lhs._values, rhs._values);
    return *this;
  }

  /**
   * Matrix multiplication operator
   */
  MC2x2Base<ValType> operator*(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base<ValType> dest;
    Matrix2x2::ATimesB(dest._values, _values, rhs._values);
    return dest;
  }

  /**
   * Matrix multiplication assignment operator given a length 4 rhs buffer
   * of possibly different type
   */
  template <typename T>
  MC2x2Base<ValType>& operator*=(const T* rhs) {
    MC2x2Base<ValType> lhs(*this);
    Matrix2x2::ATimesB(_values, lhs._values, rhs);
    return *this;
  }

  /**
   * Matrix multiplication given a length 4 rhs buffer of possibly different
   * type
   */
  template <typename T>
  MC2x2Base<ValType> operator*(const T* rhs) const {
    MC2x2Base<ValType> dest;
    Matrix2x2::ATimesB(dest._values, _values, rhs);
    return dest;
  }

  /**
   * Scalar multiplication assignment operator
   */
  MC2x2Base<ValType>& operator*=(ValType rhs) {
    Matrix2x2::ScalarMultiply(_values, rhs);
    return *this;
  }

  /**
   * Scalar multiplication operator
   */
  MC2x2Base<ValType> operator*(ValType rhs) const {
    MC2x2Base<ValType> dest(*this);
    Matrix2x2::ScalarMultiply(dest._values, rhs);
    return dest;
  }

  /**
   * Complex scalar multiplication
   */
  MC2x2Base<ValType> operator*(std::complex<ValType> rhs) const {
    MC2x2Base<ValType> dest(*this);
    Matrix2x2::ScalarMultiply(dest._values, rhs);
    return dest;
  }

  /**
   * Scalar division assignment operator
   */
  MC2x2Base<ValType>& operator/=(ValType rhs) {
    Matrix2x2::ScalarMultiply(_values, ValType(1.0) / rhs);
    return *this;
  }

  const std::complex<ValType>& operator[](size_t index) const {
    return _values[index];
  }
  std::complex<ValType>& operator[](size_t index) { return _values[index]; }

  /**
   * Get real value at given index
   */
  const ValType& IndexReal(size_t index) const {
    return reinterpret_cast<const ValType(&)[2]>(_values[index / 2])[index % 2];
  }
  ValType& IndexReal(size_t index) {
    return reinterpret_cast<ValType(&)[2]>(_values[index / 2])[index % 2];
  }

  /**
   * Return MC2x2Base matrix filled with zeros
   */
  static MC2x2Base<ValType> Zero() {
    return MC2x2Base<ValType>(0.0, 0.0, 0.0, 0.0);
  }

  /**
   * Return 2x2 identity matrix
   */
  static MC2x2Base<ValType> Unity() { return MC2x2Base(1.0, 0.0, 0.0, 1.0); }

  /**
   * Return 2x2 matrix filled with NaN values
   */
  static MC2x2Base<ValType> NaN() {
    return MC2x2Base<ValType>(
        std::complex<ValType>(std::numeric_limits<ValType>::quiet_NaN(),
                              std::numeric_limits<ValType>::quiet_NaN()),
        std::complex<ValType>(std::numeric_limits<ValType>::quiet_NaN(),
                              std::numeric_limits<ValType>::quiet_NaN()),
        std::complex<ValType>(std::numeric_limits<ValType>::quiet_NaN(),
                              std::numeric_limits<ValType>::quiet_NaN()),
        std::complex<ValType>(std::numeric_limits<ValType>::quiet_NaN(),
                              std::numeric_limits<ValType>::quiet_NaN()));
  }

  /**
   * Get pointer to underlying data
   */
  std::complex<ValType>* Data() { return _values; }
  const std::complex<ValType>* Data() const { return _values; }

  /**
   * Assign data stored by 2x2 matrix to destination buffer
   */
  template <typename T>
  void AssignTo(std::complex<T>* destination) const {
    Matrix2x2::Assign(destination, _values);
  }

  /**
   * Flatten 2x2 matrix to length 4 vector
   */
  Vector4 Vec() const {
    return Vector4(_values[0], _values[2], _values[1], _values[3]);
  }

  /**
   * Matrix multiplication, alias for the overloaded * operator
   */
  MC2x2Base<ValType> Multiply(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base<ValType> dest;
    Matrix2x2::ATimesB(dest._values, _values, rhs._values);
    return dest;
  }

  /**
   * Matrix multiplication of internal matrix with Hermitian transpose of input
   * matrix, i.e. A * B^H
   */
  MC2x2Base<ValType> MultiplyHerm(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base dest;
    Matrix2x2::ATimesHermB(dest._values, _values, rhs._values);
    return dest;
  }

  /**
   * Matrix multiplication Hermitian transpose of internal matrix with input
   * matrix, i.e. A^H * B
   */
  MC2x2Base<ValType> HermThenMultiply(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base<ValType> dest;
    Matrix2x2::HermATimesB(dest._values, _values, rhs._values);
    return dest;
  }

  /**
   * Matrix multiplication of Hermitian transposes, i.e. A^H * B^H
   */
  MC2x2Base<ValType> HermThenMultiplyHerm(const MC2x2Base<ValType>& rhs) const {
    MC2x2Base<ValType> dest;
    Matrix2x2::HermATimesHermB(dest._values, _values, rhs._values);
    return dest;
  }

  /**
   * Computes the double dot, i.e. A:B (A_ij Bij)
   * See https://en.wikipedia.org/wiki/Dyadics#Double-dot_product
   */
  std::complex<ValType> DoubleDot(const MC2x2Base<ValType>& rhs) const {
    return _values[0] * rhs[0] + _values[1] * rhs[1] + _values[2] * rhs[2] +
           _values[3] * rhs[3];
  }

  /**
   * Multiply input matrix with factor, then add assign to stored matrix
   */
  template <typename FactorType>
  void AddWithFactorAndAssign(const MC2x2Base<ValType>& rhs,
                              FactorType factor) {
    Matrix2x2::MultiplyAdd(_values, rhs._values, factor);
  }

  /**
   * Compute (regular) transpose of matrix
   */
  MC2x2Base<ValType> Transpose() const {
    return MC2x2Base(_values[0], _values[2], _values[1], _values[3]);
  }

  /**
   * Compute Hermitian transpose of matrix
   */
  MC2x2Base<ValType> HermTranspose() const {
    return MC2x2Base(std::conj(_values[0]), std::conj(_values[2]),
                     std::conj(_values[1]), std::conj(_values[3]));
  }

  /**
   * Compute the elementwise conjugate of the matrix (without transposing!)
   */
  MC2x2Base<ValType> Conjugate() const {
    return MC2x2Base(std::conj(_values[0]), std::conj(_values[1]),
                     std::conj(_values[2]), std::conj(_values[3]));
  }

  /**
   * Invert 2x2 matrix, returns false if matrix is not invertible
   */
  bool Invert() { return Matrix2x2::Invert(_values); }

  /**
   * Matrix multiplication, write result to MC2x2Base object
   */
  static void ATimesB(MC2x2Base<ValType>& dest, const MC2x2Base<ValType>& lhs,
                      const MC2x2Base<ValType>& rhs) {
    Matrix2x2::ATimesB(dest._values, lhs._values, rhs._values);
  }

  /**
   * Matrix multiplication, write result to buffer
   */
  static void ATimesB(std::complex<ValType>* dest,
                      const MC2x2Base<ValType>& lhs,
                      const MC2x2Base<ValType>& rhs) {
    Matrix2x2::ATimesB(dest, lhs._values, rhs._values);
  }

  /**
   * Matrix multiplication of \p lhs with Hermitian transpose of \p rhs
   */
  static void ATimesHermB(MC2x2Base<ValType>& dest,
                          const MC2x2Base<ValType>& lhs,
                          const MC2x2Base<ValType>& rhs) {
    Matrix2x2::ATimesHermB(dest._values, lhs._values, rhs._values);
  }

  /**
   * Matrix multiplication of Hermitian transpose of \p lhs with \p rhs
   */
  static void HermATimesB(MC2x2Base<ValType>& dest,
                          const MC2x2Base<ValType>& lhs,
                          const MC2x2Base<ValType>& rhs) {
    Matrix2x2::HermATimesB(dest._values, lhs._values, rhs._values);
  }

  /**
   * Matrix multiplication of Hermitian transpose of \p lhs with Hermitian
   * transpose \p rhs
   */
  static void HermATimesHermB(MC2x2Base<ValType>& dest,
                              const MC2x2Base<ValType>& lhs,
                              const MC2x2Base<ValType>& rhs) {
    Matrix2x2::HermATimesHermB(dest._values, lhs._values, rhs._values);
  }

  /**
   * Convert matrix to pretty string
   */
  std::string ToString() const {
    std::stringstream str;
    str << _values[0] << ", " << _values[1] << "; " << _values[2] << ", "
        << _values[3];
    return str.str();
  }

  /**
   * Copy values to buffer
   */
  void CopyValues(std::complex<ValType>* values) const {
    Matrix2x2::Assign(values, _values);
  }

  /**
   * Calculate eigen values
   */
  void EigenValues(std::complex<ValType>& e1, std::complex<ValType>& e2) const {
    Matrix2x2::EigenValues(_values, e1, e2);
  }

  /**
   * Check if matrix entries are finite
   */
  bool IsFinite() const {
    return std::isfinite(_values[0].real()) &&
           std::isfinite(_values[0].imag()) &&
           std::isfinite(_values[1].real()) &&
           std::isfinite(_values[1].imag()) &&
           std::isfinite(_values[2].real()) &&
           std::isfinite(_values[2].imag()) &&
           std::isfinite(_values[3].real()) && std::isfinite(_values[3].imag());
  }
  /**
   * Calculates L, the lower triangle of the Cholesky decomposition, such that
   * L L^H = M.
   */
  bool Cholesky() { return Matrix2x2::Cholesky(_values); }

  /**
   * See Matrix2x2::CheckedCholesky
   */
  bool CheckedCholesky() { return Matrix2x2::CheckedCholesky(_values); }

  /**
   * See Matrix2x2::UncheckedCholesky
   */
  void UncheckedCholesky() { Matrix2x2::UncheckedCholesky(_values); }

  /**
   * Decompose a Hermitian matrix X into A A^H such that
   *   X = A A^H = U D D^H U^H
   *   with A = U D
   * where D D^H = E is a diagonal matrix
   *       with the eigen values of X, and U contains the eigen vectors.
   */
  MC2x2Base<ValType> DecomposeHermitianEigenvalue() {
    std::complex<ValType> e1, e2, vec1[2], vec2[2];
    Matrix2x2::EigenValuesAndVectors(_values, e1, e2, vec1, vec2);
    ValType v1norm = std::norm(vec1[0]) + std::norm(vec1[1]);
    vec1[0] /= std::sqrt(v1norm);
    vec1[1] /= std::sqrt(v1norm);
    ValType v2norm = std::norm(vec2[0]) + std::norm(vec2[1]);
    vec2[0] /= std::sqrt(v2norm);
    vec2[1] /= std::sqrt(v2norm);

    return MC2x2Base<ValType>(
        vec1[0] * std::sqrt(e1.real()), vec2[0] * std::sqrt(e2.real()),
        vec1[1] * std::sqrt(e1.real()), vec2[1] * std::sqrt(e2.real()));
  }

 private:
  std::complex<ValType> _values[4];
};

/**
 * Calculate the Hermite transpose of a 2x2 matrix.
 */
template <typename ValType>
MC2x2Base<ValType> HermTranspose(const MC2x2Base<ValType>& matrix) {
  return MC2x2Base<ValType>(std::conj(matrix[0]), std::conj(matrix[2]),
                            std::conj(matrix[1]), std::conj(matrix[3]));
}

/**
 * Calculate the sum of the diagonal elements.
 */
template <typename ValType>
std::complex<ValType> Trace(const MC2x2Base<ValType>& matrix) {
  return matrix[0] + matrix[3];
}

/**
 * Calculate the Frobenius norm of a matrix. This
 * is the sum of squares over all the real and imaginary values
 * in the matrix.
 */
template <typename ValType>
ValType Norm(const MC2x2Base<ValType>& matrix) {
  return std::norm(matrix[0]) + std::norm(matrix[1]) + std::norm(matrix[2]) +
         std::norm(matrix[3]);
}

using MC2x2 = MC2x2Base<double>;
using MC2x2F = MC2x2Base<float>;

}  // namespace aocommon

#include "matrix2x2diag.h"

template <typename ValType>
aocommon::MC2x2Base<ValType>::MC2x2Base(
    const aocommon::MC2x2DiagBase<ValType>& diag)
    : _values{diag[0], 0.0, 0.0, diag[1]} {}

#endif
