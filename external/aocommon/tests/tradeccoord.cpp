#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <aocommon/radeccoord.h>

using aocommon::RaDecCoord;

BOOST_AUTO_TEST_SUITE(ra_dec_coord)

const std::string raValues[15] = {"00h00m00s",   "00h00m00.0s", "00h00m00.00s",
                                  "12h00m00.0s", "00:00:00.0",  "-00:00:00.0",
                                  "-00:01:00.0", "-23:59:59.9", "-23:59:59.99",
                                  "11:59:59.9",  "-11:59:59.9", "23:59:59.9",
                                  "23:59:59.90", "23:59:59.99", "23:59:59.999"};

const std::string badRaValues[8] = {"",       "00h00m00.00-", "00h00m00.00s!",
                                    "?",      "00h",          "00h00m",
                                    "00:00*", "00:00:00.0x"};

static const std::string decValues[11] = {
    "00d00m00.0s",  "90d00m00.0s",  "00.00.00.0",   "-00d00m00s",
    "-00d01m00.0s", "89.59.59.9",   "89d59m59.99s", "-89.59.59.9",
    "-89.59.59.90", "-89.59.59.99", "-89.59.59.999"};

static const std::string badDecValues[7] = {
    "", "-", "00.00.00.a", "-00d", "-00d01", "01d02m03", "05.04a"};

BOOST_DATA_TEST_CASE(testRAParsesConsistently,
                     boost::unit_test::data::make(raValues)) {
  long double val = RaDecCoord::ParseRA(sample);
  if (val < 0.0) val += 2.0L * M_PIl;
  std::string recomposed = RaDecCoord::RAToString(val);

  BOOST_CHECK_CLOSE_FRACTION(val, RaDecCoord::ParseRA(recomposed), 1e-9);
}

BOOST_DATA_TEST_CASE(testRAToString, boost::unit_test::data::make(raValues)) {
  double val = RaDecCoord::ParseRA(sample);
  if (val < 0.0) val += 2.0L * M_PIl;
  std::string recomposed = RaDecCoord::RAToString(val);

  BOOST_CHECK_CLOSE_FRACTION(val, RaDecCoord::ParseRA(recomposed), 1e-9);
}

BOOST_AUTO_TEST_CASE(testNegativeRAToString) {
  // A negative RA should parse fine and produce a negative
  // value in radians:
  double val = RaDecCoord::ParseRA("-10h30m00s");
  BOOST_CHECK_CLOSE_FRACTION(val, -10.5 * M_PIl / 12.0L, 1e-9);

  // A negative value in radians should however still produce a
  // "positive" RA string, because it is uncommon to write
  // an RA as "-10h30m".
  std::string recomposed = RaDecCoord::RAToString(val);
  BOOST_CHECK_EQUAL(recomposed.substr(0, 10), "13h30m00.0");
}

BOOST_DATA_TEST_CASE(testRAParseException,
                     boost::unit_test::data::make(badRaValues)) {
  BOOST_CHECK_THROW(RaDecCoord::ParseRA(sample), std::runtime_error);
}

BOOST_DATA_TEST_CASE(testDeclinationParsesConsistently,
                     boost::unit_test::data::make(decValues)) {
  double val = RaDecCoord::ParseDec(sample);
  std::string recomposed = RaDecCoord::DecToString(val);
  BOOST_CHECK_EQUAL(RaDecCoord::ParseDec(sample),
                    RaDecCoord::ParseDec(recomposed));
}

BOOST_DATA_TEST_CASE(testDeclinationToString,
                     boost::unit_test::data::make(decValues)) {
  double val = RaDecCoord::ParseDec(sample);
  std::string recomposed = RaDecCoord::DecToString(val);
  BOOST_CHECK_EQUAL(recomposed,
                    RaDecCoord::DecToString(RaDecCoord::ParseDec(recomposed)));
}

BOOST_DATA_TEST_CASE(testDeclinationParseException,
                     boost::unit_test::data::make(badDecValues)) {
  BOOST_CHECK_THROW(RaDecCoord::ParseDec(sample), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
