/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "globalservice.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QString>

using namespace Qt::Literals::StringLiterals;

constexpr QLatin1String OSD_DBUS_SERVICE = "org.kde.plasmashell"_L1;
constexpr QLatin1String OSD_DBUS_PATH = "/org/kde/osdService"_L1;

void GlobalService::globalMute()
{
    invokeShortcut(u"mute"_s);
}

void GlobalService::volumeUp()
{
    invokeShortcut(u"increase_volume"_s);
}

void GlobalService::volumeDown()
{
    invokeShortcut(u"decrease_volume"_s);
}

void GlobalService::volumeUpSmall()
{
    invokeShortcut(u"increase_volume_small"_s);
}

void GlobalService::volumeDownSmall()
{
    invokeShortcut(u"decrease_volume_small"_s);
}

void GlobalService::invokeShortcut(const QString &name)
{
    ensureKDEDModule();
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage invokeMsg =
        QDBusMessage::createMethodCall(u"org.kde.kglobalaccel"_s, u"/component/kmix"_s, u"org.kde.kglobalaccel.Component"_s, u"invokeShortcut"_s);
    invokeMsg.setArguments({QVariant(name)});
    bus.call(invokeMsg, QDBus::NoBlock);
}

void GlobalService::ensureKDEDModule()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage invokeMsg = QDBusMessage::createMethodCall(u"org.kde.kded6"_s, u"/kded"_s, u"org.kde.kded6"_s, u"loadModule"_s);
    invokeMsg.setArguments({u"audioshortcutsservice"_s});
    bus.call(invokeMsg, QDBus::Block);
}