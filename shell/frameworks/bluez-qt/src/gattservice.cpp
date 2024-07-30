/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattservice.h"
#include "gattapplication.h"
#include "gattservice_p.h"

namespace BluezQt
{
GattService::GattService(const QString &uuid, bool isPrimary, GattApplication *parent)
    : QObject(parent)
    , d(new GattServicePrivate(uuid, isPrimary, parent->objectPath().path()))
{
}

GattService::~GattService() = default;

QString GattService::uuid() const
{
    return d->m_uuid;
}

bool GattService::isPrimary() const
{
    return d->m_isPrimary;
}

QDBusObjectPath GattService::objectPath() const
{
    return d->m_objectPath;
}

} // namespace BluezQt

#include "moc_gattservice.cpp"
