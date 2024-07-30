/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaendpointadaptor.h"
#include "mediaendpoint.h"
#include "request.h"

#include <QDBusMessage>
#include <QDBusObjectPath>

namespace BluezQt
{
MediaEndpointAdaptor::MediaEndpointAdaptor(MediaEndpoint *parent)
    : QDBusAbstractAdaptor(parent)
    , m_endpoint(parent)
{
}

void MediaEndpointAdaptor::SetConfiguration(const QDBusObjectPath &transport, const QVariantMap &properties)
{
    m_endpoint->setConfiguration(transport.path(), properties);
}

QByteArray MediaEndpointAdaptor::SelectConfiguration(const QByteArray &capabilities, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    Request<QByteArray> req(OrgBluezMediaEndpoint, msg);

    m_endpoint->selectConfiguration(capabilities, req);
    return QByteArray();
}

void MediaEndpointAdaptor::ClearConfiguration(const QDBusObjectPath &transport)
{
    m_endpoint->clearConfiguration(transport.path());
}

void MediaEndpointAdaptor::Release()
{
    m_endpoint->release();
}

} // namespace BluezQt

#include "moc_mediaendpointadaptor.cpp"
