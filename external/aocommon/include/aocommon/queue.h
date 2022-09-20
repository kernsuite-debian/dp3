#ifndef AO_QUEUE_H
#define AO_QUEUE_H

#include <boost/circular_buffer.hpp>

namespace aocommon {

/**
 * Defines a simple FIFO queue. Properties:
 * - Expands automatically when it is full and a new element is added.
 * - Has similar complexity and performance as std::vector, since it
 *   is implemented using a circular queue.
 * @tparam T type of the elements of the queue.
 */
template <typename T>
class Queue {
 public:
  using iterator = typename boost::circular_buffer<T>::iterator;
  using const_iterator = typename boost::circular_buffer<T>::const_iterator;

  /**
   * Constructor
   * Allocates room for a single element so ResizeIfNeeded can always simply
   * multiply the size by 2.
   */
  Queue() : buffer_(1) {}

  /**
   * Adds a value to the end of the queue.
   * @param value An lvalue or rvalue reference to a value.
   * @return A reference to the newly added element.
   * @{
   */
  T& PushBack(T&& value) {
    ResizeIfNeeded();
    buffer_.push_back(std::move(value));
    return buffer_.back();
  }
  T& PushBack(const T& value) {
    ResizeIfNeeded();
    buffer_.push_back(value);
    return buffer_.back();
  }
  /** @} */

  /**
   * Removes the least recently added value from the queue.
   */
  void PopFront() { buffer_.pop_front(); }

  /**
   * Removes all elements from the queue.
   * Does not change the capacity of the queue.
   */
  void Clear() { buffer_.clear(); }

  /**
   * @return An iterator for the first element of the queue. The iterator
   * remains valid as long as the queue is not modified.
   * @{
   */
  iterator begin() { return buffer_.begin(); }
  const_iterator begin() const { return buffer_.begin(); }
  /** @} */

  /**
   * @return An iterator for the end of the queue. The iterator remains valid as
   * long as the queue is not modified.
   * @{
   */
  iterator end() { return buffer_.end(); }
  const_iterator end() const { return buffer_.end(); }
  /** @} */

  /**
   * Get an element at an index position.
   * @param i index. The least recently added value has index 0.
   * @returns A reference to the element at the given index.
   * @{
   */
  T& operator[](size_t i) { return buffer_[i]; };
  const T& operator[](size_t i) const { return buffer_[i]; };
  /** @} */

  /**
   * @return True if the queue is empty, false if it has one or more elements.
   */
  bool Empty() const { return buffer_.empty(); }

  /**
   * @return The number of elements in the queue.
   */
  std::size_t Size() const { return buffer_.size(); }

  /**
   * @return The current capacity of the queue, in number of elements.
   */
  std::size_t Capacity() const { return buffer_.capacity(); }

 private:
  inline void ResizeIfNeeded() {
    if (buffer_.full()) buffer_.set_capacity(buffer_.capacity() * 2);
  }

  boost::circular_buffer<T> buffer_;
};

}  // namespace aocommon

#endif