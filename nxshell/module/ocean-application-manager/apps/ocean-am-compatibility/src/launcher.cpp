// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "launcher.h"
#include "global.h"

#include <QDBusConnection>
#include <QDBusMetaType>
#include <DConfig>

DCORE_USE_NAMESPACE

void Launcher::setPath(const QString &path)
{
    m_path = path;
}

void Launcher::setAction(const QString &action)
{
    m_action = action;
}

void Launcher::setLaunchedType(LaunchedType type)
{
    m_launchedType = type;
}

DExpected<void> Launcher::run()
{
    if (auto value = launch(); !value)
        return value;

    return {};
}

DExpected<void> Launcher::launch()
{
    auto con = QDBusConnection::sessionBus();
    auto msg = QDBusMessage::createMethodCall(
        OCEANApplicationManager1ServiceName, m_path, ApplicationInterface, "Launch");
    const QList<QVariant> arguments {
        m_action,
        QStringList{},
        QVariantMap{{QString{"LaunchType"}, QString{"Compatibility"}}}
    };
    msg.setArguments(arguments);
    auto reply = con.call(msg);

    if (reply.type() != QDBusMessage::ReplyMessage) {
        return DUnexpected{emplace_tag::USE_EMPLACE,
                           static_cast<int>(reply.type()),
                           QString("Failed to launch: %1, error: %2").arg(m_path, reply.errorMessage())};
    }
    return {};
}