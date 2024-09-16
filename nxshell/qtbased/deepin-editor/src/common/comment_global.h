// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <qglobal.h>

#if defined(UTILS_LIBRARY)
#  define COMMENT_EXPORT Q_DECL_EXPORT
#elif  defined(UTILS_STATIC_LIB)
#  define COMMENT_EXPORT
#else
#  define COMMENT_EXPORT Q_DECL_IMPORT
#endif
