// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DTKDEVICE_GLOBAL_H
#define DTKDEVICE_GLOBAL_H
#include <qnamespace.h>
#include <QtCore/qglobal.h>

#define DTK_NAMESPACE Dtk
#define DTK_BEGIN_NAMESPACE namespace DTK_NAMESPACE {
#define DTK_END_NAMESPACE }
#define DTK_USE_NAMESPACE using namespace DTK_NAMESPACE;

#define DDEVICE_NAMESPACE Device
#define DTK_DEVICE_NAMESPACE DTK_NAMESPACE::DDEVICE_NAMESPACE

#define DDEVICE_BEGIN_NAMESPACE namespace DTK_NAMESPACE { namespace DDEVICE_NAMESPACE {
#define DDEVICE_END_NAMESPACE }}
#define DDEVICE_USE_NAMESPACE using namespace DTK_DEVICE_NAMESPACE;

#if defined(DTK_STATIC_LIB)
#  define LIBDTKDEVICESHARED_EXPORT
#else
#if defined(LIBDTKDEVICE_LIBRARY)
#  define LIBDTKDEVICESHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBDTKDEVICESHARED_EXPORT Q_DECL_IMPORT
#endif
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    #define D_SPLIT_BEHAVIOR Qt::SkipEmptyParts
#else
    #define D_SPLIT_BEHAVIOR QString::SkipEmptyParts
#endif

#endif
