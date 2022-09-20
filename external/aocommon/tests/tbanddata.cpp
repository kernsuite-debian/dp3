#include <boost/test/unit_test.hpp>

#include <aocommon/banddata.h>

using aocommon::BandData;
using aocommon::ChannelInfo;

BOOST_AUTO_TEST_SUITE(banddata)

BOOST_AUTO_TEST_CASE(construction) {
  BandData bandData1;
  BOOST_CHECK_EQUAL(bandData1.ChannelCount(), 0);

  // Check that these methods don't crash:
  bandData1.CentreFrequency();
  bandData1.CentreWavelength();
  bandData1.FrequencyStep();
  bandData1.ReferenceFrequency();
  bandData1.HighestFrequency();
  bandData1.LongestWavelength();
  bandData1.LowestFrequency();
  bandData1.SmallestWavelength();

  std::vector<ChannelInfo> channels{ChannelInfo(150e6, 10e6),
                                    ChannelInfo(160e6, 10e6),
                                    ChannelInfo(170e6, 10e6)};
  BandData bandData2(channels, 160e6);
  BOOST_CHECK_EQUAL(bandData2.ChannelCount(), 3);
  BOOST_CHECK_CLOSE_FRACTION(bandData2.ChannelFrequency(0), 150e6, 1e-5);
  BOOST_CHECK_CLOSE_FRACTION(bandData2.ChannelFrequency(1), 160e6, 1e-5);
  BOOST_CHECK_CLOSE_FRACTION(bandData2.ChannelFrequency(2), 170e6, 1e-5);
  BOOST_CHECK_CLOSE_FRACTION(bandData2.FrequencyStep(), 10e6, 1e-5);
  bandData2 = bandData1;
  BOOST_CHECK_EQUAL(bandData2.ChannelCount(), 0);
  bandData2 = BandData(channels, 160e6);
  BOOST_CHECK_EQUAL(bandData2.ChannelCount(), 3);
  BandData bandData3(std::move(bandData2));
  BOOST_CHECK_EQUAL(bandData3.ChannelCount(), 3);
  BandData bandData4(bandData3);
  BOOST_CHECK_EQUAL(bandData4.ChannelCount(), 3);
  BOOST_CHECK_CLOSE_FRACTION(bandData4.ChannelFrequency(0), 150e6, 1e-5);
  BOOST_CHECK_CLOSE_FRACTION(bandData4.ChannelFrequency(1), 160e6, 1e-5);
  BOOST_CHECK_CLOSE_FRACTION(bandData4.ChannelFrequency(2), 170e6, 1e-5);
  BOOST_CHECK_EQUAL(bandData4.ReferenceFrequency(), 160e6);
  std::vector<double> newFreqs = {120e6, 130e6};
  bandData4.Set(2, newFreqs.data());
  BOOST_CHECK_EQUAL(bandData4.ChannelFrequency(0), 120e6);
  BOOST_CHECK_EQUAL(bandData4.ChannelFrequency(1), 130e6);
}

BOOST_AUTO_TEST_CASE(copy_assignment) {
  std::vector<ChannelInfo> channelsA{ChannelInfo(150e6, 5e6),
                                     ChannelInfo(160e6, 5e6),
                                     ChannelInfo(170e6, 5e6)};
  std::vector<ChannelInfo> channelsB{
      ChannelInfo(110e6, 10e6), ChannelInfo(120e6, 10e6),
      ChannelInfo(130e6, 10e6), ChannelInfo(140e6, 10e6)};
  BandData a(channelsA, 160e6), b(channelsB, 140e6);
  a = b;
  BOOST_CHECK_EQUAL(a.ChannelCount(), 4);
  BOOST_CHECK_EQUAL(a.Channel(0).Frequency(), 110e6);
  BOOST_CHECK_EQUAL(a.Channel(1).Frequency(), 120e6);
  BOOST_CHECK_EQUAL(a.Channel(2).Frequency(), 130e6);
  BOOST_CHECK_EQUAL(a.Channel(3).Frequency(), 140e6);
  BOOST_CHECK_EQUAL(a.Channel(0).Width(), 10e6);
  BOOST_CHECK_EQUAL(a.Channel(1).Width(), 10e6);
  BOOST_CHECK_EQUAL(a.Channel(2).Width(), 10e6);
  BOOST_CHECK_EQUAL(a.Channel(3).Width(), 10e6);
}

BOOST_AUTO_TEST_CASE(extrema_and_reference) {
  std::vector<ChannelInfo> channelsA{ChannelInfo(170e6, 5e6),
                                     ChannelInfo(160e6, 5e6),
                                     ChannelInfo(150e6, 5e6)};
  BandData a(channelsA, 160e6);
  BOOST_CHECK_EQUAL(a.HighestFrequency(), 170e6);
  BOOST_CHECK_EQUAL(a.LowestFrequency(), 150e6);
  BOOST_CHECK_EQUAL(a.ReferenceFrequency(), 160e6);
  BOOST_CHECK_CLOSE(a.SmallestWavelength(), aocommon::c() / 170e6, 1e-8);
  BOOST_CHECK_CLOSE(a.LongestWavelength(), aocommon::c() / 150e6, 1e-8);
}

BOOST_AUTO_TEST_SUITE_END()
