/*
 * libcompat - system compatibility library
 * compat-zlib.h - zlib compatibility declarations
 *
 * Copyright © 2021 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMPAT_ZLIB_H
#define COMPAT_ZLIB_H

#if USE_LIBZ_IMPL == USE_LIBZ_IMPL_ZLIB_NG
#include <zlib-ng.h>
#elif USE_LIBZ_IMPL == USE_LIBZ_IMPL_ZLIB
#include <zlib.h>
#endif

#if USE_LIBZ_IMPL == USE_LIBZ_IMPL_ZLIB_NG
/* Compatibility symbols for zlib-ng. */
#define gzdopen		zng_gzdopen
#define gzopen		zng_gzopen
#define gzread		zng_gzread
#define gzwrite		zng_gzwrite
#define gzerror		zng_gzerror
#define gzclose		zng_gzclose
#define zError		zng_zError
#endif

#endif /* COMPAT_ZLIB_H */
