/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LINGMO_IPV4_ARPING
#define LINGMO_IPV4_ARPING

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <poll.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "lingmoarping.h"

#include <QString>
#include <QDebug>
#include <QObject>

enum {
    POLLFD_TIMER = 0,
    POLLFD_SOCKET,
    POLLFD_COUNT
};

class KyIpv4Arping : public QObject
{
    Q_OBJECT
public:
    explicit KyIpv4Arping(QString ifaceName, QString ipAddress, int retryCount=3, int timeout=1, QObject *parent = nullptr);
    ~KyIpv4Arping();

public:
    bool ipv4IsConflict() {
        return m_ipConflict;
    }

    QString getMacAddress() {
        return m_conflictMac;
    }

    int ipv4ConflictCheck();

private:
    void monoGetTime(struct timespec *ts);
    void saveMacAddress(const uint8_t *ptr, size_t len);
    void findBroadcastAddress();
    int checkDevice();
    int checkIfflags(unsigned int ifflags);

    int sendIpv4Packet();
    int ipv4EventLoop();
    int ipv4PacketProcess(unsigned char *buf, ssize_t len, struct sockaddr_ll *from);

    void setConflictFlag(bool isConflict);

private:
    struct in_addr m_srcAddress;
    struct in_addr m_destAddress;
    int m_destAddressFamily;

    struct sockaddr_storage m_me;
    struct sockaddr_storage m_he;
    int  m_ipv4Socket = 0;

    QString m_ifaceName;
    int m_ifindex;
    struct ifaddrs *m_ifa;
    struct ifaddrs *m_ifa0;

    QString m_ipv4Address;
    int     m_retryCount;
    int     m_timeout;

    bool    m_ipConflict = false;
    QString m_conflictMac = nullptr;
};

#endif
