// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_PACKET_CAPTURE_H
#define NETIF_PACKET_CAPTURE_H

#include <QObject>
#include "packet.h"
#include <QTimer>
#include <QMap>
#include <unistd.h>


namespace core {
namespace system {
class NetifMonitor;
class NetifPacketCapture : public QObject
{
    Q_OBJECT
public:
    explicit NetifPacketCapture(NetifMonitor *netInfmontor, QObject *parent = nullptr);
    inline void requestQuit()
    {
        m_quitRequested.store(true);
    }
    /**
     * @brief Packet dispatch handler
     */
    void dispatchPackets();
    pcap_t  *getHandle() { return m_handle;}


protected:

    /**
     * @brief 获取当前网卡名
     */
    bool getCurrentDevName();
    /**
     * @brief 判断当前网卡是否被分配可用网络IP(IPv4)
     */
    bool hasDevIP();
signals:

public slots:
    /**
     * @brief Start monitor job
     */
    void startNetifMonitorJob();

    /**
     * @brief 判断使用网卡是否变更,变更则重启startNetifMonitorJob槽函数;
     */
    void whetherDevChanged();

private:

    /**
     * @brief Refresh network interface address hash cache
     */
    void refreshIfAddrsHashCache();


private:
    // socket io stat cache
    SockStatMap     m_sockStats {};
    // network interface name hash cache
    QMap<uint64_t, int> m_ifaddrsHashCache; // key: hash code generated from ifaddr; value: not used

    // network interface monitor
    NetifMonitor       *m_netifMonitor         {};
    // pcap handler instance
    pcap_t             *m_handle               {};
    // local pending packet queue
    PacketPayloadQueue  m_localPendingPackets  {};

    // request quit atomic flag
    std::atomic_bool m_quitRequested {false};
    bool go {false};
    // packet dispatch timer
    QTimer *m_timer {};

    //是否变更网卡
    bool m_changedDev {false}; 
    //当前使用网卡名
    QString m_devName {};
    //当前系统所有网卡设备链表
    pcap_if_t *m_alldevs {};
    //判断网卡是否变更定时器
    QTimer *m_timerChangeDev {};
    friend void pcap_callback(u_char *, const struct pcap_pkthdr *, const u_char *);

};

} // namespace system
} // namespace core

#endif // NETIF_PACKET_CAPTURE_H
