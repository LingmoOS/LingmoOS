/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "lingmoipv6arping.h"

#include <sys/times.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "lingmoarping.h"

KyIpv6Arping::KyIpv6Arping(QString ifaceName, QString ipAddress, int retryCount, int timeout, QObject *parent) : QObject(parent)
{
    m_ifaceName = ifaceName;
    m_ipv6Address = ipAddress;
    m_retryCount = retryCount;
    m_timeoutMs = timeout;

    m_ipv6Conflict = false;
}

KyIpv6Arping::~KyIpv6Arping()
{

}

void KyIpv6Arping::monoGetTime (struct timespec *ts)
{
#ifdef CLOCK_MONOTONIC
    if (clock_gettime (CLOCK_MONOTONIC, ts))
#endif
    {
        static long freq = 0;
        if (freq == 0)
            freq = sysconf (_SC_CLK_TCK);

         struct tms dummy;
         clock_t t = times (&dummy);
         ts->tv_sec = t / freq;
         ts->tv_nsec = (t % freq) * (1000000000 / freq);
    }
}

int KyIpv6Arping::getLocalMacAddress(const char *ifname, unsigned char *addr)
{
# ifdef SIOCGIFHWADDR
    struct ifreq req;
    memset (&req, 0, sizeof (req));

    if (((unsigned)strlen (ifname)) >= (unsigned)IFNAMSIZ) {
        return -1; /* buffer overflow = local root */
    }

    strcpy (req.ifr_name, ifname);

    int fd = socket (AF_INET6, SOCK_DGRAM, 0);
    if (fd == -1) {
        return -1;
    }

    if (ioctl (fd, SIOCGIFHWADDR, &req)) {
        qWarning()<<"[KyIpv6Arping]"<<"get local mac address failed, errno" << errno;
        close (fd);
        return -1;
    }
    close (fd);

    memcpy (addr, req.ifr_hwaddr.sa_data, 6);
    return 0;
# else
    /* No SIOCGIFHWADDR, which seems Linux specific. */
    struct ifaddrs *ifa = NULL;
    struct ifaddrs *ifp = NULL;
    getifaddrs(&ifa);
    ifp = ifa;    /* preserve the address of ifa to free memory later */
    while (ifp != NULL) {
        if (ifp->ifa_addr->sa_family == AF_LINK && strcmp(ifp->ifa_name, ifname) == 0) {
            const struct sockaddr_dl* sdl = (const struct sockaddr_dl*) ifp->ifa_addr;
            memcpy(addr, sdl->sdl_data + sdl->sdl_nlen, 6);
            freeifaddrs(ifa);
            return 0;
        }
        ifp = ifp->ifa_next;
    }
    freeifaddrs(ifa);
    return -1;
# endif
}

int KyIpv6Arping::getIpv6ByName(struct sockaddr_in6 *addr)
{
    struct addrinfo hints, *res;
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_INET6;
    hints.ai_socktype = SOCK_DGRAM; /* dummy */
    hints.ai_flags = 0;

    int val = getaddrinfo (m_ipv6Address.toUtf8(), NULL, &hints, &res);
    if (val) {
        qWarning()<<"[KyIpv6Arping]" << m_ipv6Address <<"get addrinfo failed, errno" << val;
        return -1;
    }

    memcpy (addr, res->ai_addr, sizeof (struct sockaddr_in6));
    freeaddrinfo (res);

    val = if_nametoindex (m_ifaceName.toUtf8());
    if (val == 0) {
        qWarning()<<"[KyIpv6Arping]" <<"if_nametoindex failed, errno" << errno;
        return -1;
    }
    addr->sin6_scope_id = val;

    return 0;
}

int KyIpv6Arping::buildSolicitationPacket(solicit_packet *ns, struct sockaddr_in6 *tgt, const char *ifname)
{
    /* builds ICMPv6 Neighbor Solicitation packet */
    ns->hdr.nd_ns_type = ND_NEIGHBOR_SOLICIT;
    ns->hdr.nd_ns_code = 0;
    ns->hdr.nd_ns_cksum = 0; /* computed by the kernel */
    ns->hdr.nd_ns_reserved = 0;
    memcpy (&ns->hdr.nd_ns_target, &tgt->sin6_addr, 16);

    /* determines actual multicast destination address */
    memcpy (&tgt->sin6_addr.s6_addr, "\xff\x02\x00\x00\x00\x00\x00\x00"
                                     "\x00\x00\x00\x01\xff", 13);

    /* gets our own interface's link-layer address (MAC) */
    if (getLocalMacAddress (m_ifaceName.toUtf8().constData(), ns->hw_addr)) {
        return sizeof (ns->hdr);
     }

    ns->opt.nd_opt_type = ND_OPT_SOURCE_LINKADDR;
    ns->opt.nd_opt_len = 1; /* 8 bytes */
    return sizeof (*ns);
}

