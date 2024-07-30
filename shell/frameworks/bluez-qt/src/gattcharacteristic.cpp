/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristic.h"
#include "gattcharacteristic_p.h"
#include "gattservice.h"
#include "utils.h"

namespace BluezQt
{
GattCharacteristic::GattCharacteristic(const QString &uuid, GattService *service)
    : GattCharacteristic(uuid, {QLatin1String("read"), QLatin1String("write")}, service)
{
}

GattCharacteristic::GattCharacteristic(const QString &uuid, const QStringList &flags, GattService *service)
    : QObject(service)
    , d(new GattCharacterisiticPrivate(uuid, flags, service))
{
}

GattCharacteristic::~GattCharacteristic() = default;

QByteArray GattCharacteristic::readValue()
{
    if (d->m_readCallback) {
        d->m_value = d->m_readCallback();
    }

    return d->m_value;
}

void GattCharacteristic::writeValue(const QByteArray &value)
{
    d->m_value = value;

    if (isNotifying()) {
        d->emitPropertyChanged({{QLatin1String("Value"), value}});
    }

    Q_EMIT valueWritten(d->m_value);
}

QString GattCharacteristic::uuid() const
{
    return d->m_uuid;
}

const GattService *GattCharacteristic::service() const
{
    return d->m_service;
}

QStringList GattCharacteristic::flags() const
{
    return d->m_flags;
}

void GattCharacteristic::startNotify()
{
    if (d->m_canNotify) {
        d->m_notifying = true;
    }
}

void GattCharacteristic::stopNotify()
{
    d->m_notifying = false;
}

bool GattCharacteristic::isNotifying() const
{
    return d->m_notifying;
}

QDBusObjectPath GattCharacteristic::objectPath() const
{
    return d->m_objectPath;
}

void GattCharacteristic::setReadCallback(ReadCallback callback)
{
    d->m_readCallback = callback;
}

} // namespace BluezQt

#include "moc_gattcharacteristic.cpp"
