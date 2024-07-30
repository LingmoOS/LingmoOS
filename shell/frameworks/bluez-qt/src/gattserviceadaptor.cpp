/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattserviceadaptor.h"

#include "gattservice.h"
#include "request.h"

#include <QDBusObjectPath>

namespace BluezQt
{
GattServiceAdaptor::GattServiceAdaptor(GattService *parent)
    : QDBusAbstractAdaptor(parent)
    , m_gattService(parent)
{
}

QString GattServiceAdaptor::uuid() const
{
    return m_gattService->uuid();
}

bool GattServiceAdaptor::primary() const
{
    return m_gattService->isPrimary();
}

} // namespace BluezQt

#include "moc_gattserviceadaptor.cpp"
