/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "leadvertisement_p.h"

namespace BluezQt
{
LEAdvertisementPrivate::LEAdvertisementPrivate(const QStringList &serviceUuids)
    : m_serviceUuids(serviceUuids)
{
    static uint8_t advNumber = 0;
    QString objectPath = QLatin1String("/org/bluez/lead") + QString::number(advNumber++);
    m_objectPath.setPath(objectPath);
}

} // namespace BluezQt
