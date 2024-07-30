/*
    SPDX-FileCopyrightText: 2005 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ifaces/device.h"

#ifdef HAVE_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#endif

Solid::Ifaces::Device::Device(QObject *parent)
    : QObject(parent)
{
}

Solid::Ifaces::Device::~Device()
{
}

QString Solid::Ifaces::Device::parentUdi() const
{
    return QString();
}

void Solid::Ifaces::Device::registerAction(const QString &actionName, QObject *dest, const char *requestSlot, const char *doneSlot) const
{
#ifdef HAVE_DBUS
    QDBusConnection::sessionBus().connect(QString(), deviceDBusPath(), "org.kde.Solid.Device", actionName + "Requested", dest, requestSlot);

    QDBusConnection::sessionBus().connect(QString(), deviceDBusPath(), "org.kde.Solid.Device", actionName + "Done", dest, doneSlot);
#endif
}

void Solid::Ifaces::Device::broadcastActionDone(const QString &actionName, int error, const QString &errorString) const
{
#ifdef HAVE_DBUS
    QDBusMessage signal = QDBusMessage::createSignal(deviceDBusPath(), "org.kde.Solid.Device", actionName + "Done");
    signal << error << errorString;

    QDBusConnection::sessionBus().send(signal);
#endif
}

void Solid::Ifaces::Device::broadcastActionRequested(const QString &actionName) const
{
#ifdef HAVE_DBUS
    QDBusMessage signal = QDBusMessage::createSignal(deviceDBusPath(), "org.kde.Solid.Device", actionName + "Requested");
    QDBusConnection::sessionBus().send(signal);
#endif
}

QString Solid::Ifaces::Device::deviceDBusPath() const
{
    const QByteArray encodedUdi = udi().toUtf8().toPercentEncoding(QByteArray(), ".~-", '_');
    return QString("/org/kde/solid/Device_") + QString::fromLatin1(encodedUdi);
}

QString Solid::Ifaces::Device::displayName() const
{
    return description();
}

#include "moc_device.cpp"
