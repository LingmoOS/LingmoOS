// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

int main(int argc, char *argv[])
{
    QDBusInterface *inter1 = new QDBusInterface("org.services.demo1",
                                                "/org/services/demo",
                                                "org.services.demo",
                                                QDBusConnection::sessionBus(),
                                                nullptr);
    QDBusReply<void> setData = inter1->call("SetData", "AAA");
    if (setData.isValid()) {
        qInfo() << "Server1:setData success";
    } else {
        qInfo() << "Server1:setData fail";
    }

    QDBusInterface *inter2 = new QDBusInterface("org.services.demo2",
                                                "/org/services/demo",
                                                "org.services.demo",
                                                QDBusConnection::sessionBus(),
                                                nullptr);
    QDBusReply<void> setData2 = inter2->call("SetData", "BBB");
    if (setData2.isValid()) {
        qInfo() << "Server2:setData success";
    } else {
        qInfo() << "Server2:setData fail";
    }
}