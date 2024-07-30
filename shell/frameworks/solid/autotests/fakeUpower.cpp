/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeUpower.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <qdbusmessage.h>

FakeUpower::FakeUpower(QObject *parent)
    : QObject(parent)
    , m_onBattery(false)
{
}

QString FakeUpower::daemonVersion() const
{
    return "POP";
}

QString FakeUpower::GetCriticalAction()
{
    return "AAAA";
}

QDBusObjectPath FakeUpower::GetDisplayDevice()
{
    return QDBusObjectPath("AA");
}

bool FakeUpower::isDocked() const
{
    return true;
}

bool FakeUpower::lidIsClosed() const
{
    return false;
}

bool FakeUpower::lidIsPresent() const
{
    return true;
}

bool FakeUpower::onBattery() const
{
    return m_onBattery;
}

void FakeUpower::setOnBattery(bool onBattery)
{
    m_onBattery = onBattery;

    emitPropertiesChanged(QStringLiteral("OnBattery"), m_onBattery);
}

void FakeUpower::emitPropertiesChanged(const QString &name, const QVariant &value)
{
    auto msg = QDBusMessage::createSignal(QStringLiteral("/org/freedesktop/UPower"),
                                          QStringLiteral("org.freedesktop.DBus.Properties"),
                                          QStringLiteral("PropertiesChanged"));

    QVariantMap map;
    map.insert(name, value);
    QList<QVariant> args;
    args << QString("org.freedesktop.UPower");
    args << map;
    args << QStringList();

    msg.setArguments(args);

    QDBusConnection::systemBus().asyncCall(msg);
}

QList<QDBusObjectPath> FakeUpower::EnumerateDevices()
{
    QList<QDBusObjectPath> list;
    return list;
}

#include "moc_fakeUpower.cpp"