void KyIpv6Arping::saveMacAddress (const uint8_t *ptr, size_t len)
{
    int index;
    char macAddress[64] = {0};

    for (index = 0; index < len; index++) {
        snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%02X", ptr[index]);
        if (index != len - 1) {
            snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%s", ":");
        }
    }

    m_conflictMacAddress = QString(macAddress);
    return ;
}

int KyIpv6Arping::parseIpv6Packet(const uint8_t *buf, size_t len, const struct sockaddr_in6 *tgt)
{
    const struct nd_neighbor_advert *na =
        (const struct nd_neighbor_advert *)buf;
    const uint8_t *ptr;

    /* checks if the packet is a Neighbor Advertisement, and
     * if the target IPv6 address is the right one */
    if ((len < sizeof (struct nd_neighbor_advert))
        || (na->nd_na_type != ND_NEIGHBOR_ADVERT)
        || (na->nd_na_code != 0)
        || memcmp (&na->nd_na_target, &tgt->sin6_addr, 16)) {
        return -1;
    }
    len -= sizeof (struct nd_neighbor_advert);

    /* looks for Target Link-layer address option */
    ptr = buf + sizeof (struct nd_neighbor_advert);

    int index;
    char macAddress[64] = {0};
    uint8_t hw_addr[6] = {0};
    getLocalMacAddress(m_ifaceName.toUtf8().constData(), hw_addr);
    for (index = 0; index < 6; index++) {
        snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%02X", hw_addr[index]);
        if (index != 5) {
            snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%s", ":");
        }
    }
    QString localAddr(macAddress);

    while (len >= 8)
    {
        uint16_t optlen;

        optlen = ((uint16_t)(ptr[1])) << 3;
        if (optlen == 0)
            break; /* invalid length */

        if (len < optlen) /* length > remaining bytes */
            break;
        len -= optlen;


        /* skips unrecognized option */
        if (ptr[0] != ND_OPT_TARGET_LINKADDR)
        {
            ptr += optlen;
            continue;
        }

        /* Found! displays link-layer address */
        ptr += 2;
        optlen -= 2;

        saveMacAddress (ptr, optlen);
        if (!localAddr.isEmpty() && getConflictMacAddress() == localAddr) {
            break;
        }
        setIpv6ConflictFlag(true);
        return 0;
    }

    return -1;
}

int KyIpv6Arping::readIpv6Packet(void *buf,
                                    size_t len,
                                    int flags,
                                    struct sockaddr_in6 *addr)
{
    char cbuf[CMSG_SPACE (sizeof (int))];

    struct iovec iov = {
        .iov_base = buf,
        .iov_len = len
    };

    struct msghdr hdr = {
        .msg_name = addr,
        .msg_namelen = sizeof (*addr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = cbuf,
        .msg_controllen = sizeof (cbuf)
    };

    ssize_t val = recvmsg (m_ipv6Socket, &hdr, flags);
    if (val == -1) {
        return val;
    }

    /* ensures the hop limit is 255 */
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR (&hdr);
            cmsg != NULL; cmsg = CMSG_NXTHDR (&hdr, cmsg)) {
        if ((cmsg->cmsg_level == IPPROTO_IPV6)
                && (cmsg->cmsg_type == IPV6_HOPLIMIT)) {
            if (255 != *(int *)CMSG_DATA (cmsg)) {
                // pretend to be a spurious wake-up
                errno = EAGAIN;
                return -1;
            }
        }
    }

    return val;
}

