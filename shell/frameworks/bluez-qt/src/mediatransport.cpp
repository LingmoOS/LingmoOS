/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediatransport.h"
#include "mediatransport_p.h"

namespace BluezQt
{
MediaTransport::MediaTransport(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new MediaTransportPrivate(path, properties))
{
}

MediaTransport::~MediaTransport() = default;

AudioConfiguration MediaTransport::audioConfiguration() const
{
    return d->m_configuration;
}

MediaTransport::State MediaTransport::state() const
{
    return d->m_state;
}

quint16 MediaTransport::volume() const
{
    return d->m_volume;
}

TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t> *MediaTransport::acquire()
{
    return new TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t>(d->m_dbusInterface.Acquire(), this);
}

TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t> *MediaTransport::tryAcquire()
{
    return new TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t>(d->m_dbusInterface.TryAcquire(), this);
}

TPendingCall<void> *MediaTransport::release()
{
    return new TPendingCall<void>(d->m_dbusInterface.Release(), this);
}

} // namespace BluezQt

#include "moc_mediatransport.cpp"
