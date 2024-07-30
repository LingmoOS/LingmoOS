/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattservice_p.h"

namespace BluezQt
{
GattServicePrivate::GattServicePrivate(const QString &uuid, bool isPrimary, const QString &appPath)
    : m_uuid(uuid)
    , m_isPrimary(isPrimary)
{
    static uint8_t serviceNumber = 0;
    m_objectPath.setPath(appPath + QStringLiteral("/service") + QString::number(serviceNumber++));
}

} // namespace BluezQt
