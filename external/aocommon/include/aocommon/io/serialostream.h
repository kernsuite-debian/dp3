#ifndef AO_IO_SERIAL_OSTREAM_H
#define AO_IO_SERIAL_OSTREAM_H

#include "../uvector.h"

#include <complex>
#include <cstdint>
#include <memory>
#include <vector>

namespace aocommon {

class SerialOStream {
 public:
  SerialOStream() {}

  size_t size() const { return _buffer.size(); }

  const unsigned char* data() const { return _buffer.data(); }
  unsigned char* data() { return _buffer.data(); }

  std::string ToString() const {
    return std::string(reinterpret_cast<const char*>(_buffer.data()), size());
  }

  unsigned char* Chunk(size_t size) {
    _buffer.resize(_buffer.size() + size);
    return &*(_buffer.end() - size);
  }

  template <typename T>
  SerialOStream& UInt64(T value) {
    return write<uint64_t>(value);
  }

  template <typename T>
  SerialOStream& UInt32(T value) {
    return write<uint32_t>(value);
  }

  template <typename T>
  SerialOStream& UInt16(T value) {
    return write<uint16_t>(value);
  }

  template <typename T>
  SerialOStream& UInt8(T value) {
    return write<uint8_t>(value);
  }

  SerialOStream& Bool(bool value) { return UInt8(value ? 1 : 0); }

  SerialOStream& Float(float value) { return write(value); }

  SerialOStream& Double(double value) { return write(value); }

  SerialOStream& LDouble(long double value) { return write(value); }

  SerialOStream& CFloat(std::complex<float> value) { return write(value); }

  SerialOStream& CDouble(std::complex<double> value) { return write(value); }

  SerialOStream& CLDouble(std::complex<long double> value) {
    return write(value);
  }

  SerialOStream& String(const std::string& str) {
    UInt64(str.size());
    unsigned char* block = Chunk(str.size());
    std::copy(str.begin(), str.end(), block);
    return *this;
  }

  /**
   * Add a vector of fixed size values to the stream.
   * @tparam T Value type. The value size must be equal on all architectures.
   *         For example float and int32_t always use 32 bits.
   */
  template <typename T>
  SerialOStream& Vector(const std::vector<T>& values) {
    uint64_t size = values.size();
    UInt64(size);
    size_t n = values.size() * sizeof(T);
    unsigned char* block = Chunk(n);
    const unsigned char* valuePtr =
        reinterpret_cast<const unsigned char*>(values.data());
    std::copy_n(valuePtr, n, block);
    return *this;
  }

  /**
   * Add a vector of possibly varying size values to the stream. For example,
   * sizeof(size_t) may differ between architectures. This function always
   * uses 64 bits for encoding the values.
   * @tparam T Value type. The value size may differ between architectures,
   *         but should be at most 64 bits on all architectures.
   */
  template <typename T>
  SerialOStream& VectorUInt64(const std::vector<T>& values) {
    static_assert(sizeof(T) <= sizeof(uint64_t),
                  "Vector value type is larger than 64 bits");
    UInt64(values.size());
    for (const T& v : values) UInt64(v);
    return *this;
  }

  /**
   * Write an object to the stream using its Serialize function.
   * @tparam T Object type, which must implement Serialize(SerialOStream&).
   * @param object The object instance that must be written.
   * @return A reference to the stream, for chaining writes.
   */
  template <typename T>
  SerialOStream& Object(const T& object) {
    object.Serialize(*this);
    return *this;
  }

  /**
   * Write a vector of objects to the stream.
   * @tparam T Object type, which must implement Serialize(SerialOStream&)
   */
  template <typename T>
  SerialOStream& ObjectVector(const std::vector<T>& objects) {
    UInt64(objects.size());
    for (const T& o : objects) o.Serialize(*this);
    return *this;
  }

  template <typename T>
  SerialOStream& Ptr(const std::unique_ptr<T>& ptr) {
    return writePtr(ptr);
  }

  template <typename T>
  SerialOStream& Ptr(const std::shared_ptr<T>& ptr) {
    return writePtr(ptr);
  }

 private:
  friend class SerialIStream;

  template <typename T>
  SerialOStream& write(T value) {
    *reinterpret_cast<T*>(Chunk(sizeof(T))) = value;
    return *this;
  }

  template <typename PtrT>
  SerialOStream& writePtr(const PtrT& ptr) {
    if (ptr) {
      Bool(true);
      ptr->Serialize(*this);
    } else {
      Bool(false);
    }
    return *this;
  }

  UVector<unsigned char> _buffer;
};

}  // namespace aocommon

#endif
