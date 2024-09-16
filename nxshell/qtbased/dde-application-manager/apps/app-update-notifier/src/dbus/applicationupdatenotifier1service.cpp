// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/applicationupdatenotifier1adaptor.h"

ApplicationUpdateNotifier1Service::ApplicationUpdateNotifier1Service()
{
    new ApplicationUpdateNotifier1Adaptor{this};
    auto conn = QDBusConnection::systemBus();
    if (!conn.registerService(NotifierServiceName)) {
        qFatal("%s", conn.lastError().message().toLocal8Bit().constData());
    }

    if (!conn.registerObject(NotifierObjectPath, NotifierInterfaceName, this, QDBusConnection::ExportAdaptors)) {
        qFatal("%s", conn.lastError().message().toLocal8Bit().constData());
    }
}
