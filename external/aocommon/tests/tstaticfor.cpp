#include <aocommon/staticfor.h>

#include <mutex>

#include <unistd.h>  // for sleep

#include <boost/test/unit_test.hpp>

using aocommon::StaticFor;

BOOST_AUTO_TEST_SUITE(staticfor)

BOOST_AUTO_TEST_CASE(construct) {
  StaticFor<size_t> sFor(4);
  BOOST_CHECK_EQUAL(sFor.NThreads(), 4);
}

BOOST_AUTO_TEST_CASE(run) {
  StaticFor<size_t> loop(4);
  std::mutex mutex;
  std::vector<size_t> counts(10, 0);
  loop.Run(0, 10, [&](size_t a, size_t b) {
    for (size_t iter = a; iter != b; ++iter) {
      std::unique_lock<std::mutex> lock(mutex);
      counts[iter]++;
    }
  });

  std::vector<size_t> ref(10, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(single_threaded) {
  StaticFor<size_t> loop(1);
  std::vector<size_t> counts(10, 0);
  loop.Run(0, 10, [&](size_t a, size_t b) {
    for (size_t iter = a; iter != b; ++iter) {
      counts[iter]++;
    }
  });

  std::vector<size_t> ref(10, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(resume_run) {
  std::vector<size_t> counts(20, 0);
  std::mutex mutex;
  StaticFor<size_t> loop(40);
  loop.Run(0, 10, [&](size_t a, size_t b) {
    for (size_t iter = a; iter != b; ++iter) {
      std::unique_lock<std::mutex> lock(mutex);
      counts[iter]++;
    }
  });
  std::vector<size_t> ref(20, 0);
  std::fill(ref.begin(), ref.begin() + 10, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());

  loop.Run(10, 20, [&](size_t a, size_t b) {
    for (size_t iter = a; iter != b; ++iter) {
      std::unique_lock<std::mutex> lock(mutex);
      counts[iter]++;
    }
  });
  ref = std::vector<size_t>(20, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(run_with_thread_id) {
  StaticFor<size_t> loop(4);
  std::mutex mutex;
  std::vector<size_t> counts(10, 0);
  std::vector<size_t> threads(4, 0);
  loop.Run(0, 10, [&](size_t a, size_t b, size_t t) {
    for (size_t iter = a; iter != b; ++iter) {
      std::unique_lock<std::mutex> lock(mutex);
      counts[iter]++;
    }
    std::unique_lock<std::mutex> lock(mutex);
    threads[t]++;
  });

  std::vector<size_t> ref(10, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());

  // Not all threads might actually be used, because if one thread
  // finishes before a second thread is starting, the first
  // thread is used to perform the next loop.
  // Therefore all we can check is whether there weren't more than
  // 4 blocks:
  for (size_t i = 0; i != threads.size(); ++i) BOOST_CHECK_LT(threads[i], 5);
}

BOOST_AUTO_TEST_SUITE_END()
