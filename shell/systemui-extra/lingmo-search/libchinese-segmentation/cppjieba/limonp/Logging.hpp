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
#ifndef LIMONP_LOGGING_HPP
#define LIMONP_LOGGING_HPP

#include <sstream>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>

#ifdef XLOG
#error "XLOG has been defined already"
#endif // XLOG
#ifdef XCHECK
#error "XCHECK has been defined already"
#endif // XCHECK

#define XLOG(level) limonp::Logger(limonp::LL_##level, __FILE__, __LINE__).Stream()
#define XCHECK(exp) if(!(exp)) XLOG(FATAL) << "exp: ["#exp << "] false. "

namespace limonp {

enum {
  LL_DEBUG = 0,
  LL_INFO = 1,
  LL_WARNING = 2,
  LL_ERROR = 3,
  LL_FATAL = 4,
}; // enum

static const char * LOG_LEVEL_ARRAY[] = {"DEBUG","INFO","WARN","ERROR","FATAL"};

class Logger {
 public:
  Logger(size_t level, const char* filename, int lineno)
   : level_(level) {
#ifdef LOGGING_LEVEL
     if (level_ < LOGGING_LEVEL) {
       return;
     }
#endif
    assert(level_ <= sizeof(LOG_LEVEL_ARRAY)/sizeof(*LOG_LEVEL_ARRAY));
    char buf[32];
    time_t now;
    time(&now);
    struct tm result;
    localtime_r(&now, &result);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &result);
    stream_ << buf
      << " " << filename
      << ":" << lineno
      << " " << LOG_LEVEL_ARRAY[level_]
      << " ";
  }
  ~Logger() {
#ifdef LOGGING_LEVEL
     if (level_ < LOGGING_LEVEL) {
       return;
     }
#endif
    std::cerr << stream_.str() << std::endl;
    if (level_ == LL_FATAL) {
      abort();
    }
  }

  std::ostream& Stream() {
    return stream_;
  }

 private:
  std::ostringstream stream_;
  size_t level_;
}; // class Logger

} // namespace limonp

#endif // LIMONP_LOGGING_HPP
