#include <iostream>

#include <aocommon/uvector.h>

#include <fstream>
#include <vector>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(UVector)

template <typename Tp>
typename aocommon::UVector<Tp>::iterator insert_uninitialized(
    aocommon::UVector<Tp>& vec,
    typename aocommon::UVector<Tp>::const_iterator i, size_t count) {
  return vec.insert_uninitialized(i, count);
}

template <typename Tp>
typename std::vector<Tp>::iterator insert_uninitialized(
    std::vector<Tp>& vec, typename std::vector<Tp>::iterator i, size_t count) {
  size_t index = i - vec.begin();
  vec.insert(i, count, Tp());
  return vec.begin() + index;
}

template <typename Vec>
void test() {
  BOOST_CHECK(Vec().empty());
  BOOST_CHECK(!Vec(1).empty());
  BOOST_CHECK(Vec().capacity() == 0);
  Vec vec(1000);
  BOOST_CHECK(!vec.empty());
  BOOST_CHECK(vec.size() == 1000);
  BOOST_CHECK(vec.capacity() >= 1000);

  vec.push_back(16);
  BOOST_CHECK(vec.size() == 1001);
  BOOST_CHECK(vec[1000] == 16);

  vec.push_back(17);
  BOOST_CHECK(vec.size() == 1002);
  BOOST_CHECK(vec[1001] == 17);

  vec[0] = 1337;
  BOOST_CHECK(vec[0] == 1337);

  vec.pop_back();
  BOOST_CHECK(vec.size() == 1001);
  BOOST_CHECK(*vec.begin() == 1337);
  BOOST_CHECK(*(vec.end() - 1) == 16);
  BOOST_CHECK(*vec.rbegin() == 16);
  BOOST_CHECK(*(vec.rend() - 1) == 1337);

  vec = Vec(100, 1);
  BOOST_CHECK(vec.size() == 100);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[99] == 1);

  // construct with range
  vec[0] = 2;
  vec[99] = 3;
  vec = Vec(vec.rbegin(), vec.rend());
  BOOST_CHECK(vec.size() == 100);
  BOOST_CHECK(vec[0] == 3);
  BOOST_CHECK(vec[99] == 2);
  BOOST_CHECK(vec[1] == 1);
  BOOST_CHECK(vec[50] == 1);

  // construct with init list
  const int listA[] = {7, 8, 9};
  vec = Vec(listA, listA + 3);
  BOOST_CHECK(vec.size() == 3);
  BOOST_CHECK(vec[0] == 7);
  BOOST_CHECK(vec[1] == 8);
  BOOST_CHECK(vec[2] == 9);

  BOOST_CHECK(Vec(listA, listA + 2).size() == 2);
  BOOST_CHECK(Vec(listA, listA + 2).size() == 2);
  BOOST_CHECK(Vec(7, 8).size() == 7);

  // resize
  vec.resize(5);
  BOOST_CHECK(vec.size() == 5);
  BOOST_CHECK(vec[0] == 7);
  BOOST_CHECK(vec[1] == 8);
  BOOST_CHECK(vec[2] == 9);
  vec[4] = 4;

  vec.resize(1000);
  BOOST_CHECK(vec.size() == 1000);
  BOOST_CHECK(vec[0] == 7);
  BOOST_CHECK(vec[1] == 8);
  BOOST_CHECK(vec[2] == 9);
  BOOST_CHECK(vec[4] == 4);

  vec.resize(5, 10);
  BOOST_CHECK(vec.size() == 5);
  vec.resize(1000, 10);
  BOOST_CHECK(vec.size() == 1000);
  BOOST_CHECK(vec[0] == 7);
  BOOST_CHECK(vec[1] == 8);
  BOOST_CHECK(vec[2] == 9);
  BOOST_CHECK(vec[4] == 4);
  BOOST_CHECK(vec[5] == 10);
  BOOST_CHECK(vec[999] == 10);
  BOOST_CHECK(vec.capacity() >= 1000);

  // resize that changes capacity
  Vec().swap(vec);
  vec.resize(5, 10);
  BOOST_CHECK_EQUAL(vec.size(), 5);
  vec.resize(1000, 10);
  BOOST_CHECK_EQUAL(vec.size(), 1000);

  // reserve
  vec.reserve(0);
  BOOST_CHECK(vec.capacity() >= 1000);
  vec = Vec();
  vec.reserve(1000);
  BOOST_CHECK(vec.size() == 0);
  BOOST_CHECK(vec.capacity() >= 1000);

  // shrink_to_fit
  vec.resize(1000, 3);
  vec.resize(5);
  vec.shrink_to_fit();
  BOOST_CHECK_EQUAL(vec.size(), 5);

  vec = Vec();
  vec.shrink_to_fit();
  BOOST_CHECK_EQUAL(vec.size(), 0);
  BOOST_CHECK_EQUAL(vec.capacity(), 0);

  vec.push_back(3);
  vec.push_back(7);
  vec.shrink_to_fit();
  BOOST_CHECK_EQUAL(vec.size(), 2);
  BOOST_CHECK_EQUAL(vec.capacity(), 2);
  BOOST_CHECK_EQUAL(vec[0], 3);
  BOOST_CHECK_EQUAL(vec[1], 7);

  // at
  BOOST_CHECK(vec.at(0) == 3);
  BOOST_CHECK(vec.at(1) == 7);
  try {
    vec.at(2);
    BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }
  try {
    vec.at(-1);
    BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }

  // assign
  vec.push_back(0);
  Vec temp = vec;
  vec.assign(temp.rbegin() + 1, temp.rend());
  BOOST_CHECK(vec.size() == 2);
  BOOST_CHECK(vec[0] == 7);
  BOOST_CHECK(vec[1] == 3);

  vec.assign(10, 1337);
  BOOST_CHECK(vec.size() == 10);
  BOOST_CHECK(vec[0] == 1337);
  BOOST_CHECK(vec[9] == 1337);

  vec = Vec();
  const int listB[] = {1, 2, 3, 5, 7, 9};
  vec.assign(listB, listB + 6);
  BOOST_CHECK(vec.size() == 6);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[1] == 2);
  BOOST_CHECK(vec[5] == 9);

  // insert
  vec = Vec(1000);
  vec.push_back(16);
  BOOST_CHECK(vec.insert(vec.begin() + 1001, 42) - vec.begin() == 1001);
  BOOST_CHECK(vec.insert(vec.begin() + 1001, 37) - vec.begin() == 1001);
  BOOST_CHECK(vec.size() == 1003);
  BOOST_CHECK(vec[1000] == 16);
  BOOST_CHECK(vec[1001] == 37);
  BOOST_CHECK(vec[1002] == 42);

  // GNU C++ doesn't return something yet, so skip for now
  // BOOST_CHECK(vec.insert(vec.begin()+1001, 3, 3) == vec.begin() + 1001,
  // "insert()");
  vec.insert(vec.begin() + 1001, 3, 3);
  BOOST_CHECK(vec[1000] == 16);
  BOOST_CHECK(vec[1001] == 3);
  BOOST_CHECK(vec[1002] == 3);
  BOOST_CHECK(vec[1003] == 3);
  BOOST_CHECK(vec[1004] == 37);

  vec = Vec();
  BOOST_CHECK(vec.capacity() ==
              0);  // to make sure inserts are going to increment size
  for (size_t i = 0; i != 100; ++i) {
    vec.insert(vec.begin(), 1);
    vec.insert(vec.begin() + (vec.size() - 1) / 3 + 1, 2);
    vec.insert(vec.end(), 3);
  }
  bool allCorrect[3] = {true, true, true};
  for (size_t i = 0; i != 100; ++i) {
    allCorrect[0] = allCorrect[0] && (vec[i] == 1);
    allCorrect[1] = allCorrect[1] && (vec[i + vec.size() / 3] == 2);
    allCorrect[2] = allCorrect[2] && (vec[i + vec.size() * 2 / 3] == 3);
  }
  BOOST_CHECK(allCorrect[0]);
  BOOST_CHECK(allCorrect[1]);
  BOOST_CHECK(allCorrect[2]);

  const int listC[] = {1, 2, 3, 4, 5};
  temp = Vec(listC, listC + 5);
  vec = Vec();
  for (size_t i = 0; i != 100; ++i) {
    vec.insert(vec.begin(), temp.begin(), temp.end());
    vec.insert(vec.begin() + (vec.size() - 5) / 3 + 5, temp.rbegin(),
               temp.rend());
    vec.insert(vec.end(), temp.begin(), temp.end());
  }
  BOOST_CHECK(vec.size() == 100 * 15);
  for (size_t i = 0; i != 100; ++i) {
    for (int j = 0; j != 5; ++j) {
      allCorrect[0] = allCorrect[0] && vec[i * 5 + j] == j + 1;
      allCorrect[1] = allCorrect[1] && vec[i * 5 + vec.size() / 3 + j] == 5 - j;
      allCorrect[2] =
          allCorrect[2] && vec[i * 5 + vec.size() * 2 / 3 + j] == j + 1;
    }
  }
  BOOST_CHECK(allCorrect[0]);
  BOOST_CHECK(allCorrect[1]);
  BOOST_CHECK(allCorrect[2]);

  vec = Vec(2, 1);
  vec[1] = 4;
  vec.insert(vec.begin() + 1, 3);
  vec.insert(vec.begin() + 1, 2);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[1] == 2);
  BOOST_CHECK(vec[2] == 3);
  BOOST_CHECK(vec[3] == 4);

  typename Vec::iterator insert_iter = vec.insert(vec.begin() + 2, 1000, 7);
  BOOST_CHECK_EQUAL(insert_iter - vec.begin(), 2);
  BOOST_CHECK_EQUAL(vec.size(), 1004);

  // insert_uninitialized
  vec = Vec(1000);
  vec.push_back(16);
  BOOST_CHECK_EQUAL(
      insert_uninitialized(vec, vec.begin() + 1001, 1) - vec.begin(), 1001);
  BOOST_CHECK_EQUAL(
      insert_uninitialized(vec, vec.begin() + 1001, 1) - vec.begin(), 1001);
  BOOST_CHECK_EQUAL(vec.size(), 1003);
  BOOST_CHECK_EQUAL(vec[1000], 16);

  // insert_uninitialized with (likely) capacity change
  typename Vec::iterator uinsert_iter =
      insert_uninitialized(vec, vec.begin() + 1003, 5000);
  BOOST_CHECK_EQUAL(uinsert_iter - vec.begin(), 1003);

  vec[1001] = 37;
  // GNU C++ doesn't return something yet, so skip for now
  // BOOST_CHECK(insert_uninitialized(vec, vec.begin()+1001, 3, 3) ==
  // vec.begin() + 1001, "insert()");
  insert_uninitialized(vec, vec.begin() + 1001, 3);
  BOOST_CHECK(vec[1000] == 16);
  BOOST_CHECK(vec[1004] == 37);

  // Erase
  const int listD[] = {1, 2, 3, 4};
  vec = Vec(listD, listD + 4);
  typename Vec::iterator iter = vec.erase(vec.begin() + 1);
  BOOST_CHECK(iter == vec.begin() + 1);
  BOOST_CHECK(vec.size() == 3);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[1] == 3);
  BOOST_CHECK(vec[2] == 4);
  iter = vec.erase(vec.begin() + 2);
  BOOST_CHECK(iter == vec.end());
  BOOST_CHECK(vec.size() == 2);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[1] == 3);

  iter = vec.erase(vec.begin(), vec.end());
  BOOST_CHECK(iter == vec.end());
  BOOST_CHECK(vec.empty());
  vec = Vec(listD, listD + 4);
  iter = vec.erase(vec.begin() + 1, vec.end() - 1);
  BOOST_CHECK(iter == vec.begin() + 1);
  BOOST_CHECK(vec.size() == 2);

  // Swap
  const int listE[] = {5, 6, 7};
  temp = Vec(listE, listE + 3);
  iter = temp.begin();
  vec = Vec(listD, listD + 4);
  vec.swap(temp);
  BOOST_CHECK(vec.size() == 3);
  BOOST_CHECK(temp.size() == 4);
  BOOST_CHECK(*iter == 5);
  BOOST_CHECK(vec[0] == 5);
  BOOST_CHECK(vec.back() == 7);

  // Clear
  vec = Vec();
  vec.clear();
  BOOST_CHECK(vec.empty());
  vec = Vec(listD, listD + 4);
  vec.clear();
  BOOST_CHECK(vec.empty());
  vec.push_back(5);
  BOOST_CHECK(vec.size() == 1);
  BOOST_CHECK(vec.front() == 5);

  // Emplace
  vec = Vec(2, 1);
  vec[1] = 3;
  iter = vec.insert(vec.begin() + 1, 2);
  BOOST_CHECK(iter == vec.begin() + 1);
  BOOST_CHECK(vec.size() == 3);
  BOOST_CHECK(vec[0] == 1);
  BOOST_CHECK(vec[1] == 2);
  BOOST_CHECK(vec[2] == 3);

  // operator==
  BOOST_CHECK(Vec() == Vec());
  BOOST_CHECK(!(Vec(1) == Vec()));
  vec = Vec();
  vec.push_back(7);
  vec.pop_back();
  BOOST_CHECK(vec == Vec());
  const int listF[] = {1, 2, 3, 4, 5}, listG[] = {1, 2, 3, 4, 6};
  BOOST_CHECK(Vec(listF, listF + 5) == Vec(listF, listF + 5));
  BOOST_CHECK(!(Vec(listF, listF + 5) == Vec(listG, listG + 5)));
  BOOST_CHECK(!(Vec(listF, listF + 4) == Vec(listF, listF + 5)));

  // operator!=
  BOOST_CHECK(!(Vec() != Vec()));
  BOOST_CHECK(Vec(1) != Vec());
  vec = Vec();
  vec.push_back(7);
  vec.pop_back();
  BOOST_CHECK(!(vec != Vec()));
  BOOST_CHECK(!(Vec(listF, listF + 5) != Vec(listF, listF + 5)));
  BOOST_CHECK(Vec(listF, listF + 5) != Vec(listG, listG + 5));
  BOOST_CHECK(Vec(listF, listF + 4) != Vec(listF, listF + 5));

  // operator<
  BOOST_CHECK(!(Vec() < Vec()));
  BOOST_CHECK(!(Vec(1, 1) < Vec(1, 1)));
  BOOST_CHECK(!(Vec(1, 2) < Vec(1, 1)));
  BOOST_CHECK(Vec() < Vec(1, 1));
  BOOST_CHECK(!(Vec(1, 1) < Vec()));
  BOOST_CHECK(Vec(1, 1) < Vec(1, 2));
  const int listH[] = {1, 2, 4};
  BOOST_CHECK(Vec(listF, listF + 3) < Vec(listH, listH + 3));

  // operator>
  BOOST_CHECK(!(Vec() > Vec()));
  BOOST_CHECK(!(Vec(1, 1) > Vec(1, 1)));
  BOOST_CHECK(!(Vec(1, 1) > Vec(1, 2)));
  BOOST_CHECK(Vec(listH, listH + 3) > Vec(listF, listF + 3));

  // operator<=
  BOOST_CHECK(Vec() <= Vec());
  BOOST_CHECK(Vec(1, 1) <= Vec(1, 1));
  BOOST_CHECK(!(Vec(1, 2) <= Vec(1, 1)));
  BOOST_CHECK(Vec() <= Vec(1, 1));
  BOOST_CHECK(!(Vec(1, 1) <= Vec()));
  BOOST_CHECK(Vec(1, 1) <= Vec(1, 2));
  BOOST_CHECK(Vec(listF, listF + 3) <= Vec(listH, listH + 3));

  // operator>=
  BOOST_CHECK(Vec() >= Vec());
  BOOST_CHECK(Vec(1, 1) >= Vec(1, 1));
  BOOST_CHECK(!(Vec(1, 1) >= Vec(1, 2)));
  BOOST_CHECK(Vec(1, 1) >= Vec());
  BOOST_CHECK(!(Vec() >= Vec(1, 1)));
  BOOST_CHECK(Vec(1, 2) >= Vec(1, 1));
  BOOST_CHECK(Vec(listH, listH + 3) >= Vec(listF, listF + 3));

  // ::swap
  const int listI[] = {100, 101, 102};
  temp = Vec(listI, listI + 3);
  swap(temp, vec);
  BOOST_CHECK(vec.size() == 3);
  BOOST_CHECK(vec[0] == 100);
  BOOST_CHECK(vec[1] == 101);
  BOOST_CHECK(vec[2] == 102);
}

