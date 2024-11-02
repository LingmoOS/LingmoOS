/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2013 Yanyi Wu
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
*/
#ifndef LIMONP_BOUNDED_QUEUE_HPP
#define LIMONP_BOUNDED_QUEUE_HPP

#include <vector>
#include <fstream>
#include <cassert>

namespace limonp {
using namespace std;
template<class T>
class BoundedQueue {
 public:
  explicit BoundedQueue(size_t capacity): capacity_(capacity), circular_buffer_(capacity) {
    head_ = 0;
    tail_ = 0;
    size_ = 0;
    assert(capacity_);
  }
  ~BoundedQueue() {
  }

  void Clear() {
    head_ = 0;
    tail_ = 0;
    size_ = 0;
  }
  bool Empty() const {
    return !size_;
  }
  bool Full() const {
    return capacity_ == size_;
  }
  size_t Size() const {
    return size_;
  }
  size_t Capacity() const {
    return capacity_;
  }

  void Push(const T& t) {
    assert(!Full());
    circular_buffer_[tail_] = t;
    tail_ = (tail_ + 1) % capacity_;
    size_ ++;
  }

  T Pop() {
    assert(!Empty());
    size_t oldPos = head_;
    head_ = (head_ + 1) % capacity_;
    size_ --;
    return circular_buffer_[oldPos];
  }

 private:
  size_t head_;
  size_t tail_;
  size_t size_;
  const size_t capacity_;
  vector<T> circular_buffer_;

}; // class BoundedQueue
} // namespace limonp

#endif
