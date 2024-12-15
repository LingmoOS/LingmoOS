// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "netutils.h"

#include <QObject>

#include <pcap.h>
// header for dispatch packet
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>

#define PACKET_DISPATCH_BATCH_COUNT 64 // 执行pcap_dispatch函数单次拆包的个数
#define DISPATCH_JOB_RERUN_TIMEOUT 2 // 拆包任务重新开始时间间隔 /s

// ipv6数据包中使用的协议类型
#define IP6_NEXT_HEADER_HBH 0
#define IP6_NEXT_HEADER_TCP 6
#define IP6_NEXT_HEADER_UDP 17
#define IP6_NEXT_HEADER_EIP6 41
#define IP6_NEXT_HEADER_ROUTING 43
#define IP6_NEXT_HEADER_FRAGMENT 44
#define IP6_NEXT_HEADER_RRSVP 46
#define IP6_NEXT_HEADER_ESP 50 // skip (TODO: ipsec packets)
#define IP6_NEXT_HEADER_AUTH 51 // skip (TODO: ipsec packets)
#define IP6_NEXT_HEADER_ICMP6 58 // skip (TODO: icmpv6 packets)
#define IP6_NEXT_HEADER_NNH 59
#define IP6_NEXT_HEADER_DESOPT 60

// 初始化数据包的各项信息：数据包大小packetSize、网络类型etherType、协议类型protocol、源地址srcIpAddr、源端口srcPort
// 目标地址dstIpAddr、目标端口dstPort
struct NetPkgInfo {
    ulong packetSize {};
    long timeStampS {};
    long timeStampUS {};
    uint etherType {};
    // 协议
    uint protocol {};

    in_addr_t srcIpAddr {};
    // ipv6 address
    uint8_t srcIp6Addr[16] {};
    uint16_t srcPort {};
    in_addr_t dstIpAddr {};
    // ipv6 address
    uint8_t dstIp6Addr[16] {};
    uint16_t dstPort {};
};
Q_DECLARE_METATYPE(NetPkgInfo);

class QMutex;

class DispatchNetPkgJob : public QObject
{
    Q_OBJECT
public:
    DispatchNetPkgJob();
    ~DispatchNetPkgJob();

    void stop();
    void start();
    void setCurrentNetDev(const QString &dev);
    void setUpdateNetDevFlag(const bool update);

public Q_SLOTS:
    // 拆解网络数据包
    void DispatchPackets();

Q_SIGNALS:
    // 通知添加网络数据包
    void notifyAppendNetPkgInfo(const NetPkgInfo &info);

private:
    // 更新网络设备
    void updateNetDev();

private:
    QString m_currentNetDev;
    bool m_needUpdateNetDev;
    pcap *m_pcapHandle;
    bool m_quited;
    QMutex *m_mutexlock;
};
