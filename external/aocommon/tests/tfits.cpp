#include <boost/test/unit_test.hpp>

#include <aocommon/fits/fitsreader.h>
#include <aocommon/fits/fitswriter.h>

BOOST_AUTO_TEST_SUITE(fits)

BOOST_AUTO_TEST_CASE(write_and_read) {
  const size_t width = 32;
  const size_t height = 16;
  const double dl = 0.0125;
  const double dm = 0.025;
  const size_t val = 42;

  aocommon::UVector<double> image(width * height, val);

  // Write an image
  aocommon::FitsWriter writer;
  writer.SetImageDimensions(width, height, dl, dm);
  writer.AddHistory("aocommon test write");
  writer.Write("test-image.fits", image.data());

  // Read the image
  aocommon::FitsReader reader("test-image.fits");
  aocommon::UVector<double> imageFromDisk(width * height);
  reader.Read(imageFromDisk.data());

  BOOST_CHECK_EQUAL(reader.ImageWidth(), width);
  BOOST_CHECK_EQUAL(reader.ImageHeight(), height);
  BOOST_CHECK_CLOSE(reader.PixelSizeX(), dl, 1e-8);
  BOOST_CHECK_CLOSE(reader.PixelSizeY(), dm, 1e-8);
  BOOST_CHECK_EQUAL(image[0], val);
  BOOST_CHECK_EQUAL_COLLECTIONS(image.begin(), image.end(),
                                imageFromDisk.begin(), imageFromDisk.end());

  BOOST_CHECK_EQUAL(reader.History().size(), 1);
  BOOST_CHECK_EQUAL(reader.History()[0], "aocommon test write");

  std::string s = "unset";
  BOOST_CHECK(!reader.ReadStringKeyIfExists("nonexist", s));
  BOOST_CHECK_EQUAL(s, "unset");
  double d;
  BOOST_CHECK(!reader.ReadDoubleKeyIfExists("nonexist", d));
}

BOOST_AUTO_TEST_CASE(fitsbase) {
  class FitsTester : public aocommon::FitsBase {
   public:
    void CheckStatus(int status, const std::string& filename) {
      checkStatus(status, filename);
    }
    void CheckStatus(int status, const std::string& filename,
                     const std::string& operation) {
      checkStatus(status, filename, operation);
    }
  } tester;
  // Should not throw:
  tester.CheckStatus(0, "some_file.fits");
  tester.CheckStatus(0, "some_file.fits", "operation");
  // Exception should contain file and operation:
  try {
    tester.CheckStatus(WRITE_ERROR, "some_file.fits");
    BOOST_FAIL("Exception should be thrown");
  } catch (std::runtime_error& e) {
    std::string str = e.what();
    BOOST_CHECK_NE(str.find("some_file.fits"), std::string::npos);
  }
  try {
    tester.CheckStatus(FILE_NOT_OPENED, "some_file.fits", "operation");
    BOOST_FAIL("Exception should be thrown");
  } catch (std::runtime_error& e) {
    std::string str = e.what();
    BOOST_CHECK_NE(str.find("some_file.fits"), std::string::npos);
    BOOST_CHECK_NE(str.find("operation"), std::string::npos);
  }
}

BOOST_AUTO_TEST_SUITE_END()
