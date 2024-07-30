/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fdpowernotifier.h"

#include <QDBusConnection>
#include <QDebug>

Solid::FDPowerNotifier::FDPowerNotifier(QObject *parent)
    : PowerNotifier(parent)
{
    auto conn = QDBusConnection::systemBus();
    conn.connect(QStringLiteral("org.freedesktop.UPower"),
                 QStringLiteral("/org/freedesktop/UPower"),
                 QStringLiteral("org.freedesktop.DBus.Properties"),
                 QStringLiteral("PropertiesChanged"),
                 this,
                 SLOT(upowerPropertiesChanged(QString, QVariantMap, QStringList)));

    conn.connect(QStringLiteral("org.freedesktop.login1"),
                 QStringLiteral("/org/freedesktop/login1"),
                 QStringLiteral("org.freedesktop.login1.Manager"),
                 QStringLiteral("PrepareForSleep"),
                 this,
                 SLOT(login1Resuming(bool)));
}

void Solid::FDPowerNotifier::upowerPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidated)
{
    if (interface != QStringLiteral("org.freedesktop.UPower")) {
        return;
    }

    if (changedProperties.contains(QStringLiteral("OnBattery"))) {
        Q_EMIT acPluggedChanged(!changedProperties.value(QStringLiteral("OnBattery")).toBool());
    }

    // Just for debug purposes
    if (!invalidated.isEmpty()) {
        qDebug() << "Invalidated" << invalidated;
    }
}

void Solid::FDPowerNotifier::login1Resuming(bool active)
{
    if (active) {
        Q_EMIT aboutToSuspend();
    } else {
        Q_EMIT resumeFromSuspend();
    }
}

#include "moc_fdpowernotifier.cpp"
