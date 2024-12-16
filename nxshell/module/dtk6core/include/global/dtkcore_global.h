// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QtCore/qglobal.h>
#include <dtkcore_config.h>

#define DTK_NAMESPACE Dtk

#if !defined(DTK_NAMESPACE)
#   define DTK_BEGIN_NAMESPACE
#   define DTK_END_NAMESPACE
#   define DTK_USE_NAMESPACE
#else
#   define DTK_BEGIN_NAMESPACE namespace DTK_NAMESPACE {
#   define DTK_END_NAMESPACE }
#   define DTK_USE_NAMESPACE using namespace DTK_NAMESPACE;
#endif

#define DCORE_NAMESPACE Core
#define DTK_CORE_NAMESPACE DTK_NAMESPACE::DCORE_NAMESPACE

#if !defined(DCORE_NAMESPACE)
#   define DCORE_BEGIN_NAMESPACE
#   define DCORE_END_NAMESPACE
#   define DCORE_USE_NAMESPACE
#else
#   define DCORE_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DCORE_NAMESPACE {
#   define DCORE_END_NAMESPACE }}
#   define DCORE_USE_NAMESPACE using namespace DTK_CORE_NAMESPACE;
#endif


#if defined(DTK_STATIC_LIB)
#  define LIBDTKCORESHARED_EXPORT
#else
#if defined(LIBDTKCORE_LIBRARY)
#  define LIBDTKCORESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDTKCORESHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#ifdef D_DEPRECATED_CHECK
#define D_DECL_DEPRECATED_X(text) Q_DECL_HIDDEN
#define D_DECL_DEPRECATED Q_DECL_HIDDEN
#else
#ifdef __GNUC__
#if __GNUC__ < 13
#define D_DECL_DEPRECATED __attribute__((__deprecated__))
#define D_DECL_DEPRECATED_X(text) __attribute__((__deprecated__(text)))
#else
#define D_DECL_DEPRECATED Q_DECL_DEPRECATED
#define D_DECL_DEPRECATED_X Q_DECL_DEPRECATED_X
#endif
#else
#define D_DECL_DEPRECATED Q_DECL_DEPRECATED
#define D_DECL_DEPRECATED_X Q_DECL_DEPRECATED_X
#endif
#endif

#define DTK_VERSION_CHECK(major, minor, patch, build) ((major<<24)|(minor<<16)|(patch<<8)|build)
#define DTK_VERSION DTK_VERSION_CHECK(DTK_VERSION_MAJOR, DTK_VERSION_MINOR, DTK_VERSION_PATCH, DTK_VERSION_BUILD)

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
extern "C" {
int LIBDTKCORESHARED_EXPORT dtkVersion();
const LIBDTKCORESHARED_EXPORT char *dtkVersionString();
}
#endif
