// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netif_packet_capture.h"
#include "system/netif_monitor.h"
#include "system/packet.h"
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <pcap.h>
#include <sys/socket.h>
#include "system/netif_packet_parser.h"
#include <pcap/pcap.h>

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include <QDebug>

using namespace core::system;

/***************************************STUB begin*********************************************/

size_t stub_strlen()
{
    return IFNAMSIZ+1;
}

void stub_whetherDevChanged()
{
    return;
}

int stub_ioctl (int num, unsigned long int __request, ...)
{
    return -1;
}

char* stub_inet_ntoa(struct in_addr __in)
{
    qInfo()<<"stub_inet_ntoa";
    return nullptr;
}

int stub_pcap_findalldevs(pcap_if_t **, char *)
{
    return 1;
}

int stub_socket (int __domain, int __type, int __protocol)
{
    return -1;
}

bool stub_getCurrentDevName_true()
{
    return true;
}

bool stub_getCurrentDevName_false()
{
    return false;
}

bool stub_hasDevIP_false()
{
    return false;
}

bool stub_hasDevIP_true()
{
    return true;
}

bool stub_parsePacket_false(const pcap_pkthdr *pkt_hdr,
                                    const u_char *packet,
                                    PacketPayload &payload)
{
    return false;
}

pcap_t	*stub_pcap_create(const char *, char *)
{
    return nullptr;
}

pcap_t	*stub_pcap_create_false(const char *, char *)
{
    return nullptr;
}

int	stub_localPendingPackets_64()
{
    return 65;
}
int	stub_localPendingPackets_255()
{
    return 255;
}

/***************************************STUB end**********************************************/

class UT_NetifPacketCapture: public ::testing::Test
{
public:
    UT_NetifPacketCapture() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        NetifMonitor *netif = new NetifMonitor;;
        m_tester = new NetifPacketCapture(netif);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifPacketCapture *m_tester;
};

TEST_F(UT_NetifPacketCapture, initTest)
{
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_01)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    m_tester->m_devName = nullptr;
    bool ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_02)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    m_tester->m_devName = "123456789";
    auto ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_03)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    auto ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_04)
{
    Stub stub;
    stub.set(ioctl, stub_ioctl);
    auto ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_05)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);

    stub.set(inet_ntoa, stub_inet_ntoa);
    auto ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_hasDevIP_06)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);

    stub.set(inet_ntoa, stub_inet_ntoa);
    m_tester->m_devName = "12345678901234567";
    auto ok = m_tester->hasDevIP();
    EXPECT_EQ(ok, false);
}

TEST_F(UT_NetifPacketCapture, test_getCurrentDevName_01)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    stub.set(socket, stub_socket);
    auto ok = m_tester->getCurrentDevName();
}

TEST_F(UT_NetifPacketCapture, test_getCurrentDevName_02)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    stub.set(pcap_findalldevs, stub_pcap_findalldevs);
    auto ok = m_tester->getCurrentDevName();
}

TEST_F(UT_NetifPacketCapture, test_getCurrentDevName_03)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    stub.set(pcap_findalldevs, stub_pcap_findalldevs);
    auto ok = m_tester->getCurrentDevName();
}

//TEST_F(UT_NetifPacketCapture, test_getCurrentDevName_06)
//{
//    Stub stub;
//    auto ok = m_tester->getCurrentDevName();
//    EXPECT_EQ(ok, true);
//}

TEST_F(UT_NetifPacketCapture, test_whetherDevChanged_01)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    m_tester->m_devName = nullptr;
    m_tester->whetherDevChanged();
}

TEST_F(UT_NetifPacketCapture, test_whetherDevChanged_02)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, hasDevIP), stub_hasDevIP_false);
    stub.set(ADDR(NetifPacketCapture, getCurrentDevName), stub_getCurrentDevName_false);
    m_tester->whetherDevChanged();
    EXPECT_TRUE(m_tester->m_changedDev);
}

TEST_F(UT_NetifPacketCapture, test_whetherDevChanged_03)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, hasDevIP), stub_hasDevIP_false);
    stub.set(ADDR(NetifPacketCapture, getCurrentDevName), stub_getCurrentDevName_true);
    m_tester->whetherDevChanged();
    EXPECT_TRUE(m_tester->m_changedDev);
}

TEST_F(UT_NetifPacketCapture, test_whetherDevChanged_04)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, hasDevIP), stub_hasDevIP_true);
    m_tester->whetherDevChanged();
    EXPECT_TRUE(m_tester->m_changedDev);
}

TEST_F(UT_NetifPacketCapture, test_startNetifMonitorJob_01)
{
    Stub stub;
    stub.set(ADDR(NetifPacketCapture, whetherDevChanged), stub_whetherDevChanged);
    m_tester->m_devName = nullptr;
    m_tester->startNetifMonitorJob();
}


TEST_F(UT_NetifPacketCapture, test_startNetifMonitorJob_02)
{
    Stub stub;
    stub.set(pcap_create, stub_pcap_create);
    m_tester->startNetifMonitorJob();
}

TEST_F(UT_NetifPacketCapture, test_startNetifMonitorJob_03)
{
    Stub stub;
    m_tester->startNetifMonitorJob();
}

TEST_F(UT_NetifPacketCapture, test_startNetifMonitorJob_04)
{
    Stub stub;
    stub.set(pcap_create, stub_pcap_create);
    m_tester->startNetifMonitorJob();
}

TEST_F(UT_NetifPacketCapture, test_dispatchPackets_01)
{
    m_tester->go = true;
    m_tester->m_devName = nullptr;
    m_tester->dispatchPackets();
}

TEST_F(UT_NetifPacketCapture, test_dispatchPackets_02)
{
    m_tester->go = true;
    m_tester->m_devName = nullptr;
    m_tester->dispatchPackets();
}

TEST_F(UT_NetifPacketCapture, test_dispatchPackets_03)
{
    m_tester->go = true;
    m_tester->m_devName = nullptr;
    Stub stub;
    stub.set(ADDR(PacketPayloadQueue, size), stub_localPendingPackets_64);
    m_tester->dispatchPackets();
}
TEST_F(UT_NetifPacketCapture, test_dispatchPackets_04)
{
    m_tester->go = true;
    Stub stub;
    stub.set(ADDR(PacketPayloadQueue, size), stub_localPendingPackets_64);
    m_tester->dispatchPackets();
}

TEST_F(UT_NetifPacketCapture, test_dispatchPackets_05)
{
    m_tester->go = true;
    Stub stub;
    stub.set(ADDR(PacketPayloadQueue, size), stub_localPendingPackets_255);
    m_tester->dispatchPackets();
}


TEST_F(UT_NetifPacketCapture, test_dispatchPackets_06)
{
    m_tester->go = false;
    m_tester->dispatchPackets();
}

TEST_F(UT_NetifPacketCapture, test_refreshIfAddrsHashCache)
{
    m_tester->refreshIfAddrsHashCache();
}
