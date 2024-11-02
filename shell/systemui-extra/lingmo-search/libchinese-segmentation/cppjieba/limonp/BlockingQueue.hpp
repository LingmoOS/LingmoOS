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
#ifndef LIMONP_BLOCKINGQUEUE_HPP
#define LIMONP_BLOCKINGQUEUE_HPP

#include <queue>
#include "Condition.hpp"

namespace limonp {
template<class T>
class BlockingQueue: NonCopyable {
 public:
  BlockingQueue()
    : mutex_(), notEmpty_(mutex_), queue_() {
  }

  void Push(const T& x) {
    MutexLockGuard lock(mutex_);
    queue_.push(x);
    notEmpty_.Notify(); // Wait morphing saves us
  }

  T Pop() {
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty()) {
      notEmpty_.Wait();
    }
    assert(!queue_.empty());
    T front(queue_.front());
    queue_.pop();
    return front;
  }

  size_t Size() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }
  bool Empty() const {
    return Size() == 0;
  }

 private:
  mutable MutexLock mutex_;
  Condition         notEmpty_;
  std::queue<T>     queue_;
}; // class BlockingQueue

} // namespace limonp

#endif // LIMONP_BLOCKINGQUEUE_HPP
