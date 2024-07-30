/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIAENDPOINT_P_H
#define BLUEZQT_MEDIAENDPOINT_P_H

#include <QDBusObjectPath>
#include <QVariantMap>

#include "mediaendpoint.h"

namespace BluezQt
{
class MediaEndpointPrivate
{
public:
    explicit MediaEndpointPrivate(const MediaEndpoint::Configuration &configuration);

    void init(const MediaEndpoint::Configuration &configuration);

    QVariantMap m_properties;
    MediaEndpoint::Configuration m_configuration;
    QDBusObjectPath m_objectPath;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIAENDPOINT_P_H
