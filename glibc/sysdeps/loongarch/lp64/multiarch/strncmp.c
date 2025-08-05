/* Multiple versions of strncmp.
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2023-2025 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

/* Define multiple versions only for the definition in libc.  */
#if IS_IN (libc)
# define strncmp __redirect_strncmp
# include <string.h>
# undef strncmp

# define SYMBOL_NAME strncmp
# include "ifunc-strncmp.h"

libc_ifunc_redirected (__redirect_strncmp, strncmp, IFUNC_SELECTOR ());

# ifdef SHARED
__hidden_ver1 (strncmp, __GI_strncmp, __redirect_strncmp)
  __attribute__ ((visibility ("hidden"))) __attribute_copy__ (strncmp);
# endif
#endif
