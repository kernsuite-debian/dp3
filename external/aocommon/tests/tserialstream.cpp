#include <boost/test/unit_test.hpp>

#include <aocommon/io/serialistream.h>
#include <aocommon/io/serialostream.h>

#include <limits>

using aocommon::SerialIStream;
using aocommon::SerialOStream;

BOOST_AUTO_TEST_SUITE(serialization)

BOOST_AUTO_TEST_CASE(one_by_one_input_syntax) {
  SerialOStream ostr;
  ostr.Bool(true)
      .UInt8(80)
      .UInt16(160)
      .UInt32(320)
      .UInt64(640)
      .Float(1.5)
      .Double(3.14)
      .LDouble(2.71)
      .String("hi!");

  SerialIStream istr(std::move(ostr));

  BOOST_CHECK_EQUAL(istr.Bool(), true);
  BOOST_CHECK_EQUAL(istr.UInt8(), 80u);
  BOOST_CHECK_EQUAL(istr.UInt16(), 160u);
  BOOST_CHECK_EQUAL(istr.UInt32(), 320u);
  BOOST_CHECK_EQUAL(istr.UInt64(), 640u);
  BOOST_CHECK_EQUAL(istr.Float(), 1.5);
  BOOST_CHECK_EQUAL(istr.Double(), 3.14);
  BOOST_CHECK_EQUAL(istr.LDouble(), 2.71);
  BOOST_CHECK_EQUAL(istr.String(), "hi!");
}

BOOST_AUTO_TEST_CASE(concatenated_input_syntax) {
  SerialOStream ostr;
  ostr.Bool(true)
      .UInt8(80)
      .UInt16(160)
      .UInt32(320)
      .UInt64(640)
      .Float(1.5)
      .Double(3.14)
      .LDouble(2.71)
      .String("hi!");

  bool b = false;
  uint8_t u8 = 0;
  uint16_t u16 = 0;
  uint32_t u32 = 0;
  uint64_t u64 = 0;
  float f = 0.0f;
  double d = 0.0;
  long double ld = 0.0;
  std::string s;

  SerialIStream istr(std::move(ostr));
  istr.Bool(b)
      .UInt8(u8)
      .UInt16(u16)
      .UInt32(u32)
      .UInt64(u64)
      .Float(f)
      .Double(d)
      .LDouble(ld)
      .String(s);

  BOOST_CHECK_EQUAL(b, true);
  BOOST_CHECK_EQUAL(u8, 80u);
  BOOST_CHECK_EQUAL(u16, 160u);
  BOOST_CHECK_EQUAL(u32, 320u);
  BOOST_CHECK_EQUAL(u64, 640u);
  BOOST_CHECK_EQUAL(f, 1.5);
  BOOST_CHECK_EQUAL(d, 3.14);
  BOOST_CHECK_EQUAL(ld, 2.71);
  BOOST_CHECK_EQUAL(s, "hi!");
}

