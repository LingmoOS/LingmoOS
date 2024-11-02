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
#ifndef LIMONP_BOUNDED_BLOCKING_QUEUE_HPP
#define LIMONP_BOUNDED_BLOCKING_QUEUE_HPP

#include "BoundedQueue.hpp"

namespace limonp {

template<typename T>
class BoundedBlockingQueue : NonCopyable {
 public:
  explicit BoundedBlockingQueue(size_t maxSize)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      queue_(maxSize) {
  }

  void Push(const T& x) {
    MutexLockGuard lock(mutex_);
    while (queue_.Full()) {
      notFull_.Wait();
    }
    assert(!queue_.Full());
    queue_.Push(x);
    notEmpty_.Notify();
  }

  T Pop() {
    MutexLockGuard lock(mutex_);
    while (queue_.Empty()) {
      notEmpty_.Wait();
    }
    assert(!queue_.Empty());
    T res = queue_.Pop();
    notFull_.Notify();
    return res;
  }

  bool Empty() const {
    MutexLockGuard lock(mutex_);
    return queue_.Empty();
  }

  bool Full() const {
    MutexLockGuard lock(mutex_);
    return queue_.Full();
  }

  size_t size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const {
    return queue_.capacity();
  }

 private:
  mutable MutexLock          mutex_;
  Condition                  notEmpty_;
  Condition                  notFull_;
  BoundedQueue<T>  queue_;
}; // class BoundedBlockingQueue

} // namespace limonp

#endif // LIMONP_BOUNDED_BLOCKING_QUEUE_HPP
