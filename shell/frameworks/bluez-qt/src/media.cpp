/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "media.h"
#include "debug.h"
#include "media_p.h"
#include "mediaendpoint.h"
#include "mediaendpointadaptor.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt
{
Media::Media(const QString &path, QObject *parent)
    : QObject(parent)
    , d(new MediaPrivate())
{
    d->m_path = path;
    d->m_bluezMedia = new BluezMedia(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);
}

Media::~Media() = default;

PendingCall *Media::registerEndpoint(MediaEndpoint *endpoint)
{
    Q_ASSERT(endpoint);

    if (!d->m_bluezMedia) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Media not operational!"));
    }

    new MediaEndpointAdaptor(endpoint);

    if (!DBusConnection::orgBluez().registerObject(endpoint->objectPath().path(), endpoint)) {
        qCDebug(BLUEZQT) << "Cannot register object" << endpoint->objectPath().path();
    }

    return new PendingCall(d->m_bluezMedia->RegisterEndpoint(endpoint->objectPath(), endpoint->properties()), PendingCall::ReturnVoid, this);
}

PendingCall *Media::unregisterEndpoint(MediaEndpoint *endpoint)
{
    Q_ASSERT(endpoint);

    if (!d->m_bluezMedia) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("Media not operational!"));
    }

    DBusConnection::orgBluez().unregisterObject(endpoint->objectPath().path());

    return new PendingCall(d->m_bluezMedia->UnregisterEndpoint(endpoint->objectPath()), PendingCall::ReturnVoid, this);
}

} // namespace BluezQt

#include "moc_media.cpp"
