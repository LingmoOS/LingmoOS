/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gsd-xsettings-manager.h"

#include <QDBusConnection>
#include <QDateTime>

#include "config_editor/settings_ini.h"
#include "gsd_xsettings_manager_adaptor.h"

namespace
{
inline constexpr QLatin1String GTK_SETTINGS_DBUS_PATH{"/org/gtk/Settings"};
inline constexpr QLatin1String GTK_SETTINGS_DBUS_NAME{"org.gtk.Settings"};
}

GSDXSettingsManager::GSDXSettingsManager(QObject *parent)
    : QObject(parent)
{
    new GSDXSettingsManagerAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(GTK_SETTINGS_DBUS_PATH, this);
    dbus.registerService(GTK_SETTINGS_DBUS_NAME);
}

GSDXSettingsManager::~GSDXSettingsManager()
{
}

void GSDXSettingsManager::modulesChanged()
{
    QDBusMessage msg =
        QDBusMessage::createSignal(GTK_SETTINGS_DBUS_PATH, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    QVariantList args{
        GTK_SETTINGS_DBUS_NAME,
        QVariantMap{{QStringLiteral("Modules"), Modules()}},
        QStringList(),
    };
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

void GSDXSettingsManager::enableAnimationsChanged()
{
    QDBusMessage msg =
        QDBusMessage::createSignal(GTK_SETTINGS_DBUS_PATH, QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    QVariantList args{
        GTK_SETTINGS_DBUS_NAME,
        QVariantMap{{QStringLiteral("EnableAnimations"), EnableAnimations()}},
        QStringList(),
    };
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

qlonglong GSDXSettingsManager::FontconfigTimestamp() const
{
    return QDateTime::currentSecsSinceEpoch();
}

QString GSDXSettingsManager::Modules() const
{
    return SettingsIniEditor::value(QStringLiteral("gtk-modules"), 3);
}

bool GSDXSettingsManager::EnableAnimations() const
{
    return SettingsIniEditor::value(QStringLiteral("gtk-enable-animations"), 3) == QLatin1String("true");
}

#include "moc_gsd-xsettings-manager.cpp"
