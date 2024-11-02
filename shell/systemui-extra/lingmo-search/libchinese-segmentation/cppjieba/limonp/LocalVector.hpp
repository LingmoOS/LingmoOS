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
#ifndef LIMONP_LOCAL_VECTOR_HPP
#define LIMONP_LOCAL_VECTOR_HPP

#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

namespace limonp {
using namespace std;
/*
 * LocalVector<T> : T must be primitive type (char , int, size_t), if T is struct or class, LocalVector<T> may be dangerous..
 * LocalVector<T> is simple and not well-tested.
 */
const size_t LOCAL_VECTOR_BUFFER_SIZE = 16;
template <class T>
class LocalVector {
 public:
  typedef const T* const_iterator ;
  typedef T value_type;
  typedef size_t size_type;
 private:
  T buffer_[LOCAL_VECTOR_BUFFER_SIZE];
  T * ptr_;
  size_t size_;
  size_t capacity_;
 public:
  LocalVector() {
    init_();
  };
  LocalVector(const LocalVector<T>& vec) {
    init_();
    *this = vec;
  }
  LocalVector(const_iterator  begin, const_iterator end) { // TODO: make it faster
    init_();
    while(begin != end) {
      push_back(*begin++);
    }
  }
  LocalVector(size_t size, const T& t) { // TODO: make it faster
    init_();
    while(size--) {
      push_back(t);
    }
  }
  ~LocalVector() {
    if(ptr_ != buffer_) {
      free(ptr_);
    }
  };
 public:
  LocalVector<T>& operator = (const LocalVector<T>& vec) {
      if(this == &vec){
          return *this;
      }
    clear();
    size_ = vec.size();
    capacity_ = vec.capacity();
    if(vec.buffer_ == vec.ptr_) {
      memcpy(buffer_, vec.buffer_, sizeof(T) * size_);
      ptr_ = buffer_;
    } else {
      ptr_ = (T*) malloc(vec.capacity() * sizeof(T));
      assert(ptr_);
      memcpy(ptr_, vec.ptr_, vec.size() * sizeof(T));
    }
    return *this;
  }
 private:
  void init_() {
    ptr_ = buffer_;
    size_ = 0;
    capacity_ = LOCAL_VECTOR_BUFFER_SIZE;
  }
 public:
  T& operator [] (size_t i) {
    return ptr_[i];
  }
  const T& operator [] (size_t i) const {
    return ptr_[i];
  }
  void push_back(const T& t) {
    if(size_ == capacity_) {
      assert(capacity_);
      reserve(capacity_ * 2);
    }
    ptr_[size_ ++ ] = t;
  }
  void reserve(size_t size) {
    if(size <= capacity_) {
      return;
    }
    T * next =  (T*)malloc(sizeof(T) * size);
    assert(next);
    T * old = ptr_;
    ptr_ = next;
    memcpy(ptr_, old, sizeof(T) * capacity_);
    capacity_ = size;
    if(old != buffer_) {
      free(old);
    }
  }
  bool empty() const {
    return 0 == size();
  }
  size_t size() const {
    return size_;
  }
  size_t capacity() const {
    return capacity_;
  }
  const_iterator begin() const {
    return ptr_;
  }
  const_iterator end() const {
    return ptr_ + size_;
  }
  void clear() {
    if(ptr_ != buffer_) {
      free(ptr_);
    }
    init_();
  }
};

template <class T>
ostream & operator << (ostream& os, const LocalVector<T>& vec) {
  if(vec.empty()) {
    return os << "[]";
  }
  os<<"[\""<<vec[0];
  for(size_t i = 1; i < vec.size(); i++) {
    os<<"\", \""<<vec[i];
  }
  os<<"\"]";
  return os;
}

}

#endif
