// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef Q_HTTP_SERVER_API
#define Q_HTTP_SERVER_API

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#ifdef Q_OS_WIN
// Define to export or import depending if we are building or using the library.
// QHTTPSERVER_EXPORT should only be defined when building.
#if defined(QHTTPSERVER_EXPORT)
#define QHTTPSERVER_API __declspec(dllexport)
#else
#define QHTTPSERVER_API __declspec(dllimport)
#endif
#else
// Define empty for other platforms
#define QHTTPSERVER_API
#endif
#else
#ifdef Q_WS_WIN
// Define to export or import depending if we are building or using the library.
// QHTTPSERVER_EXPORT should only be defined when building.
#if defined(QHTTPSERVER_EXPORT)
#define QHTTPSERVER_API __declspec(dllexport)
#else
#define QHTTPSERVER_API __declspec(dllimport)
#endif
#else
// Define empty for other platforms
#define QHTTPSERVER_API
#endif
#endif

#endif
