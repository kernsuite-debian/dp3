
#include <aocommon/queue.h>

#include <boost/test/unit_test.hpp>

#include <memory>
#include <type_traits>
#include <vector>

using aocommon::Queue;

BOOST_AUTO_TEST_SUITE(queue)

BOOST_AUTO_TEST_CASE(constructor) {
  const Queue<int> q;
  BOOST_CHECK(q.Empty());
  BOOST_CHECK_EQUAL(q.Size(), 0u);
  BOOST_CHECK(q.begin() == q.end());
  // The initial capacity shouldn't be too much.
  BOOST_CHECK_LE(q.Capacity(), 16u);
}

BOOST_AUTO_TEST_CASE(push_pop_push) {
  Queue<std::pair<int, int>> q;  // Use a non-pod value type.

  // Test lvalue push back.
  const std::pair<int, int> value(42, 43);
  const std::pair<int, int>& pushed_lvalue = q.PushBack(value);  // lvalue
  BOOST_CHECK_EQUAL(&pushed_lvalue, &q[0]);
  BOOST_CHECK_EQUAL(&*q.begin(), &q[0]);
  BOOST_CHECK_EQUAL(q.end() - q.begin(), 1);
  BOOST_CHECK_EQUAL(q.Size(), 1u);
  BOOST_CHECK_EQUAL(q[0].first, 42);
  BOOST_CHECK_EQUAL(q[0].second, 43);

  q.PopFront();
  BOOST_CHECK(q.Empty());

  // Test rvalue push back.
  const std::pair<int, int>& pushed_rvalue = q.PushBack(std::make_pair(52, 53));
  BOOST_CHECK(q.Size() == 1u);
  BOOST_CHECK_EQUAL(&pushed_rvalue, &q[0]);
  BOOST_CHECK(&*q.begin() == &q[0]);
  BOOST_CHECK_EQUAL(q.end() - q.begin(), 1);
  BOOST_CHECK_EQUAL(q[0].first, 52);
  BOOST_CHECK_EQUAL(q[0].second, 53);
}

BOOST_AUTO_TEST_CASE(push50_pop50_push50) {
  const int n = 50;
  Queue<int> q;

  for (int i = 0; i < n; ++i) q.PushBack(i);
  BOOST_CHECK_EQUAL(q.end() - q.begin(), n);
  BOOST_CHECK_EQUAL(q.Size(), std::size_t(n));
  for (int i = 0; i < n; ++i) BOOST_CHECK_EQUAL(q[i], i);

  for (int i = 0; i < n; ++i) {
    q.PopFront();
    BOOST_CHECK_EQUAL(q.Size(), std::size_t(n - i - 1));
  }

  for (int i = 0; i < n; ++i) q.PushBack(i);
  BOOST_CHECK_EQUAL(q.end() - q.begin(), n);
  BOOST_CHECK_EQUAL(q.Size(), std::size_t(n));
  for (int i = 0; i < n; ++i) BOOST_CHECK_EQUAL(q[i], i);

  // Check that the capacity is still reasonable.
  BOOST_CHECK_LT(q.Capacity(), std::size_t(2 * n));
}

BOOST_AUTO_TEST_CASE(clear) {
  Queue<int> q;
  const std::size_t initial_capacity = q.Capacity();

  for (int i = 0; i < 42; ++i) q.PushBack(i);
  BOOST_CHECK_EQUAL(q.Size(), 42u);
  const std::size_t new_capacity = q.Capacity();
  BOOST_REQUIRE_GT(new_capacity, initial_capacity);

  q.Clear();
  BOOST_CHECK(q.Empty());
  BOOST_CHECK_EQUAL(q.Size(), 0u);
  BOOST_CHECK_EQUAL(q.Capacity(), new_capacity);
}

/// Test creating a vector of Queues with move-only types, which is a use case.
/// When a Queue contains move-only values, such as std::unique_ptr, a
/// std::vector<Queue<std::unique_ptr<X>>> only works if Queue has a noexcept
/// move constructor. (A std::vector<std::deque<std::unique_ptr<X>> does not
/// work, for that reason: The compiler will complain.)
BOOST_AUTO_TEST_CASE(vector_of_fifo_of_move_only_type) {
  // Check using type traits.
  static_assert(
      std::is_nothrow_move_constructible<Queue<std::unique_ptr<int>>>::value,
      "Queue is 'nothrow move constructible'");

  // Check by creating a vector of Fifos.
  std::vector<Queue<std::unique_ptr<int>>> queues(1);
  queues.emplace_back();
  // This test is a mainly compile-time test -> No further checks are needed.
}

BOOST_AUTO_TEST_SUITE_END()
