// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dispatchnetpkgjob.h"

#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QThread>

void pcap_callback(u_char *context, const struct pcap_pkthdr *hdr, const u_char *packet);

DispatchNetPkgJob::DispatchNetPkgJob()
    : m_needUpdateNetDev(false)
    , m_pcapHandle(nullptr)
    , m_quited(false)
    , m_mutexlock(new QMutex)
{
    // registerMetaType
    qRegisterMetaType<NetPkgInfo>("NetPkgInfo");
}

DispatchNetPkgJob::~DispatchNetPkgJob()
{
    pcap_close(m_pcapHandle);
    delete (m_mutexlock);
}

void DispatchNetPkgJob::stop()
{
    m_quited = true;
}

void DispatchNetPkgJob::start()
{
    m_quited = false;
    m_mutexlock->unlock();
}

void DispatchNetPkgJob::setCurrentNetDev(const QString &dev)
{
    m_currentNetDev = dev;
}

void DispatchNetPkgJob::setUpdateNetDevFlag(const bool update)
{
    m_needUpdateNetDev = update;
}

// 拆解网络数据包
void DispatchNetPkgJob::DispatchPackets()
{
    while (1) {
        m_mutexlock->lock();
        if (m_quited) {
            break;
        }

        if (m_needUpdateNetDev) {
            updateNetDev();
        }
        m_mutexlock->unlock();

        if (!m_pcapHandle) {
            continue;
        }

        int ret = pcap_dispatch(m_pcapHandle, PACKET_DISPATCH_BATCH_COUNT,
                                pcap_callback, reinterpret_cast<u_char *>(this));
        if (ret == 0) {
            // no packets are available, idle this loop for a fraction second
            qDebug() << Q_FUNC_INFO << "no packets are available" << pcap_geterr(m_pcapHandle);
            // 失败后，删除失效pcap,等待有效pcap
            pcap_close(m_pcapHandle);
            m_pcapHandle = nullptr;
            QThread::currentThread()->msleep(DISPATCH_JOB_RERUN_TIMEOUT * 1000);
        } else if (ret == -1) {
            qDebug() << Q_FUNC_INFO << "pcap_dispatch failed: " << pcap_geterr(m_pcapHandle);
            // 失败后，删除失效pcap,等待有效pcap
            pcap_close(m_pcapHandle);
            m_pcapHandle = nullptr;
            QThread::currentThread()->msleep(DISPATCH_JOB_RERUN_TIMEOUT * 1000);
        } else if (ret == -2) {
            // breakloop requested (can only happen inside the callback function)
            qDebug() << Q_FUNC_INFO << pcap_geterr(m_pcapHandle);
            // 失败后，删除失效pcap,等待有效pcap
            pcap_close(m_pcapHandle);
            m_pcapHandle = nullptr;
            QThread::currentThread()->msleep(DISPATCH_JOB_RERUN_TIMEOUT * 1000);
        }

        QThread::currentThread()->msleep(10);
    }
}

// 更新网络设备
void DispatchNetPkgJob::updateNetDev()
{
    char errbuf[PCAP_ERRBUF_SIZE] {};
    if (m_pcapHandle) {
        pcap_close(m_pcapHandle);
    }

    // reinitialize
    m_pcapHandle = pcap_open_live(m_currentNetDev.toStdString().data(), 65535, 1, 0, errbuf);

    m_needUpdateNetDev = false;
}

