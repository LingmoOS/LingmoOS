/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_LEADVERTISEMENT_P_H
#define BLUEZQT_LEADVERTISEMENT_P_H

#include <QDBusObjectPath>

namespace BluezQt
{
class LEAdvertisementPrivate
{
public:
    explicit LEAdvertisementPrivate(const QStringList &serviceUuids);
    QStringList m_serviceUuids;
    QDBusObjectPath m_objectPath;
    QHash<QString, QByteArray> m_serviceData;
    QHash<quint16, QByteArray> m_manufacturerData;
};

} // namespace BluezQt

#endif // BLUEZQT_LEADVERTISEMENT_P_H
