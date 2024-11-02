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
#include "lingmoipv4arping.h"

#include <sys/times.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define FINAL_PACKS 2

__attribute__((const)) static inline size_t sll_len(const size_t halen)
{
    struct sockaddr_ll unused;
    const size_t len = offsetof(struct sockaddr_ll, sll_addr) + halen;

    if (len < sizeof(unused))
        return sizeof(unused);

    return len;
}


KyIpv4Arping::KyIpv4Arping(QString ifaceName, QString ipAddress, int retryCount, int timeout, QObject *parent) : QObject(parent)
{
    m_ifaceName = ifaceName;
    m_ipv4Address = ipAddress;
    m_retryCount = retryCount;
    m_timeout = timeout;

    memset(&m_srcAddress, 0, sizeof(m_srcAddress));
}

KyIpv4Arping::~KyIpv4Arping()
{

}

void KyIpv4Arping::monoGetTime (struct timespec *ts)
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

void KyIpv4Arping::saveMacAddress(const uint8_t *ptr, size_t len)
{
    int index;
    char macAddress[64] = {0};

    for (index = 0; index < len; index++) {
        snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%02X", ptr[index]);
        if (index != len - 1) {
            snprintf(&macAddress[strlen(macAddress)], sizeof(macAddress) - strlen(macAddress), "%s", ":");
        }
    }

    m_conflictMac = QString(macAddress);
    return ;
}

void KyIpv4Arping::setConflictFlag(bool isConflict)
{
    m_ipConflict = isConflict;
}

int KyIpv4Arping::sendIpv4Packet()
{
    int err;
    unsigned char buf[256];
    struct arphdr *ah = (struct arphdr *)buf;
    unsigned char *p = (unsigned char *)(ah + 1);
    struct sockaddr_ll *p_me = (struct sockaddr_ll *)&(m_me);
    struct sockaddr_ll *p_he = (struct sockaddr_ll *)&(m_he);

    ah->ar_hrd = htons(p_me->sll_hatype);
    if (ah->ar_hrd == htons(ARPHRD_FDDI))
        ah->ar_hrd = htons(ARPHRD_ETHER);
    ah->ar_pro = htons(ETH_P_IP);
    ah->ar_hln = p_me->sll_halen;
    ah->ar_pln = 4;
    ah->ar_op  = htons(ARPOP_REQUEST);

    memcpy(p, &p_me->sll_addr, ah->ar_hln);
    p += p_me->sll_halen;

    qWarning()<<"[KyIpv4Arping]" <<"m_src address:" << inet_ntoa(m_srcAddress);
    memcpy(p, &m_srcAddress, 4);
    p += 4;

    memcpy(p, &p_he->sll_addr, ah->ar_hln);
    p += ah->ar_hln;

    memcpy(p, &m_destAddress, 4);
    p += 4;

    err = sendto(m_ipv4Socket, buf, p - buf, 0, (struct sockaddr *)p_he, sll_len(ah->ar_hln));

    return err;
}

int KyIpv4Arping::ipv4PacketProcess(unsigned char *buf, ssize_t len, struct sockaddr_ll *from)
{
   struct arphdr *ah = (struct arphdr *)buf;
   unsigned char *p = (unsigned char *)(ah + 1);
   struct in_addr src_ip, dst_ip;

   /* Filter out wild packets */
   if (from->sll_pkttype != PACKET_HOST &&
       from->sll_pkttype != PACKET_BROADCAST &&
       from->sll_pkttype != PACKET_MULTICAST)
       return 0;

   /* Only these types are recognised */
   if (ah->ar_op != htons(ARPOP_REQUEST) &&
       ah->ar_op != htons(ARPOP_REPLY))
       return 0;

   /* ARPHRD check and this darned FDDI hack here :-( */
   if (ah->ar_hrd != htons(from->sll_hatype) &&
       (from->sll_hatype != ARPHRD_FDDI || ah->ar_hrd != htons(ARPHRD_ETHER)))
       return 0;

   /* Protocol must be IP. */
   if (ah->ar_pro != htons(ETH_P_IP))
       return 0;
   if (ah->ar_pln != 4)
       return 0;
   if (ah->ar_hln != ((struct sockaddr_ll *)&m_me)->sll_halen)
       return 0;

   if (len < (ssize_t) sizeof(*ah) + 2 * (4 + ah->ar_hln))
       return 0;

   memcpy(&src_ip, p + ah->ar_hln, 4);
   memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);
   /*
    * DAD packet was:
    * src_ip = 0 (or some src)
    * src_hw = ME
    * dst_ip = tested address
    * dst_hw = <unspec>
    *
    * We fail, if receive request/reply with:
    * src_ip = tested_address
    * src_hw != ME
    * if src_ip in request was not zero, check
    * also that it matches to dst_ip, otherwise
    * dst_ip/dst_hw do not matter.
    */
    if (src_ip.s_addr != m_destAddress.s_addr)
       return 0;

    if (memcmp(p, ((struct sockaddr_ll *)&m_me)->sll_addr,
          ((struct sockaddr_ll *)&m_me)->sll_halen) == 0)
       return 0;

    saveMacAddress(p, ah->ar_hln);
    setConflictFlag(true);

    return FINAL_PACKS;
}

