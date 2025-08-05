/* Auxiliary vector processing.  Linux/PPC version.
   Copyright (C) 2020-2025 Free Software Foundation, Inc.
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

#ifndef __powerpc64__
/* We have to detect 8xx processors, which have buggy dcbz implementations
 * that cannot report page faults correctly. That requires reading SPR,
 * which is a privileged operation.  Fortunately 2.2.18 and later emulates
 * PowerPC mfspr reads from the PVR register.  */
#define DL_PLATFORM_AUXV \
  {									\
    unsigned pvr = 0;							\
    asm ("mfspr %0, 287" : "=r" (pvr));					\
    if ((pvr & 0xffff0000) == 0x00500000)				\
      GLRO(dl_cache_line_size) = 0;					\
    else								\
      GLRO(dl_cache_line_size) = auxv_values[AT_DCACHEBSIZE];		\
  }
#else
#define DL_PLATFORM_AUXV \
  GLRO(dl_cache_line_size) = auxv_values[AT_DCACHEBSIZE];
#endif
