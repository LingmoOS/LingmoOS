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
#ifndef LIMONP_THREAD_HPP
#define LIMONP_THREAD_HPP

#include "Logging.hpp"
#include "NonCopyable.hpp"

namespace limonp {

class IThread: NonCopyable {
 public:
  IThread(): isStarted(false), isJoined(false) {
  }
  virtual ~IThread() {
    if(isStarted && !isJoined) {
      XCHECK(!pthread_detach(thread_));
    }
  };

  virtual void Run() = 0;
  void Start() {
    XCHECK(!isStarted);
    XCHECK(!pthread_create(&thread_, NULL, Worker, this));
    isStarted = true;
  }
  void Join() {
    XCHECK(!isJoined);
    XCHECK(!pthread_join(thread_, NULL));
    isJoined = true;
  }
 private:
  static void * Worker(void * data) {
    IThread * ptr = (IThread* ) data;
    ptr->Run();
    return NULL;
  }

  pthread_t thread_;
  bool isStarted;
  bool isJoined;
}; // class IThread

} // namespace limonp

#endif // LIMONP_THREAD_HPP