void pcap_callback(u_char *context, const struct pcap_pkthdr *pkt_hdr, const u_char *packet)
{
    // packet payload calc
    if (!context)
        return;

    DispatchNetPkgJob *dispatchNewPkgJob = reinterpret_cast<DispatchNetPkgJob *>(context);

    // 初始化数据包的各项信息：数据包大小packetSize、网络类型etherType、协议类型protocol、源地址srcIpAddr、源端口srcPort
    // 目标地址dstIpAddr、目标端口dstPort
    NetPkgInfo netPkgInfo;

    // time stamp
    netPkgInfo.timeStampS = pkt_hdr->ts.tv_sec;
    netPkgInfo.timeStampUS = pkt_hdr->ts.tv_usec;

    // 数据包大小
    netPkgInfo.packetSize = pkt_hdr->len;

    // 分析数据包
    // 数据包头部
    const struct ether_header *eth_hdr = reinterpret_cast<const struct ether_header *>(packet);
    netPkgInfo.etherType = ntohs(eth_hdr->ether_type);

    // 以太网数据头部长度
    ulong eth_hdr_len = sizeof(struct ether_header);

    // ip头部长度
    ulong ip_hdr_len {};

    // 数据包头
    const u_char *payloadHdr {};

    // 如果数据类型为ipv4
    if (netPkgInfo.etherType == ETHERTYPE_IP) {
        const ip *ip_hdr = reinterpret_cast<const struct ip *>(packet + eth_hdr_len);
        // ip header length
        ip_hdr_len = ulong((ip_hdr->ip_hl & 0x0f) * 4);
        // ip payload
        payloadHdr = packet + eth_hdr_len + ip_hdr_len;

        // truncated packets
        if (pkt_hdr->caplen <= eth_hdr_len + ip_hdr_len) {
            return;
        }

        // protocol
        netPkgInfo.protocol = ip_hdr->ip_p;
    } else if (netPkgInfo.etherType == ETHERTYPE_IPV6) {
        const ip6_hdr *ip6_hdr = reinterpret_cast<const struct ip6_hdr *>(packet + eth_hdr_len);

        // next header field in ip6 header
        uint8_t nhtype = ip6_hdr->ip6_nxt;
        // find upper-layer protocol header (the last extension header)
        payloadHdr = packet + eth_hdr_len + sizeof(struct ip6_hdr);
        bool stop {false};
        while (!stop) {
            switch (nhtype) {
            case IP6_NEXT_HEADER_HBH: {
                // Hop-by-Hop Options Header
                auto *hbh = reinterpret_cast<const struct ip6_hbh *>(payloadHdr);
                payloadHdr = payloadHdr + hbh->ip6h_len * 8 + 8;
                nhtype = hbh->ip6h_nxt;
                break;
            }
            case IP6_NEXT_HEADER_TCP: { // UPPER-LAYER protocol header
                // TCP
                netPkgInfo.protocol = IPPROTO_TCP;
                stop = true;
                break;
            }
            case IP6_NEXT_HEADER_UDP: { // UPPER-LAYER protocol header
                // UDP
                netPkgInfo.protocol = IPPROTO_UDP;
                stop = true;
                break;
            }
            case IP6_NEXT_HEADER_EIP6: {
                // Encapsulated IPv6 Header: IPv6 Tunneling
                return;
            }
            case IP6_NEXT_HEADER_ROUTING: {
                // Routing Header
                auto *rh = reinterpret_cast<const struct ip6_rthdr *>(payloadHdr);
                payloadHdr = payloadHdr + rh->ip6r_len * 8 + 8;
                nhtype = rh->ip6r_nxt;
                break;
            }
            case IP6_NEXT_HEADER_FRAGMENT: {
                // Fragment Header
                auto *fh = reinterpret_cast<const struct ip6_frag *>(payloadHdr);
                payloadHdr = payloadHdr + sizeof(struct ip6_frag);
                nhtype = fh->ip6f_nxt;
                break;
            }
            case IP6_NEXT_HEADER_RRSVP: {
                // Resource ReSerVation Protocol
                return;
            }
            case IP6_NEXT_HEADER_ESP: {
                // Encapsulating Security Payload: ignored for now
                return;
            }
            case IP6_NEXT_HEADER_AUTH: {
                // Authentication Header: ignored for now
                return;
            }
            case IP6_NEXT_HEADER_ICMP6: { // UPPER-LAYER protocol header
                // ICMPv6: ignored for now
                return;
            }
            case IP6_NEXT_HEADER_NNH: {
                // No next header: empty payload
                return;
            }
            case IP6_NEXT_HEADER_DESOPT: {
                // Destination Options Header
                auto *dh = reinterpret_cast<const struct ip6_dest *>(payloadHdr);
                payloadHdr = payloadHdr + dh->ip6d_len * 8 + 8;
                nhtype = dh->ip6d_nxt;
                break;
            }
            default:
                // unknown next header type found, stop parsing
                return;
            } // !switch
        } // !while
    } else {
        // ignore non ip4 & ip6 packets
        return;
    }

    // 根据协议获取端口和负载大小
    if (netPkgInfo.protocol == IPPROTO_TCP) {
        // tcp负载头
        auto *tcp_hdr = reinterpret_cast<const struct tcphdr *>(payloadHdr);
        auto tcp_hdr_len = ulong(((tcp_hdr->th_off & 0xf0) >> 4) * 4);
        // no payload data
        if (pkt_hdr->caplen <= eth_hdr_len + ip_hdr_len + tcp_hdr_len) {
            return;
        }
        // 源地址端口号
        netPkgInfo.srcPort = ntohs(tcp_hdr->th_sport);
        // 目标地址端口号
        netPkgInfo.dstPort = ntohs(tcp_hdr->th_dport);

    } else if (netPkgInfo.protocol == IPPROTO_UDP) {
        const udphdr *udp_hdr = reinterpret_cast<const struct udphdr *>(payloadHdr);
        ulong udp_len = ulong(ntohs(udp_hdr->uh_ulen));
        ulong udp_hdr_len = sizeof(struct udphdr);
        ulong udpRealLen {};
        if (udp_len == 0) {
            // rfc#2675:
            // the length field is set to zero if the length of the UDP header plus
            // UDP data is greater than 65,535
            udpRealLen = udp_hdr_len;
        } else if (udp_len <= udp_hdr_len) {
            udpRealLen = udp_len;
        } else {
            udpRealLen = udp_hdr_len;
        }
        // truncated, no payload data
        if (pkt_hdr->caplen <= eth_hdr_len + ip_hdr_len + udpRealLen) {
            return;
        }
        // 源地址端口号
        netPkgInfo.srcPort = ntohs(udp_hdr->uh_sport);
        // 目标地址端口号
        netPkgInfo.dstPort = ntohs(udp_hdr->uh_dport);
    } else {
        // unexpected case, unknown proto type
        return;
    }

    Q_EMIT dispatchNewPkgJob->notifyAppendNetPkgInfo(netPkgInfo);
}
