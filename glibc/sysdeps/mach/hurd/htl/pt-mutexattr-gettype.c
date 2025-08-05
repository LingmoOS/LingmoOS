/* pthread_mutexattr_gettype.  Hurd version.
   Copyright (C) 2016-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#include <pthreadP.h>
#include <stdlib.h>
#include <assert.h>
#include <pt-internal.h>
#include "pt-mutex.h"
#include <hurdlock.h>
#include <shlib-compat.h>

int
__pthread_mutexattr_gettype (const pthread_mutexattr_t *attrp, int *outp)
{
  *outp = attrp->__mutex_type;
  return 0;
}
libc_hidden_def (__pthread_mutexattr_gettype)
versioned_symbol (libc, __pthread_mutexattr_gettype, pthread_mutexattr_gettype, GLIBC_2_41);

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_12, GLIBC_2_41)
compat_symbol (libpthread, __pthread_mutexattr_gettype,pthread_mutexattr_gettype, GLIBC_2_12);
#endif
