/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemfirmware.h"
#include "modemfirmware_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::ModemFirmwarePrivate::ModemFirmwarePrivate(const QString &path, ModemFirmware *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemFirmwareIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemFirmwareIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
}

ModemManager::ModemFirmware::ModemFirmware(const QString &path, QObject *parent)
    : Interface(*new ModemFirmwarePrivate(path, this), parent)
{
}

ModemManager::ModemFirmware::~ModemFirmware()
{
}

QDBusPendingReply<QString, ModemManager::QVariantMapList> ModemManager::ModemFirmware::listImages()
{
    Q_D(ModemFirmware);
    return d->modemFirmwareIface.List();
}

void ModemManager::ModemFirmware::selectImage(const QString &uniqueid)
{
    Q_D(ModemFirmware);
    d->modemFirmwareIface.Select(uniqueid);
}

#include "moc_modemfirmware.cpp"
