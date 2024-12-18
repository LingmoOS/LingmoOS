// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_SYSTEM_H_
#define CORE_FXCRT_FX_SYSTEM_H_

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stoceanf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "build/build_config.h"

#if defined(_MSC_VER) && _MSC_VER < 1900
#error Sorry, VC++ 2015 or later is required to compile PDFium.
#endif  // defined(_MSC_VER) && _MSC_VER < 1900

#if defined(__wasm__) && defined(PDF_ENABLE_V8)
#error Cannot compile v8 with wasm.
#endif  // PDF_ENABLE_V8

#if defined(OS_WIN)
#include <windows.h>
#include <sal.h>
#endif  // defined(OS_WIN)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa, fb) IsFloatZero((fa) - (fb))

// PDFium file sizes match the platform. The value must be signed to support -1
// error returns.
#if defined(OS_WIN)
#define FX_FILESIZE int64_t
#else
#define FX_FILESIZE off_t
#endif  // defined(OS_WIN)

#ifndef ASSERT
#ifndef NDEBUG
#define ASSERT assert
#else
#define ASSERT(a)
#endif  // NDEBUG
#endif  // ASSERT

// M_PI not universally present on all platforms.
#define FX_PI 3.1415926535897932384626433832795f
#define FX_BEZIER 0.5522847498308f

// NOTE: prevent use of the return value from snprintf() since some platforms
// have different return values.
#define FXSYS_snprintf (void)snprintf
#define FXSYS_vsnprintf (void)vsnprintf
#define FXSYS_sprintf DO_NOT_USE_SPRINTF_DIE_DIE_DIE
#define FXSYS_vsprintf DO_NOT_USE_VSPRINTF_DIE_DIE_DIE

#ifdef __cplusplus
}  // extern "C"

// Overloaded functions for C++ templates
inline size_t FXSYS_len(const char* ptr) {
  return strlen(ptr);
}

inline size_t FXSYS_len(const wchar_t* ptr) {
  return wcslen(ptr);
}

inline int FXSYS_cmp(const char* ptr1, const char* ptr2, size_t len) {
  return memcmp(ptr1, ptr2, len);
}

inline int FXSYS_cmp(const wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return wmemcmp(ptr1, ptr2, len);
}

inline const char* FXSYS_chr(const char* ptr, char ch, size_t len) {
  return reinterpret_cast<const char*>(memchr(ptr, ch, len));
}

inline const wchar_t* FXSYS_chr(const wchar_t* ptr, wchar_t ch, size_t len) {
  return wmemchr(ptr, ch, len);
}

extern "C" {
#endif  // __cplusplus

#if defined(OS_WIN)
#define FXSYS_GetACP GetACP
#define FXSYS_itoa _itoa
#define FXSYS_WideCharToMultiByte WideCharToMultiByte
#define FXSYS_MultiByteToWideChar MultiByteToWideChar
#define FXSYS_strlwr _strlwr
#define FXSYS_strupr _strupr
#define FXSYS_stricmp _stricmp
#define FXSYS_wcsicmp _wcsicmp
#define FXSYS_wcslwr _wcslwr
#define FXSYS_wcsupr _wcsupr
#define FXSYS_pow(a, b) (float)powf(a, b)
size_t FXSYS_wcsftime(wchar_t* strDest,
                      size_t maxsize,
                      const wchar_t* format,
                      const struct tm* timeptr);
#define FXSYS_SetLastError SetLastError
#define FXSYS_GetLastError GetLastError
#else  // defined(OS_WIN)
int FXSYS_GetACP();
char* FXSYS_itoa(int value, char* str, int radix);
int FXSYS_WideCharToMultiByte(uint32_t codepage,
                              uint32_t dwFlags,
                              const wchar_t* wstr,
                              int wlen,
                              char* buf,
                              int buflen,
                              const char* default_str,
                              int* pUseDefault);
int FXSYS_MultiByteToWideChar(uint32_t codepage,
                              uint32_t dwFlags,
                              const char* bstr,
                              int blen,
                              wchar_t* buf,
                              int buflen);
char* FXSYS_strlwr(char* str);
char* FXSYS_strupr(char* str);
int FXSYS_stricmp(const char* str1, const char* str2);
int FXSYS_wcsicmp(const wchar_t* str1, const wchar_t* str2);
wchar_t* FXSYS_wcslwr(wchar_t* str);
wchar_t* FXSYS_wcsupr(wchar_t* str);
#define FXSYS_pow(a, b) (float)pow(a, b)
#define FXSYS_wcsftime wcsftime
void FXSYS_SetLastError(uint32_t err);
uint32_t FXSYS_GetLastError();
#endif  // defined(OS_WIN)

#define FXWORD_GET_LSBFIRST(p)                                \
  (static_cast<uint16_t>((static_cast<uint16_t>(p[1]) << 8) | \
                         (static_cast<uint16_t>(p[0]))))
#define FXWORD_GET_MSBFIRST(p)                                \
  (static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) | \
                         (static_cast<uint16_t>(p[1]))))
#define FXDWORD_GET_LSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[3]) << 24) | (static_cast<uint32_t>(p[2]) << 16) | \
   (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[0])))
#define FXDWORD_GET_MSBFIRST(p)                                                \
  ((static_cast<uint32_t>(p[0]) << 24) | (static_cast<uint32_t>(p[1]) << 16) | \
   (static_cast<uint32_t>(p[2]) << 8) | (static_cast<uint32_t>(p[3])))
int32_t FXSYS_atoi(const char* str);
uint32_t FXSYS_atoui(const char* str);
int32_t FXSYS_wtoi(const wchar_t* str);
int64_t FXSYS_atoi64(const char* str);
const char* FXSYS_i64toa(int64_t value, char* str, int radix);
int FXSYS_roundf(float f);
int FXSYS_round(double d);
#define FXSYS_sqrt2(a, b) (float)sqrt((a) * (a) + (b) * (b))
#ifdef __cplusplus
}  // extern C
#endif  // __cplusplus

#endif  // CORE_FXCRT_FX_SYSTEM_H_
