// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netif_packet_parser.h"
#include <net/ethernet.h>
#include <netinet/ip6.h>
//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

/***************************************STUB begin*********************************************/

unsigned short int stub_ntohs_IP()
{
    return ETHERTYPE_IP;
}

unsigned short int stub_ntohs_IPV6()
{
    return ETHERTYPE_IPV6;
}

/***************************************STUB end**********************************************/


class UT_NetifPacketParser: public ::testing::Test
{
public:
    UT_NetifPacketParser() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifPacketParser();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifPacketParser *m_tester;
};

TEST_F(UT_NetifPacketParser, initTest)
{
}

TEST_F(UT_NetifPacketParser, test_parsePacket_01)
{
    pcap_pkthdr hdr;
    memset(&hdr,0,sizeof(pcap_pkthdr));
    const u_char packet[64] = {0};
    PacketPayload payload = nullptr;

    Stub stub;
    stub.set(ntohs, stub_ntohs_IP);
    EXPECT_EQ(m_tester->parsePacket(&hdr, packet, payload), false);
}

TEST_F(UT_NetifPacketParser, test_parsePacket_02)
{
    pcap_pkthdr hdr;
    memset(&hdr,0,sizeof(pcap_pkthdr));
    const u_char packet[64] = {0};
    PacketPayload payload = QSharedPointer<struct packet_payload_t>::create();

    Stub stub;
    stub.set(ntohs, stub_ntohs_IP);
    EXPECT_EQ(m_tester->parsePacket(&hdr, packet, payload), false);
}

TEST_F(UT_NetifPacketParser, test_parsePacket_03)
{
    pcap_pkthdr hdr;
    memset(&hdr,0,sizeof(pcap_pkthdr));
    hdr.caplen = 15;
    const u_char packet[64] = {0};
    PacketPayload payload = QSharedPointer<struct packet_payload_t>::create();

    Stub stub;
    stub.set(ntohs, stub_ntohs_IP);
    EXPECT_EQ(m_tester->parsePacket(&hdr, packet, payload), false);
}

TEST_F(UT_NetifPacketParser, test_parsePacket_04)
{
//    pcap_pkthdr hdr;
//    memset(&hdr,0,sizeof(hdr));
//    const u_char packet[64] = {0};
//    PacketPayload payload = QSharedPointer<struct packet_payload_t>::create();

//    Stub stub;
//    stub.set(ntohs, stub_ntohs_IPV6);
//    EXPECT_EQ(m_tester->parsePacket(&hdr, packet, payload), false);
}

TEST_F(UT_NetifPacketParser, test_parsePacket_05)
{
//    pcap_pkthdr hdr;
//    memset(&hdr,0,sizeof(hdr));
//    int eth_hdr_len = sizeof(struct ether_header);
//    int ip6_len = sizeof(struct ip6_hdr);
//    u_char packet[eth_hdr_len+ip6_len+1] = {0};
//    ether_header header{};
//    ip6_hdr ip6body{};
//    char *packet = (char*)header;

//    memcpy(packet, &header, eth_hdr_len);
//    memcpy(packet+eth_hdr_len, &ip6body, ip6_len);
//    const u_char packet1[eth_hdr_len + ip6_len+1]= "00000000000000<<<<<<<<<<";
//    PacketPayload payload = QSharedPointer<struct packet_payload_t>::create();

//    Stub stub;
//    stub.set(ntohs, stub_ntohs_IPV6);
//    EXPECT_EQ(m_tester->parsePacket(&hdr, packet1, payload), false);
}
