// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_PACKET_PARSER_H
#define NETIF_PACKET_PARSER_H

#include "packet.h"

namespace core {
namespace system {

class NetifPacketParser
{
public:
    static bool parsePacket(const struct pcap_pkthdr *pkt_hdr,
                            const u_char *packet,
                            PacketPayload &payload);


private:
    NetifPacketParser() = default;
};

} // namespace system
} // namespace core

#endif // NETIF_PACKET_PARSER_H