template <typename Tp>
class FailingAllocator {
 public:
  typedef Tp value_type;
  typedef std::size_t size_type;
  typedef Tp* pointer;
  FailingAllocator() {}
  FailingAllocator(bool fail) { _failAllocation = fail; }
  void SetFailAllocation(bool fail) { _failAllocation = fail; }
  bool operator==(const FailingAllocator<Tp>&) { return true; }
  pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0) {
    if (_failAllocation) throw std::bad_alloc();
    return static_cast<pointer>(malloc(n * sizeof(Tp)));
  }
  void deallocate(pointer ptr, size_type n) { free(ptr); }

 private:
  static bool _failAllocation;
};

template <typename Tp>
bool FailingAllocator<Tp>::_failAllocation = false;

template <typename Vec>
void testBadAllocs() {
  // typedef typename std::allocator_traits<typename
  // Vec::allocator_type>::propagate_on_container_copy_assignment DoCopy;
  // typedef typename std::allocator_traits<typename
  // Vec::allocator_type>::propagate_on_container_move_assignment DoMove;
  // typedef typename std::allocator_traits<typename
  // Vec::allocator_type>::propagate_on_container_swap DoSwap; std::cout <<
  //	"Propogate on copy assignment: " << DoCopy().value << "\n"
  //	"Propogate on move assignment: " << DoMove().value << "\n"
  //	"Propogate on swap: " << DoSwap().value << "\n";

  Vec(5, 0, FailingAllocator<int>(false));
  try {
    Vec(5, 0, FailingAllocator<int>(true));
    BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }

  aocommon::UVector<int> goodVec;

  const int ListA[] = {6, 7, 8, 9};
  Vec vec(FailingAllocator<int>(false));
  vec.assign(ListA, ListA + 4);
  goodVec.assign(ListA, ListA + 4);
  vec.get_allocator().SetFailAllocation(true);
  try {
    for (size_t i = 0; i != 10000; ++i) {
      vec.push_back(i);
      goodVec.push_back(i);
    }
    BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }
  BOOST_CHECK(vec.size() == goodVec.size());
  BOOST_CHECK(vec.front() == goodVec.front());
  BOOST_CHECK(vec.back() == goodVec.back());

  vec.get_allocator().SetFailAllocation(false);
  vec = Vec();
  vec.reserve(1000);
  vec.push_back(1);
  vec.get_allocator().SetFailAllocation(true);
  try {
    vec.shrink_to_fit();
    // shrink to fit is not required to resize
    // BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }
  BOOST_CHECK(vec.size() == 1);
  BOOST_CHECK(vec[0] == 1);
  vec.get_allocator().SetFailAllocation(false);
  vec.pop_back();
  vec.shrink_to_fit();
  vec.push_back(1);

  vec.get_allocator().SetFailAllocation(true);
  try {
    vec.insert(vec.begin(), 1000, 0);
    BOOST_CHECK(false);
  } catch (...) {
    BOOST_CHECK(true);
  }
  BOOST_CHECK(vec.size() == 1);
  BOOST_CHECK(vec[0] == 1);
}

template <typename Tp, typename Propagate_on_container_copy = std::true_type,
          typename Propagate_on_container_move = std::true_type>
class IdAllocater {
 public:
  typedef IdAllocater<Tp, Propagate_on_container_copy,
                      Propagate_on_container_move>
      Myself;
  typedef std::size_t size_type;
  typedef Tp value_type;
  typedef Tp* pointer;
  typedef std::true_type propagate_on_container_swap;
  typedef Propagate_on_container_copy propagate_on_container_copy_assignment;
  typedef Propagate_on_container_move propagate_on_container_move_assignment;
  IdAllocater(size_t id) : _id(id) {}
  bool operator==(const Myself& rhs) const { return rhs._id == _id; }
  bool operator!=(const Myself& rhs) const { return rhs._id != _id; }
  Myself& operator=(const IdAllocater<Tp>& rhs) {
    _id = rhs._id;
    return *this;
  }
  Myself select_on_container_copy_construction() const {
    return IdAllocater(_id + 10);
  }
  size_t Id() const { return _id; }
  pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0) {
    char* mem = static_cast<char*>(malloc(n * sizeof(Tp) + sizeof(_id)));
    *reinterpret_cast<size_t*>(mem) = _id;
    return reinterpret_cast<pointer>(mem + sizeof(_id));
  }
  void deallocate(pointer ptr, size_type n) {
    char* mem = reinterpret_cast<char*>(ptr) - sizeof(_id);
    size_t allocId = *reinterpret_cast<size_t*>(mem);
    if (allocId == _id) {
      BOOST_CHECK(true);
    } else {
      throw std::runtime_error(
          "\nDeallocation error: Allocator id of allocation (" +
          std::to_string(allocId) +
          ") is different from allocator id of deallocation (" +
          std::to_string(_id) + ")\n");
    }
    free(mem);
  }

 private:
  size_t _id;
};

