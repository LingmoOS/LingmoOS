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
#ifndef LINGMOIPV6ARPING_H
#define LINGMOIPV6ARPING_H

//#include <gettext.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* div() */
#include <inttypes.h> /* uint8_t */
#include <limits.h> /* UINT_MAX */
#include <locale.h>
#include <stdbool.h>

#include <errno.h> /* EMFILE */
#include <sys/types.h>
#include <unistd.h> /* close() */
#include <time.h> /* clock_gettime() */
#include <poll.h> /* poll() */
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>

//#include "gettime.h"

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include <netdb.h> /* getaddrinfo() */
#include <arpa/inet.h> /* inet_ntop() */
#include <net/if.h> /* if_nametoindex() */

#include <netinet/in.h>
#include <netinet/icmp6.h>

#ifndef IPV6_RECVHOPLIMIT
/* Using obsolete RFC 2292 instead of RFC 3542 */
# define IPV6_RECVHOPLIMIT IPV6_HOPLIMIT
#endif

/* BSD-like systems define ND_RA_FLAG_HA instead of ND_RA_FLAG_HOME_AGENT */
#ifndef ND_RA_FLAG_HOME_AGENT
# ifdef ND_RA_FLAG_HA
#  define ND_RA_FLAG_HOME_AGENT ND_RA_FLAG_HA
# endif
#endif

#ifndef AI_IDN
# define AI_IDN 0
#endif


#include <QString>
#include <QDebug>
#include <QObject>

typedef struct
{
    struct nd_neighbor_solicit hdr;
    struct nd_opt_hdr opt;
    uint8_t hw_addr[6];
} solicit_packet;

class KyIpv6Arping: public QObject
{
    Q_OBJECT
public:
    explicit KyIpv6Arping(QString ifaceName, QString ipAddress, int retryCount=3, int timeout=1000, QObject *parent = nullptr);
    ~KyIpv6Arping();

public:
    bool ipv6IsConflict() {
        return m_ipv6Conflict;
    }

    void setIpv6ConflictFlag(bool conflict) {
        m_ipv6Conflict = conflict;
    }

    QString getConflictMacAddress() {
        return m_conflictMacAddress;
    }

    int ipv6ConflictCheck();

private:
    void monoGetTime (struct timespec *ts);
    void saveMacAddress (const uint8_t *ptr, size_t len);
    int  getLocalMacAddress(const char *ifname, unsigned char *addr);

    int getIpv6ByName(struct sockaddr_in6 *addr);
    int buildSolicitationPacket(solicit_packet *ns, struct sockaddr_in6 *tgt, const char *ifname);
    int parseIpv6Packet(const uint8_t *buf, size_t len, const struct sockaddr_in6 *tgt);
    int readIpv6Packet(void *buf, size_t len, int flags, struct sockaddr_in6 *addr);
    int reciveAndProcessIpv6Packet(const struct sockaddr_in6 *tgt, unsigned wait_ms);

private:
    int  m_ipv6Socket = 0;

    QString m_ifaceName;
    QString m_ipv6Address;
    int     m_retryCount;
    int     m_timeoutMs;

    bool    m_ipv6Conflict = false;
    QString m_conflictMacAddress = nullptr;
};

#endif // LINGMOIPV6ARPING_H
