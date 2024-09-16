// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NET_INFO_H
#define DISKIO_INFO_H

#include <QSharedPointer>

#ifndef IF_NAMESIZE
#define IF_NAMESIZE 16
#endif

namespace core {
namespace system {

struct net_stat {
    unsigned long long rx_bytes; // received bytes
    unsigned long long tx_bytes; // transmitted bytes
    char iface[IF_NAMESIZE + 1]; // interface name
};

class NetInfo
{
    enum StatIndex { kLastStat = 0, kCurrentStat = 1, kStatCount = kCurrentStat + 1 };

public:
    explicit NetInfo();
    virtual ~NetInfo();

    void update();

    /**
     * @brief recvBps   获取接收速度
     * @return
     */
    qreal recvBps();

    /**
     * @brief sentBps   获取发送速度
     * @return
     */
    qreal sentBps();

    /**
     * @brief totalRecvBytes    总接收流量
     * @return
     */
    qulonglong totalRecvBytes();

    /**
     * @brief totalSentBytes    总发送流量
     * @return
     */
    qulonglong totalSentBytes();

    /**
     * @brief resdNetInfo   读取网络信息
     */
    void resdNetInfo();

private:
    timeval timevalList[kStatCount] = {timeval{0, 0}, timeval{0, 0}};
    QSharedPointer<struct net_stat> m_netStat[kStatCount] {{}, {}};

    qreal m_recvBps = 0;             // 接收速度
    qreal m_sentBps = 0;             // 发送速度
    qulonglong m_totalRecvBytes = 0; // 接收总流量
    qulonglong m_totalSentBytes = 0; // 发送总流量
};

} // namespace system
} // namespace core

#endif // DISKIO_INFO_H
