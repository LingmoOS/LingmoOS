// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "net_info.h"
#include "common/common.h"
#include "system/sys_info.h"

#include <QScopedArrayPointer>

using namespace common::error;

namespace core {
namespace system {

#define PROC_PATH_NET       "/proc/net/dev"

NetInfo::NetInfo()
{

}

NetInfo::~NetInfo()
{

}

qreal NetInfo::recvBps()
{
    return m_recvBps;
}

qreal NetInfo::sentBps()
{
    return m_sentBps;
}

qulonglong NetInfo::totalRecvBytes()
{
    return m_totalRecvBytes;
}

qulonglong NetInfo::totalSentBytes()
{
    return m_totalSentBytes;
}

void NetInfo::resdNetInfo()
{
    // 时间间隔
    timevalList[kLastStat] = timevalList[kCurrentStat];
    timevalList[kCurrentStat] = SysInfo::instance()->uptime();

    bool b = false;

    auto statSum = QSharedPointer<struct net_stat>(new net_stat {});
    memset(statSum.data(), 0, sizeof(struct net_stat));
    strncpy(statSum->iface, "(sum)", 6);

    FILE *fp;
    const size_t bsiz = 256;
    QScopedArrayPointer<char> line(new char[bsiz] {});
    int rc;

    if ((fp = fopen(PROC_PATH_NET, "r")) == nullptr) {
        print_errno(errno, QString("open %1 failed").arg(PROC_PATH_NET));
        return;
    }

    while (fgets(line.data(), bsiz, fp)) {
        char *pos, *start;
        start = line.data();
        pos = strchr(line.data(), ':');
        if (!pos)
            continue;

        *pos++ = '\0';

        auto stat = QSharedPointer<struct net_stat>(new net_stat {});
        rc = sscanf(start, "%16s", stat->iface);
        if (rc != 1)
            continue;

        unsigned long long rx_packets; // received packets
        unsigned long long tx_packets; // transmitted packets
        unsigned long long rx_compressed; // number of compressed packets received
        unsigned long long tx_compressed; // number of compressed packets transmitted
        unsigned long long multicast; // number of multicast frames transmitted or received

        //******************1****2********************3****4****5****6************************7**
        rc = sscanf(pos, "%llu %llu %*u %*u %*u %*u %llu %llu %llu %llu %*u %*u %*u %*u %*u %llu",
                    &stat->rx_bytes,
                    &rx_packets,
                    &rx_compressed,
                    &multicast,
                    &stat->tx_bytes,
                    &tx_packets,
                    &tx_compressed);
        if (rc != 7)
            continue;

        statSum->rx_bytes += stat->rx_bytes;
        statSum->tx_bytes += stat->tx_bytes;

        b = true;
    }
    b = !ferror(fp) && b;
    fclose(fp);
    if (!b) {
        print_errno(errno, QString("read %1 failed").arg(PROC_PATH_NET));
    }

    m_netStat[kLastStat] = m_netStat[kCurrentStat];
    m_netStat[kCurrentStat] = statSum;

    qulonglong prxb {}, ptxb{}, crxb {}, ctxb {};
    if (!m_netStat[kCurrentStat].isNull()) {
        crxb = m_netStat[kCurrentStat]->rx_bytes;
        ctxb = m_netStat[kCurrentStat]->tx_bytes;
    }
    if (!m_netStat[kLastStat].isNull()) {
        prxb = m_netStat[kLastStat]->rx_bytes;
        ptxb = m_netStat[kLastStat]->tx_bytes;
    }
    if(m_netStat[kLastStat].isNull()){
        m_recvBps = 1;   // Bps
        m_sentBps = 1;
        return ;
    }
    // receive increment between interval
    auto rxdiff = (crxb > prxb) ? (crxb - prxb) : 0;
    // transfer increment between interval
    auto txdiff = (ctxb > ptxb) ? (ctxb - ptxb) : 0;

    // 计算时间间隔
    timeval cur_time = timevalList[kCurrentStat];
    timeval prev_time = timevalList[kLastStat];
    auto ltime = prev_time.tv_sec + prev_time.tv_usec * 1. / 1000000;
    auto rtime = cur_time.tv_sec + cur_time.tv_usec * 1. / 1000000;
    auto interval = (rtime > ltime) ? (rtime - ltime) : 1;

    // 得出当前速度和总流量大小
    m_totalRecvBytes = crxb;
    m_totalSentBytes = ctxb;
    m_recvBps = rxdiff / interval;   // Bps
    m_sentBps = txdiff / interval;
}

} // namespace system
} // namespace core
