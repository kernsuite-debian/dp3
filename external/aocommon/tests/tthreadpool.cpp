#include <iostream>
#include <cmath>

#include <unistd.h>  // for usleep

#include <aocommon/threadpool.h>

#include <boost/test/unit_test.hpp>

using namespace aocommon;

BOOST_AUTO_TEST_SUITE(threadpool)

BOOST_AUTO_TEST_CASE(empty) {
  ThreadPool();
  BOOST_CHECK(true);

  BOOST_CHECK_THROW(ThreadPool(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(single) {
  ThreadPool pool;
  std::mutex mutex;
  std::vector<size_t> counts(10, 0);
  pool.For(0, 10, [&](size_t iter, size_t) {
    std::unique_lock<std::mutex> lock(mutex);
    counts[iter]++;
  });
  std::vector<size_t> ref(10, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(recursive) {
  ThreadPool pool;
  std::mutex mutex;
  std::vector<size_t> counts(800, 0);
  pool.For(0, 100, [&](size_t iter1, size_t) {
    pool.For(0, 8, [&](size_t iter2, size_t) {
      std::unique_lock<std::mutex> lock(mutex);
      counts[iter1 + iter2 * 100]++;
    });
  });
  std::vector<size_t> ref(800, 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(slow_task) {
  std::mutex mutex;
  ThreadPool pool;
  std::vector<size_t> counts(5 * pool.NThreads(), 0);
  pool.For(0, 5 * pool.NThreads(), [&](size_t iter, size_t) {
    usleep(1000);
    std::unique_lock<std::mutex> lock(mutex);
    counts[iter]++;
  });
  std::vector<size_t> ref(5 * pool.NThreads(), 1);
  BOOST_CHECK_EQUAL_COLLECTIONS(counts.begin(), counts.end(), ref.begin(),
                                ref.end());
}

BOOST_AUTO_TEST_CASE(reuse) {
  ThreadPool pool;
  for (size_t i = 0; i != 100; ++i) {
    volatile double x = 0.1;
    pool.For(0, 100, [&](size_t iter, size_t) { (void)sin(x); });
  }
}

BOOST_AUTO_TEST_CASE(set_n_threads) {
  ThreadPool pool(1);
  BOOST_CHECK_EQUAL(pool.NThreads(), 1);
  pool.SetNThreads(4);
  BOOST_CHECK_EQUAL(pool.NThreads(), 4);
}

BOOST_AUTO_TEST_SUITE_END()