int KyIpv4Arping::ipv4EventLoop()
{
    int rc = -1;
    int ret;
    int index;
    int exit_loop = 0;
    ssize_t bytes;

    int tfd;
    struct pollfd pfds[POLLFD_COUNT];

    struct itimerspec timerfd_vals;
    timerfd_vals.it_interval.tv_sec = m_timeout,
    timerfd_vals.it_interval.tv_nsec = 0,
    timerfd_vals.it_value.tv_sec = m_timeout,
    timerfd_vals.it_value.tv_nsec = 0;

    uint64_t exp;
    uint64_t total_expires = 1;

    unsigned char packet[4096];
    struct sockaddr_storage from = { 0 };
    socklen_t addr_len = sizeof(from);

    /* timerfd */
    tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (tfd == -1) {
        qWarning()<<"[KyIpv4Arping]" << "timerfd_create failed, errno" << errno;
        return -1;
    }

    if (timerfd_settime(tfd, 0, &timerfd_vals, NULL)) {
        close(tfd);
        qWarning()<<"[KyIpv4Arping]" << "timerfd_settime failed, errno" << errno;
        return -1;
    }

    pfds[POLLFD_TIMER].fd = tfd;
    pfds[POLLFD_TIMER].events = POLLIN | POLLERR | POLLHUP;

    /* socket */
    pfds[POLLFD_SOCKET].fd = m_ipv4Socket;
    pfds[POLLFD_SOCKET].events = POLLIN | POLLERR | POLLHUP;
    sendIpv4Packet();

    while (!exit_loop) {
        ret = poll(pfds, POLLFD_COUNT, -1);
        if (ret <= 0) {
            if (errno == EAGAIN) {
                continue;
            }

            if (errno) {
                qWarning()<<"[KyIpv4Arping]" <<"poll failed, errno" << errno;
            }

            exit_loop = 1;
            continue;
        }

        for (index = 0; index < POLLFD_COUNT; index++) {
            if (pfds[index].revents == 0) {
                continue;
            }

            switch (index) {
            case POLLFD_TIMER:
                bytes = read(tfd, &exp, sizeof(uint64_t));
                if (bytes != sizeof(uint64_t)) {
                    qWarning() <<"[KyIpv4Arping]" << "could not read timerfd";
                    continue;
                }

                total_expires += exp;
                if (0 < m_retryCount && (uint64_t)m_retryCount < total_expires) {
                    exit_loop = 1;
                    rc = 0;
                    continue;
                }

                sendIpv4Packet();
                break;
            case POLLFD_SOCKET:
                bytes = recvfrom(m_ipv4Socket, packet, sizeof(packet), 0,
                                        (struct sockaddr *)&from, &addr_len);
                if (bytes < 0) {
                    qWarning()<<"[KyIpv4Arping]" << "recvfrom function failed, errno" << errno;
                    continue;
                }
                if (ipv4PacketProcess(packet, bytes, (struct sockaddr_ll *)&from) == FINAL_PACKS) {
                    exit_loop = 1;
                    rc = 0;
                }
                break;
            default:
                qWarning()<<"[KyIpv4Arping]" <<"the fd index is undefine" << index;
                break;
            }
        }
    }

    close(tfd);
    freeifaddrs(m_ifa0);

    return rc;
}

int KyIpv4Arping::checkIfflags(unsigned int ifflags)
{
    if (!(ifflags & IFF_UP)) {
        qWarning()<<"[KyIpv4Arping]" <<"the iface" << m_ifaceName <<" is down.";
        return -1;
    }

    if (ifflags & (IFF_NOARP | IFF_LOOPBACK)) {
        qWarning()<<"[KyIpv4Arping]" << "Interface" << m_ifaceName << "is not ARPable.";
        return -1;
    }

    return 0;
}


