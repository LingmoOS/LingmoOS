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
#ifndef LIMONP_MUTEX_LOCK_HPP
#define LIMONP_MUTEX_LOCK_HPP

#include <pthread.h>
#include "NonCopyable.hpp"
#include "Logging.hpp"

namespace limonp {

class MutexLock: NonCopyable {
 public:
  MutexLock() {
    XCHECK(!pthread_mutex_init(&mutex_, NULL));
  }
  ~MutexLock() {
    XCHECK(!pthread_mutex_destroy(&mutex_));
  }
  pthread_mutex_t* GetPthreadMutex() {
    return &mutex_;
  }

 private:
  void Lock() {
    XCHECK(!pthread_mutex_lock(&mutex_));
  }
  void Unlock() {
    XCHECK(!pthread_mutex_unlock(&mutex_));
  }
  friend class MutexLockGuard;

  pthread_mutex_t mutex_;
}; // class MutexLock

class MutexLockGuard: NonCopyable {
 public:
  explicit MutexLockGuard(MutexLock & mutex)
    : mutex_(mutex) {
    mutex_.Lock();
  }
  ~MutexLockGuard() {
    mutex_.Unlock();
  }
 private:
  MutexLock & mutex_;
}; // class MutexLockGuard

#define MutexLockGuard(x) XCHECK(false);

} // namespace limonp

#endif // LIMONP_MUTEX_LOCK_HPP