int KyIpv6Arping::reciveAndProcessIpv6Packet(const struct sockaddr_in6 *tgt, unsigned wait_ms)
{
    struct timespec end;
    struct pollfd fds;

    /* computes deadline time */
    monoGetTime (&end);
    div_t d;
    d = div (wait_ms, 1000);
    end.tv_sec = end.tv_sec + d.quot;
    end.tv_nsec = end.tv_nsec + (d.rem * 1000000);

    /* receive loop */
    for (;;)
    {
        /* waits for reply until deadline */
        struct timespec now;
        int val = 0;

        monoGetTime (&now);
        if (end.tv_sec >= now.tv_sec) {
            val = (end.tv_sec - now.tv_sec) * 1000
                + (int)((end.tv_nsec - now.tv_nsec) / 1000000);
            if (val < 0) {
                val = 0;
            }
        }

        fds.fd = m_ipv6Socket;
        fds.events = POLLIN;
        val = poll (&fds, 1, val);
        if (val < 0)
            break;

        if (val == 0)
            return 0;

        /* receives an ICMPv6 packet */
        // TODO: use interface MTU as buffer size
        uint8_t buf[1460];
        struct sockaddr_in6 addr;

        val = readIpv6Packet(buf, sizeof (buf), MSG_DONTWAIT, &addr);
        if (val < 0) {
            if (errno != EAGAIN)
                qWarning()<<"[KyIpv6Arping]"<<"Receiving ICMPv6 packet failed";
            continue;
        }

        /* ensures the response came through the right interface */
        if (addr.sin6_scope_id
            && (addr.sin6_scope_id != tgt->sin6_scope_id))
            continue;

        if (parseIpv6Packet(buf, val, tgt) == 0) {
            return 1 /* = responses */;
        }
    }

    return -1; /* error */
}


int KyIpv6Arping::ipv6ConflictCheck()
{
    struct sockaddr_in6 tgt;
    struct icmp6_filter filter;
    int retry = 0;

    limit_capabilities();

    enable_capability_raw();
    m_ipv6Socket = socket (PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    disable_capability_raw();
    if (m_ipv6Socket < 0) {
        qDebug()<<"[KyIpv6Arping]" <<"create ipv6 socket failed:";
        return -1;
    }

    fcntl (m_ipv6Socket, F_SETFD, FD_CLOEXEC);

    /* set ICMPv6 filter */
    ICMP6_FILTER_SETBLOCKALL (&filter);
    ICMP6_FILTER_SETPASS (ND_NEIGHBOR_ADVERT, &filter);

    enable_capability_raw();
    setsockopt (m_ipv6Socket, IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof (filter));

    int soDontRoute = 1;
    setsockopt (m_ipv6Socket, SOL_SOCKET, SO_DONTROUTE, &soDontRoute, sizeof(soDontRoute));

    /* sets Hop-by-hop limit to 255 */
    int multicastHopLimit = 255;
    setsockopt (m_ipv6Socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                &multicastHopLimit, sizeof (multicastHopLimit));

    int unicastHops = 255;
    setsockopt (m_ipv6Socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                    &unicastHops, sizeof (unicastHops));

    int recvHopLimit = 1;
    setsockopt(m_ipv6Socket, IPPROTO_IPV6, IPV6_RECVHOPLIMIT,
               &recvHopLimit, sizeof (recvHopLimit));

    disable_capability_raw();

    drop_capabilities();

    /* resolves target's IPv6 address */
    int ret = getIpv6ByName(&tgt);
    if (ret < 0) {
        qWarning()<<"[KyIpv6Arping]"<<"get ipv6 by name failed.";
        close (m_ipv6Socket);
        return ret;
    }

    solicit_packet packet;
    struct sockaddr_in6 dst;
    ssize_t plen;

    memcpy (&dst, &tgt, sizeof (dst));
    plen = buildSolicitationPacket(&packet, &dst, m_ifaceName.toUtf8().constData());
    if (plen == -1) {
        qWarning()<<"[KyIpv6Arping]"<<"build solicit packet failed.";
        close (m_ipv6Socket);
        return -1;
    }

    while (retry < m_retryCount) {
        /* sends a Solitication */
        if (sendto (m_ipv6Socket, &packet, plen, MSG_DONTROUTE,
                    (const struct sockaddr *)&dst,
                    sizeof (dst)) != plen) {
            qWarning()<<"[KyIpv6Arping]"<<"Sending ICMPv6 packet failed.";
            close (m_ipv6Socket);
            return -1;
        }

        retry++;

        /* receives an Advertisement */
        ssize_t val = reciveAndProcessIpv6Packet(&tgt, m_timeoutMs);
        if (val > 0) {
            close (m_ipv6Socket);
            return 0;
        } else if (val == 0) {
            continue;
        } else {
            close (m_ipv6Socket);
            return -1;
        }
    }

    close(m_ipv6Socket);

    if (retry == m_retryCount) {
        return 0;
    } else {
        return -2;
    }
}