BOOST_AUTO_TEST_CASE(vector) {
  const std::vector<std::int32_t> int32_a;
  const std::vector<std::int32_t> int32_b{12, -13, 14};
  const std::vector<std::uint64_t> uint64_a;
  const std::vector<std::uint64_t> uint64_b{18, 19, 20};
  const std::vector<std::complex<float>> cf_a;
  const std::vector<std::complex<float>> cf_b{{1, 1}, {-2, -2}, {3, 3}};

  SerialOStream ostr;
  ostr.Vector(int32_a)
      .Vector(int32_b)
      .Vector(uint64_a)
      .Vector(uint64_b)
      .Vector(cf_a)
      .Vector(cf_b);

  SerialIStream istr(std::move(ostr));

  std::vector<std::int32_t> out_int32_a, out_int32_b;
  std::vector<std::uint64_t> out_uint64_a, out_uint64_b;
  std::vector<std::complex<float>> out_cf_a, out_cf_b;
  istr.Vector(out_int32_a)
      .Vector(out_int32_b)
      .Vector(out_uint64_a)
      .Vector(out_uint64_b)
      .Vector(out_cf_a)
      .Vector(out_cf_b);
  BOOST_CHECK(out_int32_a.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(int32_b.begin(), int32_b.end(),
                                out_int32_b.begin(), out_int32_b.end());
  BOOST_CHECK(out_uint64_a.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(uint64_b.begin(), uint64_b.end(),
                                out_uint64_b.begin(), out_uint64_b.end());
  BOOST_CHECK(out_cf_a.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(cf_b.begin(), cf_b.end(), out_cf_b.begin(),
                                out_cf_b.end());
}

BOOST_AUTO_TEST_CASE(vector64) {
  const std::vector<std::int8_t> int8_a;
  const std::vector<std::int8_t> int8_b{
      0, std::numeric_limits<std::int8_t>::min(),
      std::numeric_limits<std::int8_t>::max()};
  const std::vector<std::int64_t> int64_a;
  const std::vector<std::int64_t> int64_b{
      0, std::numeric_limits<std::int64_t>::min(),
      std::numeric_limits<std::int64_t>::max()};

  SerialOStream ostr;
  ostr.VectorUInt64(int8_a)
      .VectorUInt64(int8_b)
      .VectorUInt64(int64_a)
      .VectorUInt64(int64_b);

  SerialIStream istr(std::move(ostr));

  std::vector<std::int8_t> out_int8_a, out_int8_b;
  std::vector<std::int64_t> out_int64_a, out_int64_b;
  istr.VectorUInt64(out_int8_a)
      .VectorUInt64(out_int8_b)
      .VectorUInt64(out_int64_b)
      .VectorUInt64(out_int64_b);

  BOOST_CHECK(out_int8_a.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(int8_b.begin(), int8_b.end(),
                                out_int8_b.begin(), out_int8_b.end());
  BOOST_CHECK(out_int64_a.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(int64_b.begin(), int64_b.end(),
                                out_int64_b.begin(), out_int64_b.end());
}

struct Serializable {
  // By updating the stream pointer members, the test can check that these
  // functions were called with the proper argument.
  void Serialize(SerialOStream& stream) { ostream = &stream; }
  void Unserialize(SerialIStream& stream) { istream = &stream; }

  SerialIStream* istream = nullptr;
  SerialOStream* ostream = nullptr;
};

BOOST_AUTO_TEST_CASE(pointer) {
  std::shared_ptr<Serializable> shared_empty;
  std::shared_ptr<Serializable> shared(new Serializable());
  std::unique_ptr<Serializable> unique_empty;
  std::unique_ptr<Serializable> unique(new Serializable());

  SerialOStream ostr;
  ostr.Ptr(shared_empty).Ptr(shared).Ptr(unique_empty).Ptr(unique);
  BOOST_CHECK_EQUAL(shared->ostream, &ostr);
  BOOST_CHECK_EQUAL(unique->ostream, &ostr);
  BOOST_CHECK_EQUAL(shared->istream, nullptr);
  BOOST_CHECK_EQUAL(unique->istream, nullptr);

  SerialIStream istr(std::move(ostr));

  std::shared_ptr<Serializable> out_shared_empty;
  std::shared_ptr<Serializable> out_shared;
  std::unique_ptr<Serializable> out_unique_empty;
  std::unique_ptr<Serializable> out_unique;
  istr.Ptr(out_shared_empty)
      .Ptr(out_shared)
      .Ptr(out_unique_empty)
      .Ptr(out_unique);
  BOOST_CHECK(!out_shared_empty);
  BOOST_CHECK(!out_unique_empty);
  BOOST_REQUIRE(out_shared);
  BOOST_REQUIRE(out_unique);
  BOOST_CHECK_EQUAL(out_shared->ostream, nullptr);
  BOOST_CHECK_EQUAL(out_unique->ostream, nullptr);
  BOOST_CHECK_EQUAL(out_shared->istream, &istr);
  BOOST_CHECK_EQUAL(out_unique->istream, &istr);
}

struct TestObject {
  uint64_t a, b;
  void Serialize(SerialOStream& stream) const { stream.UInt64(a).UInt64(b); }
  void Unserialize(SerialIStream& stream) { stream.UInt64(a).UInt64(b); }
  // The Boost checks require operator== and operator!= to be implemented.
  bool operator==(const TestObject& rhs) const {
    return a == rhs.a && b == rhs.b;
  }
  bool operator!=(const TestObject& rhs) const { return !operator==(rhs); }
};
void operator<<(std::ostream& str, const TestObject& obj) {
  str << "TestObject{" << obj.a << ", " << obj.b << '}';
}

BOOST_AUTO_TEST_CASE(object) {
  const TestObject in{42, 4242};
  SerialOStream ostr;
  BOOST_CHECK_EQUAL(&ostr.Object(in), &ostr);

  SerialIStream istr(std::move(ostr));
  TestObject out;
  BOOST_CHECK_EQUAL(&istr.Object(out), &istr);
  BOOST_CHECK_EQUAL(out, in);
}

BOOST_AUTO_TEST_CASE(object_vector) {
  const std::vector<TestObject> empty;
  const std::vector<TestObject> filled{{13, 37}, {0, 42}};

  SerialOStream ostr;
  ostr.ObjectVector(empty)
      .ObjectVector(filled)
      .ObjectVector(empty)
      .ObjectVector(filled);

  SerialIStream istr(std::move(ostr));

  std::vector<TestObject> out_empty1;
  std::vector<TestObject> out_empty2;
  std::vector<TestObject> out_filled1;
  std::vector<TestObject> out_filled2;
  istr.ObjectVector(out_empty1)
      .ObjectVector(out_filled1)
      .ObjectVector(out_empty2)
      .ObjectVector(out_filled2);

  BOOST_CHECK(out_empty1.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(filled.begin(), filled.end(),
                                out_filled1.begin(), out_filled1.end());
  BOOST_CHECK(out_empty2.empty());
  BOOST_CHECK_EQUAL_COLLECTIONS(filled.begin(), filled.end(),
                                out_filled2.begin(), out_filled2.end());
}

BOOST_AUTO_TEST_SUITE_END()
