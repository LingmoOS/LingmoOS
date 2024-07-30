/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHATPROFILE_H
#define CHATPROFILE_H

#include "device.h"
#include "profile.h"

class QLocalSocket;

class ChatProfile : public BluezQt::Profile
{
    Q_OBJECT

public:
    explicit ChatProfile(QObject *parent);

    QDBusObjectPath objectPath() const override;
    QString uuid() const override;

    void newConnection(BluezQt::DevicePtr device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties, const BluezQt::Request<> &request) override;
    void requestDisconnection(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;
    void release() override;

private Q_SLOTS:
    void socketReadyRead();
    void socketDisconnected();
    void writeToSocket();

private:
    QSharedPointer<QLocalSocket> m_socket;
};

#endif // CHATPROFILE_H
