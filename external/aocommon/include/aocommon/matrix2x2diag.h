#ifndef MATRIX_2X2_DIAG_H
#define MATRIX_2X2_DIAG_H

#include <cmath>
#include <complex>
#include <limits>
#include <sstream>

#include "matrix2x2.h"

namespace aocommon {
/**
 * Class implements a 2x2 complex-valued diagonal matrix.
 */
template <typename ValType>
class MC2x2DiagBase {
 public:
  MC2x2DiagBase() {}
  MC2x2DiagBase(const MC2x2DiagBase<ValType>& source) {
    _values[0] = source._values[0];
    _values[1] = source._values[1];
  }

  /**
   * Construct MC2x2Base object from (length 2) data buffer
   */
  template <typename T>
  explicit MC2x2DiagBase(const T* source) {
    _values[0] = source[0];
    _values[1] = source[1];
  }

  /**
   * Construct object from diagonal input values. Values are
   * converted to complex type.
   */
  MC2x2DiagBase(ValType m00, ValType m11) {
    _values[0] = m00;
    _values[1] = m11;
  }

  /**
   * Construct MC2x2Base object from two complex-valued input values.
   */
  MC2x2DiagBase(std::complex<ValType> m00, std::complex<ValType> m11) {
    _values[0] = m00;
    _values[1] = m11;
  }

  MC2x2DiagBase<ValType>& operator=(const MC2x2DiagBase<ValType>& source) {
    _values[0] = source._values[0];
    _values[1] = source._values[1];
    return *this;
  }

  MC2x2DiagBase<ValType> operator*(const MC2x2DiagBase<ValType>& source) const {
    return Multiply(source);
  }

  MC2x2DiagBase<ValType>& operator*=(const MC2x2DiagBase<ValType>& source) {
    (*this) = (*this) * source;
    return *this;
  }

  /**
   * Scalar multiplication operator, real valued rhs
   */
  MC2x2DiagBase<ValType> operator*(ValType rhs) const {
    MC2x2DiagBase<ValType> dest(*this);
    MC2x2DiagBase::ScalarMultiply(dest._values, rhs);
    return dest;
  }

  /**
   * Scalar multiplication assignment operator, real valued rhs
   */
  MC2x2DiagBase<ValType>& operator*=(ValType rhs) {
    MC2x2DiagBase::ScalarMultiply(_values, rhs);
    return *this;
  }

  /**
   * Scalar multiplication operator, complex valued rhs
   */
  MC2x2DiagBase<ValType> operator*(std::complex<ValType> rhs) const {
    MC2x2DiagBase<ValType> dest(*this);
    MC2x2DiagBase::ScalarMultiply(dest._values, rhs);
    return dest;
  }

  /**
   * Scalar multiplication assignment operator, complex valued rhs
   */
  MC2x2DiagBase<ValType>& operator*=(std::complex<ValType> rhs) {
    MC2x2DiagBase::ScalarMultiply(_values, rhs);
    return *this;
  }

  /**
   * Scalar division operator
   */
  MC2x2DiagBase<ValType> operator/(ValType rhs) const {
    MC2x2DiagBase<ValType> dest(*this);
    MC2x2DiagBase::ScalarMultiply(dest._values, 1.0 / rhs);
    return dest;
  }

  /**
   * Scalar division assignment operator
   */
  MC2x2DiagBase<ValType>& operator/=(ValType rhs) {
    MC2x2DiagBase::ScalarMultiply(_values, 1.0 / rhs);
    return *this;
  }

  MC2x2DiagBase<ValType> operator+(const MC2x2DiagBase<ValType>& source) {
    return MC2x2DiagBase<ValType>(_values[0] + source._values[0],
                                  _values[1] + source._values[1]);
  }

  MC2x2DiagBase<ValType>& operator+=(const MC2x2DiagBase<ValType>& source) {
    _values[0] += source._values[0];
    _values[1] += source._values[1];
    return *this;
  }

  const std::complex<ValType>& operator[](size_t index) const {
    return _values[index];
  }
  std::complex<ValType>& operator[](size_t index) { return _values[index]; }

  /**
   * Return MC2x2Base matrix filled with zeros
   */
  static MC2x2DiagBase<ValType> Zero() {
    return MC2x2DiagBase<ValType>(0.0, 0.0);
  }

  /**
   * Return 2x2 identity matrix
   */
  static MC2x2DiagBase<ValType> Unity() { return MC2x2DiagBase(1.0, 1.0); }

  /**
   * Return 2x2 matrix filled with NaN values
   */
  static MC2x2DiagBase<ValType> NaN() {
    return MC2x2DiagBase<ValType>(
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
   * Matrix multiplication, alias for the overloaded * operator
   */
  MC2x2DiagBase<ValType> Multiply(const MC2x2DiagBase<ValType>& rhs) const {
    return MC2x2DiagBase<ValType>(_values[0] * rhs[0], _values[1] * rhs[1]);
  }

  /**
   * Scalar multiplication of diagonal matrix.
   */
  template <typename T>
  static void ScalarMultiply(std::complex<T>* dest, T factor) {
    for (size_t p = 0; p != 2; ++p) dest[p] *= factor;
  }

  /**
   * Scalar multiplication of matrix.
   */
  template <typename T>
  static void ScalarMultiply(T* dest, T factor) {
    for (size_t p = 0; p != 2; ++p) dest[p] *= factor;
  }

  /**
   * Compute Hermitian transpose of matrix
   */
  MC2x2DiagBase<ValType> HermTranspose() const {
    return MC2x2DiagBase(std::conj(_values[0]), std::conj(_values[1]));
  }

 private:
  std::complex<ValType> _values[2];
};

/**
 * Diagonal - non-diagonal Matrix multiplication operator
 */
template <typename ValType>
MC2x2Base<ValType> operator*(const MC2x2DiagBase<ValType>& lhs,
                             const MC2x2Base<ValType>& rhs) {
  return MC2x2Base<ValType>(lhs[0] * rhs[0], lhs[0] * rhs[1], lhs[1] * rhs[2],
                            lhs[1] * rhs[3]);
}

/**
 * Non-diagonal - diagonal Matrix multiplication operator
 */
template <typename ValType>
MC2x2Base<ValType> operator*(const MC2x2Base<ValType>& lhs,
                             const MC2x2DiagBase<ValType>& rhs) {
  return MC2x2Base<ValType>(lhs[0] * rhs[0], lhs[1] * rhs[1], lhs[2] * rhs[0],
                            lhs[3] * rhs[1]);
}

/**
 * Obtain the diagonal of a 2x2 matrix
 */
template <typename ValType>
MC2x2DiagBase<ValType> Diagonal(const MC2x2Base<ValType>& matrix) {
  return MC2x2DiagBase<ValType>(matrix[0], matrix[3]);
}

using MC2x2Diag = MC2x2DiagBase<double>;
using MC2x2FDiag = MC2x2DiagBase<float>;

}  // namespace aocommon

#endif
