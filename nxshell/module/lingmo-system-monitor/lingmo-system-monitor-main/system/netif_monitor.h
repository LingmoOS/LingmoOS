// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_MONITOR_H
#define NETIF_MONITOR_H

#include "common/time_period.h"
#include "netif_packet_capture.h"

#include <QObject>
#include <QBasicTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

using namespace common::core;

namespace core {
namespace system {

/**
 * @brief Network interface socket io stat structure
 */
struct sock_io_stat_t {
    ino_t ino; // socket inode
    qulonglong rx_bytes; // received bytes
    qulonglong rx_packets; // received packets
    qulonglong tx_bytes; // sent bytes
    qulonglong tx_packets; // sent packets
};

// socket io stat typedef
using SockIOStat = QSharedPointer<struct sock_io_stat_t>;

class NetifMonitor : public QObject
{
    Q_OBJECT
public:
    explicit NetifMonitor(QObject *parent = nullptr);
    virtual ~NetifMonitor();

    inline void requestQuit()
    {
        m_quitRequested.store(true);
        m_pktqWatcher.wakeAll();
    }

public:
    static NetifMonitor *instance();

    void startNetmonitorJob();

    void handleNetData();
public:
    /**
     * @brief Get socket io stat data with specified inode (thread safe accessor)
     * @param ino Socket inode
     * @param stat Socket io stat data
     * @return Return true if stat data found in stat cache, otherwise return false
     */
    inline bool getSockIOStatByInode(ino_t ino, SockIOStat &stat)
    {
        bool ok = false;

        // lock cache before access
        m_sockIOStatMapLock.lock();

        // check stat data existence
        if (m_sockIOStatMap.contains(ino)) {
            stat = m_sockIOStatMap[ino];
            // remove stat data from cache
            m_sockIOStatMap.remove(ino);
            ok = true;
        }

        m_sockIOStatMapLock.unlock();

        return ok;
    }
    // socket inode to io stat mapping
    QMap<ino_t, SockIOStat> m_sockIOStatMap     {};

private:
    NetifPacketCapture *m_netifCapture;
    // packet monitor thread object
    QThread m_packetMonitorThread;

    // pending packet queue
    PacketPayloadQueue  m_pendingPackets        {};
    // pending packet queue locker
    QMutex              m_pktqLock              {};
    // packet queue watcher
    QWaitCondition      m_pktqWatcher           {};
    // local pending packet queue
    PacketPayloadQueue  m_localPendingPackets   {}; // local cache


    // socket io stat map access locker
    QMutex                  m_sockIOStatMapLock {};

    // packet monitor thread object
    //QThread             m_packetMonitorThread;
    // packet monitor job instace

    // quit atomic test flag
    std::atomic_bool m_quitRequested {false};



    friend void pcap_callback(u_char *, const struct pcap_pkthdr *, const u_char *);
    friend class NetifPacketCapture;
};
}
}

#endif // NETIF_MONITOR_H
