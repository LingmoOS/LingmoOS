// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "startmanagerservice.h"

#include <qdbusconnection.h>
#include <qdbuserror.h>
#include <qdebug.h>

DLOGIN_BEGIN_NAMESPACE

StartManagerService::StartManagerService(QObject *parent)
    : QObject(parent)
{
    registerService();
}

StartManagerService::~StartManagerService()
{
    unRegisterService();
}

bool StartManagerService::AddAutostart(const QString &fileName)
{
    m_fileName = fileName;
    return m_success;
}

bool StartManagerService::RemoveAutostart(const QString &fileName)
{
    m_fileName = fileName;
    return m_success;
}

bool StartManagerService::registerService()
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
    if (!connection.registerObject("/com/deepin/FakeStartManager",
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

void StartManagerService::unRegisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject("/com/deepin/FakeStartManager");
    connection.unregisterService("com.deepin.FakeSessionManager");
}

DLOGIN_END_NAMESPACE