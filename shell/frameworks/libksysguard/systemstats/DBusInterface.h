/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "org.kde.ksystemstats1.h"
#include "systemstats_export.h"

namespace KSysGuard
{
namespace SystemStats
{
const QString ServiceName = QStringLiteral("org.kde.ksystemstats1");
const QString ObjectPath = QStringLiteral("/org/kde/ksystemstats1");
const QString InterfaceName = QStringLiteral("org.kde.ksystemstats1");

/**
 * This exposes the generated DBus interface for org.kde.ksystemstats
 */
class SYSTEMSTATS_EXPORT DBusInterface : public org::kde::ksystemstats1
{
    Q_OBJECT
public:
    DBusInterface(const QString &service = ServiceName,
                  const QString &path = ObjectPath,
                  const QDBusConnection &connection = QDBusConnection::sessionBus(),
                  QObject *parent = nullptr);
};

} // namespace SystemStats
} // namespace KSysGuard