template <typename Vec>
void testAllocater() {
  Vec vecA(3, 0, IdAllocater<int>(4)), vecB(3, 0, IdAllocater<int>(5)),
      vecC(vecA);  // select_on_container_copy_construction will add 10 to id
  vecA[0] = 1;
  vecA[1] = 2;
  vecA[2] = 3;
  vecB[0] = 11;
  vecB[1] = 12;
  vecB[2] = 13;
  BOOST_CHECK(vecA.get_allocator().Id() == 4);
  BOOST_CHECK(vecB.get_allocator().Id() == 5);
  BOOST_CHECK(vecC.get_allocator().Id() == 14);
  swap(vecA, vecB);
  BOOST_CHECK(vecA.get_allocator().Id() == 5);
  BOOST_CHECK(vecB.get_allocator().Id() == 4);
  BOOST_CHECK(vecA[0] == 11);
  BOOST_CHECK(vecB[0] == 1);
  vecC = Vec(3, 0, IdAllocater<int>(6));
  vecC[0] = 21;
  vecC[1] = 22;
  vecC[2] = 23;
  BOOST_CHECK(vecC.get_allocator().Id() == 6);
  BOOST_CHECK(vecC[0] == 21);
  vecC = std::move(vecA);
  BOOST_CHECK(vecC.get_allocator().Id() == 5);
  BOOST_CHECK(vecC[0] == 11);
  Vec vecD(std::move(vecC));
  BOOST_CHECK(vecD.get_allocator().Id() == 5);
  BOOST_CHECK(vecD[0] == 11);

  class A {};
  A a1, a2;
  std::swap(a1, a2);
}

