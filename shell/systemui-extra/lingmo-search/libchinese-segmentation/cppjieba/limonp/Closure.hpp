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
#ifndef LIMONP_CLOSURE_HPP
#define LIMONP_CLOSURE_HPP

namespace limonp {

class ClosureInterface {
 public:
  virtual ~ClosureInterface() {
  }
  virtual void Run() = 0;
};

template <class Funct>
class Closure0: public ClosureInterface {
 public:
  Closure0(Funct fun) {
    fun_ = fun;
  }
  virtual ~Closure0() {
  }
  virtual void Run() {
    (*fun_)();
  }
 private:
  Funct fun_;
}; 

template <class Funct, class Arg1>
class Closure1: public ClosureInterface {
 public:
  Closure1(Funct fun, Arg1 arg1) {
    fun_ = fun;
    arg1_ = arg1;
  }
  virtual ~Closure1() {
  }
  virtual void Run() {
    (*fun_)(arg1_);
  }
 private:
  Funct fun_;
  Arg1 arg1_;
}; 

template <class Funct, class Arg1, class Arg2>
class Closure2: public ClosureInterface {
 public:
  Closure2(Funct fun, Arg1 arg1, Arg2 arg2) {
    fun_ = fun;
    arg1_ = arg1;
    arg2_ = arg2;
  }
  virtual ~Closure2() {
  }
  virtual void Run() {
    (*fun_)(arg1_, arg2_);
  }
 private:
  Funct fun_;
  Arg1 arg1_;
  Arg2 arg2_;
}; 

template <class Funct, class Arg1, class Arg2, class Arg3>
class Closure3: public ClosureInterface {
 public:
  Closure3(Funct fun, Arg1 arg1, Arg2 arg2, Arg3 arg3) {
    fun_ = fun;
    arg1_ = arg1;
    arg2_ = arg2;
    arg3_ = arg3;
  }
  virtual ~Closure3() {
  }
  virtual void Run() {
    (*fun_)(arg1_, arg2_, arg3_);
  }
 private:
  Funct fun_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
}; 

template <class Obj, class Funct> 
class ObjClosure0: public ClosureInterface {
 public:
  ObjClosure0(Obj* p, Funct fun) {
   p_ = p;
   fun_ = fun;
  }
  virtual ~ObjClosure0() {
  }
  virtual void Run() {
    (p_->*fun_)();
  }
 private:
  Obj* p_;
  Funct fun_;
}; 

template <class Obj, class Funct, class Arg1> 
class ObjClosure1: public ClosureInterface {
 public:
  ObjClosure1(Obj* p, Funct fun, Arg1 arg1) {
   p_ = p;
   fun_ = fun;
   arg1_ = arg1;
  }
  virtual ~ObjClosure1() {
  }
  virtual void Run() {
    (p_->*fun_)(arg1_);
  }
 private:
  Obj* p_;
  Funct fun_;
  Arg1 arg1_;
}; 

template <class Obj, class Funct, class Arg1, class Arg2> 
class ObjClosure2: public ClosureInterface {
 public:
  ObjClosure2(Obj* p, Funct fun, Arg1 arg1, Arg2 arg2) {
   p_ = p;
   fun_ = fun;
   arg1_ = arg1;
   arg2_ = arg2;
  }
  virtual ~ObjClosure2() {
  }
  virtual void Run() {
    (p_->*fun_)(arg1_, arg2_);
  }
 private:
  Obj* p_;
  Funct fun_;
  Arg1 arg1_;
  Arg2 arg2_;
}; 
template <class Obj, class Funct, class Arg1, class Arg2, class Arg3> 
class ObjClosure3: public ClosureInterface {
 public:
  ObjClosure3(Obj* p, Funct fun, Arg1 arg1, Arg2 arg2, Arg3 arg3) {
   p_ = p;
   fun_ = fun;
   arg1_ = arg1;
   arg2_ = arg2;
   arg3_ = arg3;
  }
  virtual ~ObjClosure3() {
  }
  virtual void Run() {
    (p_->*fun_)(arg1_, arg2_, arg3_);
  }
 private:
  Obj* p_;
  Funct fun_;
  Arg1 arg1_;
  Arg2 arg2_;
  Arg3 arg3_;
}; 

template<class R>
ClosureInterface* NewClosure(R (*fun)()) {
  return new Closure0<R (*)()>(fun);
}

template<class R, class Arg1>
ClosureInterface* NewClosure(R (*fun)(Arg1), Arg1 arg1) {
  return new Closure1<R (*)(Arg1), Arg1>(fun, arg1);
}

template<class R, class Arg1, class Arg2>
ClosureInterface* NewClosure(R (*fun)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new Closure2<R (*)(Arg1, Arg2), Arg1, Arg2>(fun, arg1, arg2);
}

template<class R, class Arg1, class Arg2, class Arg3>
ClosureInterface* NewClosure(R (*fun)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new Closure3<R (*)(Arg1, Arg2, Arg3), Arg1, Arg2, Arg3>(fun, arg1, arg2, arg3);
}

template<class R, class Obj>
ClosureInterface* NewClosure(Obj* obj, R (Obj::* fun)()) {
  return new ObjClosure0<Obj, R (Obj::* )()>(obj, fun);
}

template<class R, class Obj, class Arg1>
ClosureInterface* NewClosure(Obj* obj, R (Obj::* fun)(Arg1), Arg1 arg1) {
  return new ObjClosure1<Obj, R (Obj::* )(Arg1), Arg1>(obj, fun, arg1);
}

template<class R, class Obj, class Arg1, class Arg2>
ClosureInterface* NewClosure(Obj* obj, R (Obj::* fun)(Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
  return new ObjClosure2<Obj, R (Obj::*)(Arg1, Arg2), Arg1, Arg2>(obj, fun, arg1, arg2);
}

template<class R, class Obj, class Arg1, class Arg2, class Arg3>
ClosureInterface* NewClosure(Obj* obj, R (Obj::* fun)(Arg1, Arg2, Arg3), Arg1 arg1, Arg2 arg2, Arg3 arg3) {
  return new ObjClosure3<Obj, R (Obj::*)(Arg1, Arg2, Arg3), Arg1, Arg2, Arg3>(obj, fun, arg1, arg2, arg3);
}

} // namespace limonp

#endif // LIMONP_CLOSURE_HPP
