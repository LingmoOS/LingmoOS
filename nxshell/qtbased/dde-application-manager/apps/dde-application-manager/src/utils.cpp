// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global.h"

bool registerObjectToDBus(QObject *o, const QString &path, const QString &interface)
{
    auto &con = ApplicationManager1DBus::instance().globalServerBus();
    if (!con.registerObject(path, interface, o, QDBusConnection::RegisterOption::ExportAdaptors)) {
        qCritical() << "register object failed:" << path << interface << con.lastError();
        return false;
    }
    qDebug() << "register object:" << path << interface;
    return true;
}

void unregisterObjectFromDBus(const QString &path)
{
    auto &con = ApplicationManager1DBus::instance().globalServerBus();
    con.unregisterObject(path);
    qDebug() << "unregister object:" << path;
}