template <typename Vec>
void testExtensions() {
  // push_back(n, val)
  Vec vecA(1, 31);
  vecA.push_back(2, 1337);
  BOOST_CHECK(vecA.size() == 3);
  BOOST_CHECK(vecA[0] == 31);
  BOOST_CHECK(vecA[1] == 1337);
  BOOST_CHECK(vecA[2] == 1337);

  // push_back(initializer list)
  Vec vecB;
  const int listA[] = {1, 2, 3}, listC[] = {4, 5, 6};
  vecB.push_back(listA, listA + 3);
  BOOST_CHECK(vecB.size() == 3);
  BOOST_CHECK(vecB[0] == 1);
  BOOST_CHECK(vecB[1] == 2);
  BOOST_CHECK(vecB[2] == 3);
  vecB.push_back(listC, listC + 3);
  BOOST_CHECK(vecB.size() == 6);
  for (int i = 0; i != 6; ++i) BOOST_CHECK(vecB[i] == i + 1);

  // push_back(range)
  const int listB[] = {9, 11, 12, 13};
  vecB = Vec(listB, listB + 4);
  Vec vecC(1, 10);
  vecC.push_back(vecB.begin() + 1, vecB.end() - 1);
  BOOST_CHECK(vecC.size() == 3);
  BOOST_CHECK(vecC[0] == 10);
  BOOST_CHECK(vecC[1] == 11);
  BOOST_CHECK(vecC[2] == 12);

  // push_back_uninitialized(n)
  Vec vecD(1, 42);
  vecD.push_back_uninitialized(2);
  BOOST_CHECK(vecD.size() == 3);
  BOOST_CHECK(vecD[0] == 42);
}

