// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPORTGLOBAL_H
#define EXPORTGLOBAL_H

// Qt build
#ifdef QT_BUILD_SHARED
  #include <QtCore/QtGlobal>
  #define EXPORT_API Q_DECL_EXPORT
  #define IMPORT_API Q_DECL_IMPORT
#elif defined(BUILD_SHARED)
  #define EXPORT_API __declspec(dllexport)
  #define IMPORT_API __declspec(dllimport)
#else
  #define EXPORT_API
  #define IMPORT_API
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

#endif // EXPORTGLOBAL_H
