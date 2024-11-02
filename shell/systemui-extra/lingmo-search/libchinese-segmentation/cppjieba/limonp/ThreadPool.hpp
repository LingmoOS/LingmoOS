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
#ifndef LIMONP_THREAD_POOL_HPP
#define LIMONP_THREAD_POOL_HPP

#include "Thread.hpp"
#include "BlockingQueue.hpp"
#include "BoundedBlockingQueue.hpp"
#include "Closure.hpp"

namespace limonp {

using namespace std;

//class ThreadPool;
class ThreadPool: NonCopyable {
 public:
  class Worker: public IThread {
   public:
    Worker(ThreadPool* pool): ptThreadPool_(pool) {
      assert(ptThreadPool_);
    }
    virtual ~Worker() {
    }

    virtual void Run() {
      while (true) {
        ClosureInterface* closure = ptThreadPool_->queue_.Pop();
        if (closure == NULL) {
          break;
        }
        try {
          closure->Run();
        } catch(std::exception& e) {
          XLOG(ERROR) << e.what();
        } catch(...) {
          XLOG(ERROR) << " unknown exception.";
        }
        delete closure;
      }
    }
   private:
    ThreadPool * ptThreadPool_;
  }; // class Worker

  ThreadPool(size_t thread_num)
    : threads_(thread_num), 
      queue_(thread_num) {
    assert(thread_num);
    for(size_t i = 0; i < threads_.size(); i ++) {
      threads_[i] = new Worker(this);
    }
  }
  ~ThreadPool() {
    Stop();
  }

  void Start() {
    for(size_t i = 0; i < threads_.size(); i++) {
      threads_[i]->Start();
    }
  }
  void Stop() {
    for(size_t i = 0; i < threads_.size(); i ++) {
      queue_.Push(NULL);
    }
    for(size_t i = 0; i < threads_.size(); i ++) {
      threads_[i]->Join();
      delete threads_[i];
    }
    threads_.clear();
  }

  void Add(ClosureInterface* task) {
    assert(task);
    queue_.Push(task);
  }

 private:
  friend class Worker;

  vector<IThread*> threads_;
  BoundedBlockingQueue<ClosureInterface*> queue_;
}; // class ThreadPool

} // namespace limonp

#endif // LIMONP_THREAD_POOL_HPP
