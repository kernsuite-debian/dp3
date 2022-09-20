#ifndef AO_IO_SERIAL_ISTREAM_H
#define AO_IO_SERIAL_ISTREAM_H

#include "../uvector.h"
#include "serialostream.h"

#include <complex>
#include <cstdint>
#include <memory>
#include <vector>

namespace aocommon {

class SerialIStream {
 public:
  SerialIStream(UVector<unsigned char>&& buffer)
      : _buffer(std::move(buffer)), _position(_buffer.begin()) {}

  SerialIStream(SerialOStream&& oStream)
      : _buffer(std::move(oStream._buffer)), _position(_buffer.begin()) {}

  size_t size() const { return _buffer.size(); }

  const unsigned char* data() const { return _buffer.data(); }

  std::string ToString() const {
    return std::string(reinterpret_cast<const char*>(_buffer.data()), size());
  }

  const unsigned char* Chunk(size_t size) {
    const unsigned char* chunk = &*_position;
    _position += size;
    return chunk;
  }

  template <typename T>
  SerialIStream& UInt64(T& value) {
    value = (T)read<uint64_t>();
    return *this;
  }

  uint64_t UInt64() { return read<uint64_t>(); }

  template <typename T>
  SerialIStream& UInt32(T& value) {
    value = (T)read<uint32_t>();
    return *this;
  }

  uint32_t UInt32() { return read<uint32_t>(); }

  template <typename T>
  SerialIStream& UInt16(T& value) {
    value = (T)read<uint16_t>();
    return *this;
  }

  uint16_t UInt16() { return read<uint16_t>(); }

  template <typename T>
  SerialIStream& UInt8(T& value) {
    value = (T)read<uint8_t>();
    return *this;
  }

  uint8_t UInt8() { return read<uint8_t>(); }

  SerialIStream& Bool(bool& value) {
    value = (UInt8() != 0);
    return *this;
  }

  bool Bool() { return UInt8() != 0; }

  SerialIStream& Float(float& value) { return read(value); }

  float Float() { return read<float>(); }

  SerialIStream& Double(double& value) { return read(value); }

  double Double() { return read<double>(); }

  SerialIStream& LDouble(long double& value) { return read(value); }

  long double LDouble() { return read<long double>(); }

  SerialIStream& CFloat(std::complex<float>& value) { return read(value); }

  std::complex<float> CFloat() { return read<std::complex<float>>(); }

  SerialIStream& CDouble(std::complex<double>& value) { return read(value); }

  std::complex<double> CDouble() { return read<std::complex<double>>(); }

  SerialIStream& CLDouble(std::complex<long double>& value) {
    return read(value);
  }

  long double CLDouble() { return read<long double>(); }

  SerialIStream& String(std::string& str) {
    size_t n = UInt64();
    const unsigned char* block = Chunk(n);
    str.resize(n);
    std::copy_n(block, n, str.begin());
    return *this;
  }

  std::string String() {
    size_t n = UInt64();
    const unsigned char* block = Chunk(n);
    std::string str(n, 0);
    std::copy_n(block, n, str.begin());
    return str;
  }

  /**
   * Read a vector of fixed-size literal values from the stream.
   * @tparam T A type whose size is fixed across architectures, e.g. int32_t.
   */
  template <typename T>
  SerialIStream& Vector(std::vector<T>& values) {
    uint64_t size = UInt64();
    values.resize(size);
    size_t n = size * sizeof(T);
    const unsigned char* block = Chunk(n);
    unsigned char* valuePtr = reinterpret_cast<unsigned char*>(values.data());
    std::copy_n(block, n, valuePtr);
    return *this;
  }

  /**
   * Read a vector with 64-bit unsigned integers from the stream into a vector
   * of non-fixed-sized literal values.
   * @tparam T A literal type whose size may differ across architectures, e.g.
   * size_t. The size should fit in 64 bits on all architectures.
   */
  template <typename T>
  SerialIStream& VectorUInt64(std::vector<T>& values) {
    static_assert(sizeof(T) <= sizeof(uint64_t),
                  "Vector value type is larger than 64 bits");
    uint64_t size = UInt64();
    values.clear();
    values.reserve(size);
    for (size_t i = 0; i < size; ++i) {
      values.push_back(UInt64());
    }
    return *this;
  }

  /**
   * Read an object from the stream using its Unserialize function.
   * @tparam T Object type, which must implement Unserialize(SerialIStream&).
   * @param object The object instance that must be read.
   * @return A reference to the stream, for chaining reads.
   */
  template <typename T>
  SerialIStream& Object(T& object) {
    object.Unserialize(*this);
    return *this;
  }

  /**
   * Read a vector of objects from the stream.
   * @tparam T Object type, which must implement Unserialize(SerialIStream&).
   */
  template <typename T>
  SerialIStream& ObjectVector(std::vector<T>& objects) {
    uint64_t size = UInt64();
    objects.resize(size);
    for (T& object : objects) {
      object.Unserialize(*this);
    }
    return *this;
  }

  template <typename T>
  SerialIStream& Ptr(std::unique_ptr<T>& ptr) {
    return readPtr(ptr);
  }

  template <typename T>
  SerialIStream& Ptr(std::shared_ptr<T>& ptr) {
    return readPtr(ptr);
  }

 private:
  template <typename T>
  SerialIStream& read(T& value) {
    value = *reinterpret_cast<const T*>(Chunk(sizeof(T)));
    return *this;
  }

  template <typename T>
  T read() {
    return *reinterpret_cast<const T*>(Chunk(sizeof(T)));
  }

  template <typename PtrT>
  SerialIStream& readPtr(PtrT& ptr) {
    if (Bool()) {
      ptr.reset(new typename std::remove_reference<decltype(*ptr)>::type());
      ptr->Unserialize(*this);
    } else {
      ptr.reset();
    }
    return *this;
  }

  UVector<unsigned char> _buffer;
  UVector<unsigned char>::const_iterator _position;
};

}  // namespace aocommon

#endif