template <typename Vec>
void testAllocatorPropagation() {
  // These vectors should NOT propagate on copy:
  Vec vecA(3, 0, IdAllocater<int, std::false_type, std::false_type>(4)),
      vecB(3, 0, IdAllocater<int, std::false_type, std::false_type>(5));
  Vec vecC(vecA);
  BOOST_CHECK_EQUAL(vecA.get_allocator().Id(), 4);
  BOOST_CHECK_EQUAL(vecB.get_allocator().Id(), 5);
  BOOST_CHECK_EQUAL(vecC.get_allocator().Id(), 14);
  BOOST_CHECK_EQUAL_COLLECTIONS(vecA.begin(), vecA.end(), vecC.begin(),
                                vecC.end());
  vecC = vecB;
  BOOST_CHECK_EQUAL(vecA.get_allocator().Id(), 4);
  BOOST_CHECK_EQUAL(vecB.get_allocator().Id(), 5);
  BOOST_CHECK_EQUAL(vecC.get_allocator().Id(), 14);
  BOOST_CHECK_EQUAL_COLLECTIONS(vecB.begin(), vecB.end(), vecC.begin(),
                                vecC.end());
  Vec bigvec(1000, 1, IdAllocater<int, std::false_type, std::false_type>(7));
  vecA = bigvec;
  BOOST_CHECK_EQUAL(vecA.get_allocator().Id(), 4);
  BOOST_CHECK_EQUAL_COLLECTIONS(vecA.begin(), vecA.end(), bigvec.begin(),
                                bigvec.end());

  vecA = Vec(3, 0, IdAllocater<int, std::false_type, std::false_type>(4));
  vecC = vecA;
  vecB = std::move(vecA);
  BOOST_CHECK_EQUAL(vecB.get_allocator().Id(), 5);
  BOOST_CHECK_EQUAL_COLLECTIONS(vecB.begin(), vecB.end(), vecC.begin(),
                                vecC.end());
  vecC = bigvec;
  vecB = std::move(bigvec);
  BOOST_CHECK_EQUAL(vecB.get_allocator().Id(), 5);
  BOOST_CHECK_EQUAL_COLLECTIONS(vecB.begin(), vecB.end(), vecC.begin(),
                                vecC.end());
}

BOOST_AUTO_TEST_CASE(std_vector_int) { test<std::vector<int>>(); }
BOOST_AUTO_TEST_CASE(uvector_int) { test<aocommon::UVector<int>>(); }
BOOST_AUTO_TEST_CASE(uvector_long_int) { test<aocommon::UVector<long int>>(); }
BOOST_AUTO_TEST_CASE(std_vector_allocator) {
  testBadAllocs<std::vector<int, FailingAllocator<int>>>();
  testAllocater<std::vector<int, IdAllocater<int>>>();
  testAllocatorPropagation<
      std::vector<int, IdAllocater<int, std::false_type, std::false_type>>>();
}
BOOST_AUTO_TEST_CASE(uvector_allocator) {
  testBadAllocs<aocommon::UVector<int, FailingAllocator<int>>>();
  testAllocater<aocommon::UVector<int, IdAllocater<int>>>();
  testAllocatorPropagation<aocommon::UVector<
      int, IdAllocater<int, std::false_type, std::false_type>>>();
}
BOOST_AUTO_TEST_CASE(uvector_extensions) {
  testExtensions<aocommon::UVector<int>>();
}

BOOST_AUTO_TEST_SUITE_END()
