// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSUTILS_H
#define DBUSUTILS_H
#include <QVariant>


class DBusUtils
{
public:
    DBusUtils();
    static QVariant redDBusProperty(const QString &service, const QString &path, const QString &interface = QString(), const char* propert = "");
    static QVariant redDBusMethod(const QString &service, const QString &path, const QString &interface, const char *method);
};

#endif // DBUSUTILS_H
