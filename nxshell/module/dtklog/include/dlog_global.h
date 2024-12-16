// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DLOGGER_GLOBAL_H
#define DLOGGER_GLOBAL_H

#include <QtCore/qglobal.h>

#define DLOG_NAMESPACE Dtk

#if !defined(DLOG_NAMESPACE)
#define DLOG_BEGIN_NAMESPACE
#define DLOG_END_NAMESPACE
#define DLOG_USE_NAMESPACE
#else
#define DLOG_BEGIN_NAMESPACE \
    namespace DLOG_NAMESPACE \
    {
#define DLOG_END_NAMESPACE }
#define DLOG_USE_NAMESPACE using namespace DLOG_NAMESPACE;
#endif

#define DLOGCORE_NAMESPACE Core
#define DLOG_CORE_NAMESPACE DLOG_NAMESPACE::DLOGCORE_NAMESPACE

#if !defined(DLOGCORE_NAMESPACE)
#define DLOG_CORE_BEGIN_NAMESPACE
#define DLOG_CORE_END_NAMESPACE
#define DLOG_CORE_USE_NAMESPACE
#else
#define DLOG_CORE_BEGIN_NAMESPACE    \
    namespace DLOG_NAMESPACE         \
    {                                \
        namespace DLOGCORE_NAMESPACE \
        {
#define DLOG_CORE_END_NAMESPACE \
    }                           \
    }
#define DLOG_CORE_USE_NAMESPACE using namespace DLOG_CORE_NAMESPACE;
#endif

#if defined(DLOG_STATIC_LIB)
#define LIBDLOG_SHARED_EXPORT
#else
#if defined(LIBDLOG_LIBRARY)
#define LIBDLOG_SHARED_EXPORT Q_DECL_EXPORT
#else
#define LIBDLOG_SHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#ifdef DLOG_DEPRECATED_CHECK
#define DLOG_DECL_DEPRECATED_X(text) Q_DECL_HIDDEN
#define DLOG_DECL_DEPRECATED Q_DECL_HIDDEN
#else
#ifdef __GNUC__
#if __GNUC__ < 13
#define DLOG_DECL_DEPRECATED __attribute__((__deprecated__))
#define DLOG_DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
#else
#define DLOG_DECL_DEPRECATED Q_DECL_DEPRECATED
#define DLOG_DECL_DEPRECATED_X Q_DECL_DEPRECATED_X
#endif
#else
#define DLOG_DECL_DEPRECATED Q_DECL_DEPRECATED
#define DLOG_DECL_DEPRECATED_X Q_DECL_DEPRECATED_X
#endif
#endif

#endif // DLOGGER_GLOBAL_H
