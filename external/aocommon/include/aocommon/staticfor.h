#ifndef STATIC_FOR_H
#define STATIC_FOR_H

#include "barrier.h"

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include <sched.h>

namespace aocommon {

/**
 * The StaticFor class implements a parallel for loop that
 * is statically distributed over all threads. It is suitable
 * for large loops that approximately take an equal amount of
 * time per iteration. If one thread finishes its chunk
 * earlier, it won't be rescheduled to more.
 *
 * The advantage of this is that it doesn't require communication
 * between iterations, and thus the loop is faster, as long as
 * iterations take similar time.
 *
 * An example to count to 1000 with 4 threads:
 * StaticFor<size_t> loop(4);
 * loop.Run(0, 1000, [&](size_t a, size_t b) {
 *   for(size_t i=a; i!=b; ++i) {
 *     std::cout << i << '\n';
 *   }
 * }
 *
 * It is also possible to acquire the thread index, by providing
 * a function with 3 parameters:
 * StaticFor<size_t> loop(4);
 * loop.Run(0, 1000, [&](size_t a, size_t b, size_t thread) {
 *   for(size_t i=a; i!=b; ++i) {
 *     std::cout << i << " from thread " << thread << '\n';
 *   }
 * }
 */
template <typename Iter>
class StaticFor {
 public:
  /**
   * Construct class with given nr of threads.
   */
  StaticFor(size_t nThreads)
      : _nThreads(nThreads),
        _barrier(nThreads, [&]() { _hasTasks = false; }),
        _stop(false),
        _hasTasks(false) {}

  /**
   * Destructor will wait for all threads to be finished.
   */
  ~StaticFor() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (!_threads.empty()) {
      _stop = true;
      _hasTasks = true;
      _conditionChanged.notify_all();
      lock.unlock();
      for (std::thread &thr : _threads) thr.join();
    }
  }

  /**
   * Iteratively call a function in parallel.
   *
   * The provided function is expected to accept two size_t parameters, the
   * start and end indices of this thread, e.g.: void loopFunction(size_t
   * chunkStart, size_t chunkEnd);
   */
  void Run(Iter start, Iter end, std::function<void(Iter, Iter)> function) {
    _loopFunction = std::move(function);
    run(start, end);
    _loopFunction = nullptr;
  }

  /**
   * Iteratively call a function in parallel with thread id.
   *
   * The provided function is expected to accept three parameters, the start
   * and end indices of this thread, e.g.:
   *   void loopFunction(size_t chunkStart, size_t chunkEnd, size_t threadId);
   */
  void Run(Iter start, Iter end,
           std::function<void(Iter, Iter, size_t)> function) {
    _loopFunctionEx = std::move(function);
    run(start, end);
    _loopFunctionEx = nullptr;
  }

  /**
   * Number of threads
   */
  size_t NThreads() const { return _nThreads; }

  /**
   * This method is only allowed to be called before Run() is
   * called.
   */
  void SetNThreads(size_t nThreads) {
    if (_threads.empty()) {
      _nThreads = nThreads;
      _barrier = Barrier(nThreads, [&]() { _hasTasks = false; });
    } else {
      throw std::runtime_error("Can not set NThreads after calling Run()");
    }
  }

 private:
  StaticFor(const StaticFor &) = delete;

  void run(Iter start, Iter end) {
    if (end == start + 1 || _nThreads <= 1) {
      callFunction(start, end, 0);
    } else {
      if (_threads.empty()) startThreads();
      std::unique_lock<std::mutex> lock(_mutex);
      _iterStart = start;
      _iterEnd = end;
      _currentChunk = 0;
      _nChunks = std::min(_nThreads, end - start);
      _hasTasks = true;
      _conditionChanged.notify_all();
      lock.unlock();

      // To avoid one extra thread-spawn, this thread also performs
      // the iterations for one chunk: (with thread id 0)
      loop(0);

      _barrier.wait();
    }
  }

  void callFunction(Iter start, Iter end, size_t threadId) const {
    if (_loopFunction)
      _loopFunction(start, end);
    else
      _loopFunctionEx(start, end, threadId);
  }

  void loop(size_t threadId) {
    if (threadId < _nChunks) {
      Iter chunkStart =
          _iterStart + (_iterEnd - _iterStart) * threadId / _nChunks;
      Iter chunkEnd =
          _iterStart + (_iterEnd - _iterStart) * (threadId + 1) / _nChunks;

      callFunction(chunkStart, chunkEnd, threadId);
    }
  }

  void threadLoop(size_t threadId) {
    waitForTasks();
    while (!_stop) {
      loop(threadId);
      _barrier.wait();
      waitForTasks();
    }
  }

  void waitForTasks() {
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_hasTasks) _conditionChanged.wait(lock);
  }

  void startThreads() {
    if (_nThreads > 1) {
      _threads.reserve(_nThreads - 1);
      for (unsigned t = 1; t != _nThreads; ++t)
        _threads.emplace_back(&StaticFor::threadLoop, this, t);
    }
  }

  size_t _currentChunk, _nChunks;
  Iter _iterStart, _iterEnd;
  std::mutex _mutex;
  size_t _nThreads;
  Barrier _barrier;
  std::atomic<bool> _stop;
  bool _hasTasks;
  std::condition_variable _conditionChanged;
  std::vector<std::thread> _threads;
  std::function<void(Iter, Iter)> _loopFunction;
  std::function<void(Iter, Iter, size_t)> _loopFunctionEx;
};

}  // namespace aocommon

#endif
