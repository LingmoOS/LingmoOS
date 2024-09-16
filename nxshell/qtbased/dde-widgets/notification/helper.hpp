// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDebug>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>

namespace notify {
class Helper {
public:
    inline static Helper *instance()
    {
        static Helper *g_helper = nullptr;
        if (!g_helper)
            g_helper = new Helper();
        return g_helper;
    }

    inline void showNotificationModuleOfControlCenter();
};

void Helper::showNotificationModuleOfControlCenter()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1",
                                                          "org.deepin.dde.ControlCenter1","ShowPage");
    message << "notification";
    QDBusConnection::sessionBus().asyncCall(message);
}
}
using namespace notify;
