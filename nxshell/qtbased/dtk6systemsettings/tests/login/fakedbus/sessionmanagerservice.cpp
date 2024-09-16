// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionmanagerservice.h"

#include <qdbusconnection.h>
#include <qdbuserror.h>
#include <qdebug.h>

DLOGIN_BEGIN_NAMESPACE

SessionManagerService::SessionManagerService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

SessionManagerService::~SessionManagerService()
{
    unRegisterService();
}

bool SessionManagerService::Locked()
{
    return m_locked;
}

void SessionManagerService::SetLocked(const bool Locked)
{
    m_locked = Locked;
}

bool SessionManagerService::registerService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.FakeSessionManager")) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.")
                              .arg("com.deepin.FakeSessionManager")
                              .arg(errorMsg);
        return false;
    }
    if (!connection.registerObject("/com/deepin/FakeSessionManager",
                                   this,
                                   QDBusConnection::ExportScriptableContents)) {
        QString errorMsg = connection.lastError().message();
        qWarning() << QString("Can't register %1 the D-Bus object, %2.")
                              .arg("com.deepin.FakeSessionManager")
                              .arg(errorMsg);
        return false;
    }
    return true;
}

void SessionManagerService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject("/com/deepin/FakeSessionManager");
    connection.unregisterService("com.deepin.FakeSessionManager");
}

DLOGIN_END_NAMESPACE
