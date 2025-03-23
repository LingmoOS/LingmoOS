// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQUICKGLOBAL_P_H
#define DQUICKGLOBAL_P_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
#define ThrowError(obj, message) {\
    auto e = qmlEngine(obj);\
    if (e) e->throwError(message);\
    else qCritical().noquote() << message.toLocal8Bit();\
}
#else
#define ThrowError(obj, message) \
    qCritical().noquote() << message.toLocal8Bit()
#endif

#endif // DQUICKGLOBAL_P_H
