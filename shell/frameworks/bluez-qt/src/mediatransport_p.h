/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIATRANSPORT_P_H
#define BLUEZQT_MEDIATRANSPORT_P_H

#include "bluezmediatransport1.h"
#include "dbusproperties.h"
#include "mediatransport.h"

namespace BluezQt
{
typedef org::freedesktop::DBus::Properties DBusProperties;

class MediaTransportPrivate : public QObject
{
    Q_OBJECT

public:
    explicit MediaTransportPrivate(const QString &path, const QVariantMap &properties);

    void init(const QVariantMap &properties);

public Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

public:
    QWeakPointer<MediaTransport> q;
    OrgBluezMediaTransport1Interface m_dbusInterface;
    DBusProperties *m_dbusProperties;

    QString m_path;
    AudioConfiguration m_configuration;
    MediaTransport::State m_state = MediaTransport::State::Idle;
    quint16 m_volume = 0;
};

} // namespace BluezQt

#endif
