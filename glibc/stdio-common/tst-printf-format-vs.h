/* Test feature wrapper for formatted 'vsprintf' output.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <support/next_to_fault.h>

#define SPRINTF_BUFFER_SIZE 65536

static struct support_next_to_fault ntf;

#define PREPARE printf_under_test_init
static void
printf_under_test_init (int argc, char **argv)
{
  ntf = support_next_to_fault_allocate (SPRINTF_BUFFER_SIZE);
}

static void __attribute__ ((destructor))
printf_under_test_fini (void)
{
  support_next_to_fault_free (&ntf);
}

static int
printf_under_test (const char *restrict fmt, ...)
{
  char *str = ntf.buffer;
  va_list ap;
  int result;

  va_start (ap, fmt);
  result = vsprintf (str, fmt, ap);
  va_end (ap);
  if (result < 0)
    {
      perror ("vsprintf");
      goto out;
    }
  if (fwrite (str, sizeof (*str), result, stdout) != result)
    {
      perror ("fwrite");
      result = -1;
    }
out:
  return result;
}
