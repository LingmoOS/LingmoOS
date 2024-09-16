// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QT5DWAYLANDPLUGIN_GLOBAL_H
#define QT5DWAYLANDPLUGIN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QT5DWAYLANDPLUGIN_LIBRARY)
#  define QT5DWAYLANDPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QT5DWAYLANDPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QT5DWAYLANDPLUGIN_GLOBAL_H
