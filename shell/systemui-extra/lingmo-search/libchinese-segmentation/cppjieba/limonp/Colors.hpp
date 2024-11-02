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
#ifndef LIMONP_COLOR_PRINT_HPP
#define LIMONP_COLOR_PRINT_HPP

#include <string>
#include <stdarg.h>

namespace limonp {

using std::string;

enum Color {
  BLACK = 30,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  PURPLE
}; // enum Color

static void ColorPrintln(enum Color color, const char * fmt, ...) {
  va_list ap;
  printf("\033[0;%dm", color);
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\033[0m\n"); // if not \n , in some situation , the next lines will be set the same color unexpectedly
}

} // namespace limonp

#endif // LIMONP_COLOR_PRINT_HPP
