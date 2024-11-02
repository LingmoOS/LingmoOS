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

/************************************
 * file enc : ascii
 * author   : wuyanyi09@gmail.com
 ************************************/

#ifndef LIMONP_ARGV_FUNCTS_H
#define LIMONP_ARGV_FUNCTS_H

#include <set>
#include <sstream>
#include "StringUtil.hpp"

namespace limonp {

using namespace std;

class ArgvContext {
 public :
  ArgvContext(int argc, const char* const * argv) {
    for(int i = 0; i < argc; i++) {
      if(StartsWith(argv[i], "-")) {
        if(i + 1 < argc && !StartsWith(argv[i + 1], "-")) {
          mpss_[argv[i]] = argv[i+1];
          i++;
        } else {
          sset_.insert(argv[i]);
        }
      } else {
        args_.push_back(argv[i]);
      }
    }
  }
  ~ArgvContext() {
  }

  friend ostream& operator << (ostream& os, const ArgvContext& args);
  string operator [](size_t i) const {
    if(i < args_.size()) {
      return args_[i];
    }
    return "";
  }
  string operator [](const string& key) const {
    map<string, string>::const_iterator it = mpss_.find(key);
    if(it != mpss_.end()) {
      return it->second;
    }
    return "";
  }

  bool HasKey(const string& key) const {
    if(mpss_.find(key) != mpss_.end() || sset_.find(key) != sset_.end()) {
      return true;
    }
    return false;
  }

 private:
  vector<string> args_;
  map<string, string> mpss_;
  set<string> sset_;
}; // class ArgvContext

inline ostream& operator << (ostream& os, const ArgvContext& args) {
  return os<<args.args_<<args.mpss_<<args.sset_;
}

} // namespace limonp

#endif
