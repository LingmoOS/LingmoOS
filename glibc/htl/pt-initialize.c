/* Initialize pthreads library.
   Copyright (C) 2000-2025 Free Software Foundation, Inc.
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

#include <assert.h>
#include <string.h>

#include <pt-internal.h>
#include <set-hooks.h>

#include <pthread.h>
#include <pthread-functions.h>

#if IS_IN (libpthread)
static const struct pthread_functions pthread_functions = {
  .ptr___pthread_exit = __pthread_exit,
  .ptr_pthread_mutex_destroy = __pthread_mutex_destroy,
  .ptr_pthread_mutex_init = __pthread_mutex_init,
  .ptr_pthread_mutex_lock = __pthread_mutex_lock,
  .ptr_pthread_mutex_trylock = __pthread_mutex_trylock,
  .ptr_pthread_mutex_unlock = __pthread_mutex_unlock,
  .ptr___pthread_setcancelstate = __pthread_setcancelstate,
  .ptr_pthread_setcanceltype = __pthread_setcanceltype,
  .ptr___pthread_get_cleanup_stack = __pthread_get_cleanup_stack,
  .ptr_pthread_once = __pthread_once,
  .ptr_pthread_rwlock_rdlock = __pthread_rwlock_rdlock,
  .ptr_pthread_rwlock_wrlock = __pthread_rwlock_wrlock,
  .ptr_pthread_rwlock_unlock = __pthread_rwlock_unlock,
  .ptr___pthread_key_create = __pthread_key_create,
  .ptr___pthread_getspecific = __pthread_getspecific,
  .ptr___pthread_setspecific = __pthread_setspecific,
  .ptr__IO_flockfile = _cthreads_flockfile,
  .ptr__IO_funlockfile = _cthreads_funlockfile,
  .ptr__IO_ftrylockfile = _cthreads_ftrylockfile,
};
#endif /* IS_IN (libpthread) */

/* Initialize the pthreads library.  */
void
___pthread_init (void)
{
#if IS_IN (libpthread)
  __libc_pthread_init (&pthread_functions);
#endif
}
