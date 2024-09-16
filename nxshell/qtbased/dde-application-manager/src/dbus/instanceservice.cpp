// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/instanceservice.h"
#include "constant.h"
#include "propertiesForwarder.h"
#include <QDBusMessage>
#include "global.h"

InstanceService::InstanceService(QString instanceId, QString application, QString systemdUnitPath, QString launcher)
    : m_Launcher(std::move(launcher))
    , m_instanceId(std::move(instanceId))
    , m_Application(std::move(application))
    , m_SystemdUnitPath(std::move(systemdUnitPath))
{
    if (auto *tmp = new (std::nothrow) PropertiesForwarder{application + "/" + instanceId, this}; tmp == nullptr) {
        qCritical() << "couldn't new PropertiesForwarder for instanceService.";
        return;
    }
}

InstanceService::~InstanceService() = default;

void InstanceService::KillAll(int signal)
{
    auto killMsg = QDBusMessage::createMethodCall(SystemdService, m_SystemdUnitPath.path(), SystemdUnitInterfaceName, "Kill");
    killMsg << QString{"all"} << signal;

    auto &con = ApplicationManager1DBus::instance().globalDestBus();
    auto reply = con.call(killMsg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        return;
    }

    safe_sendErrorReply(reply.errorName(), reply.errorMessage());
}
