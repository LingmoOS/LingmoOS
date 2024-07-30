/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisement.h"
#include "leadvertisement_p.h"

namespace BluezQt
{
LEAdvertisement::LEAdvertisement(const QStringList &serviceUuids, QObject *parent)
    : QObject(parent)
    , d(new LEAdvertisementPrivate(serviceUuids))
{
}

LEAdvertisement::~LEAdvertisement() = default;

QDBusObjectPath LEAdvertisement::objectPath() const
{
    return d->m_objectPath;
}

QStringList LEAdvertisement::serviceUuids() const
{
    return d->m_serviceUuids;
}

QHash<QString, QByteArray> LEAdvertisement::serviceData() const
{
    return d->m_serviceData;
}

void LEAdvertisement::setServiceData(const QHash<QString, QByteArray> &data)
{
    d->m_serviceData = data;
}

QHash<quint16, QByteArray> LEAdvertisement::manufacturerData() const
{
    return d->m_manufacturerData;
}

void LEAdvertisement::setManufacturerData(const QHash<quint16, QByteArray> &data)
{
    d->m_manufacturerData = data;
}

void LEAdvertisement::release()
{
}

} // namespace BluezQt

#include "moc_leadvertisement.cpp"
