/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIAENDPOINTADAPTOR_H
#define BLUEZQT_MEDIAENDPOINTADAPTOR_H

#include <QDBusAbstractAdaptor>

class QDBusMessage;
class QDBusObjectPath;

namespace BluezQt
{
class MediaEndpoint;

class MediaEndpointAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.MediaEndpoint1")

public:
    explicit MediaEndpointAdaptor(MediaEndpoint *parent);

public Q_SLOTS:
    void SetConfiguration(const QDBusObjectPath &transport, const QVariantMap &properties);
    QByteArray SelectConfiguration(const QByteArray &capabilities, const QDBusMessage &msg);
    void ClearConfiguration(const QDBusObjectPath &transport);
    Q_NOREPLY void Release();

private:
    MediaEndpoint *m_endpoint;
};

} // namespace BluezQt

#endif // BLUEZQT_MEDIAENDPOINTADAPTOR_H
