// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;

#define SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#define QT_ENDL Qt::endl

using EnterEvent = QEnterEvent;
#else
class QEvent;

#define SKIP_EMPTY_PARTS QString::SkipEmptyParts
#define QT_ENDL endl

using EnterEvent = QEvent;
#endif

#endif // QTCOMPAT_H
