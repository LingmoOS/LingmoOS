/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 * SPDX-FileCopyrightText: 2021 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "NetworkBackend.h"
#include "NetworkDevice.h"

#include <QElapsedTimer>

#include <netlink/cache.h>
#include <netlink/socket.h>

struct rtnl_addr;
struct rtnl_link;

class RtNetlinkDevice : public NetworkDevice
{
    Q_OBJECT
public:
    RtNetlinkDevice(const QString &id);
    void update(rtnl_link *link, nl_cache *address_cache, nl_cache *route_cache, qint64 elapsedTime);
Q_SIGNALS:
    void connected();
    void disconnected();

private:
    bool m_connected = false;
};

class RtNetlinkBackend : public NetworkBackend
{
public:
    RtNetlinkBackend(QObject *parent);
    ~RtNetlinkBackend() override;
    bool isSupported() override;
    void start() override;
    void stop() override;
    void update() override;

private:
    QHash<QByteArray, RtNetlinkDevice *> m_devices;
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> m_socket;
    QElapsedTimer m_updateTimer;
};