int KyIpv4Arping::checkDevice()
{
    int rc;
    struct ifaddrs *ifa;
    int n = 0;

    rc = getifaddrs(&m_ifa0);
    if (rc < 0) {
        qWarning()<<"[KyIpv4Arping]" <<"get iface address failed, erron"<< errno;
        return -errno;
    }

    for (ifa = m_ifa0; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        if (ifa->ifa_addr->sa_family != AF_PACKET) {
            continue;
        }

        if (!m_ifaceName.isEmpty()
                && (NULL != ifa->ifa_name)
                && strcmp(ifa->ifa_name, m_ifaceName.toUtf8())) {
            continue;
        }

        if (checkIfflags(ifa->ifa_flags) < 0) {
            continue;
        }

        if (!((struct sockaddr_ll *)ifa->ifa_addr)->sll_halen) {
            continue;
        }

        if (!ifa->ifa_broadaddr) {
            continue;
        }

        m_ifa = ifa;

        if (n++)
            break;
    }

    if (n == 1 && m_ifa) {
        m_ifindex = if_nametoindex(m_ifa->ifa_name);
        if (!m_ifindex) {
            qWarning()<<"[KyIpv4Arping]" <<"if_nametoindex is invalid";
            freeifaddrs(m_ifa0);
            return -1;
        }
        m_ifaceName = m_ifa->ifa_name;

        return 0;
    }

    freeifaddrs(m_ifa0);
    return -1;
}

void KyIpv4Arping::findBroadcastAddress()
{
    struct sockaddr_ll *he = (struct sockaddr_ll *)&(m_he);

    if (m_ifa) {
        struct sockaddr_ll *sll =
                    (struct sockaddr_ll *)m_ifa->ifa_broadaddr;

        if (sll->sll_halen == he->sll_halen) {
            memcpy(he->sll_addr, sll->sll_addr, he->sll_halen);
            return;
        }
    }

    qWarning()<<"[KyIpv4Arping]" <<"using default broadcast address.";

    memset(he->sll_addr, -1, he->sll_halen);

    return;
}

int KyIpv4Arping::ipv4ConflictCheck()
{
    limit_capabilities();

    int ret = checkDevice();
    if (ret < 0) {
        qWarning()<<"[KyIpv4Arping]"<<"the device is invalid" << m_ifaceName;
        return -1;
    }

    enable_capability_raw();
    m_ipv4Socket = socket(PF_PACKET, SOCK_DGRAM, 0);
    disable_capability_raw();
    if (m_ipv4Socket < 0) {
        qWarning()<<"[KyIpv4Arping]" << "create ipv4 socket failed, errno" << errno;
        return -1;
    }

    inet_aton(m_ipv4Address.toUtf8(), &m_destAddress);
    m_destAddressFamily = AF_INET;

    ((struct sockaddr_ll *)&m_me)->sll_family = AF_PACKET;
    ((struct sockaddr_ll *)&m_me)->sll_ifindex = m_ifindex;
    ((struct sockaddr_ll *)&m_me)->sll_protocol = htons(ETH_P_ARP);

    ret = bind(m_ipv4Socket, (struct sockaddr *)&m_me, sizeof(m_me));
    if (ret < 0) {
        qWarning()<<"[KyIpv4Arping]" <<"ipv4 socket bind failed, errno"<< errno;
        close(m_ipv4Socket);
        return -1;
    }

    socklen_t alen = sizeof(m_me);
    ret = getsockname(m_ipv4Socket, (struct sockaddr *)&m_me, &alen);
    if (ret < 0) {
        qWarning()<<"[KyIpv4Arping]" <<"ipv4 get socket name failed, errno" << errno;
        close(m_ipv4Socket);
        return -1;
    }

    if (((struct sockaddr_ll *)&m_me)->sll_halen == 0) {
        qWarning()<<"[KyIpv4Arping]" <<"the iface"<< m_ifaceName <<" is not suitable for arp";
        close(m_ipv4Socket);
        return -1;
    }

    m_he = m_me;

    findBroadcastAddress();

    drop_capabilities();

    ret = ipv4EventLoop();
    close(m_ipv4Socket);

    return ret;
}
