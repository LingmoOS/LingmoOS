/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PACKET_H
#define PACKET_H

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>

#include "TimeStamps.h"

class Packet
{
public:
    enum class NetworkProtocolType {
        Unknown,
        IPv4,
        IPv6,
    };

    enum class TransportProtocolType {
        Unknown,
        Tcp,
        Udp,
    };

    enum class Direction {
        Inbound,
        Outbound,
    };

    struct Address {
        std::array<uint32_t, 4> address = {0};
        uint32_t port = 0;

        inline bool operator==(const Address &other) const
        {
            return address == other.address && port == other.port;
        }
    };

    Packet();

    Packet(const TimeStamp::MicroSeconds &timeStamp, const uint8_t *data, uint32_t dataLength, uint32_t packetSize);

    ~Packet();

    Packet(const Packet &other) = delete;
    Packet(Packet &&other) = default;

    TimeStamp::MicroSeconds timeStamp() const;
    unsigned int size() const;
    NetworkProtocolType networkProtocol() const;
    TransportProtocolType transportProtocol() const;
    Address sourceAddress() const;
    Address destinationAddress() const;

private:
    void parseIPv4(const uint8_t *data, int32_t dataLength);
    void parseIPv6(const uint8_t *data, int32_t dataLength);
    void parseTransport(uint8_t type, const uint8_t *data, int32_t dataLength);

    TimeStamp::MicroSeconds m_timeStamp;
    unsigned int m_size = 0;

    NetworkProtocolType m_networkProtocol = NetworkProtocolType::Unknown;
    TransportProtocolType m_transportProtocol = TransportProtocolType::Unknown;

    Address m_sourceAddress;
    Address m_destinationAddress;
};

inline std::ostream &operator<<(std::ostream &stream, const Packet::Address &address)
{
    stream << std::hex << address.address[0] << ":" << address.address[1] << ":" << address.address[2] << ":" << address.address[3];
    stream << std::dec << "::" << address.port;
    return stream;
}

namespace std
{
template<>
struct hash<Packet::Address> {
    using argument_type = Packet::Address;
    using result_type = std::size_t;
    inline result_type operator()(argument_type const &address) const noexcept
    {
        return std::hash<uint32_t>{}(address.address[0]) //
            ^ (std::hash<uint32_t>{}(address.address[1]) << 1) //
            ^ (std::hash<uint32_t>{}(address.address[2]) << 2) //
            ^ (std::hash<uint32_t>{}(address.address[3]) << 3) //
            ^ (std::hash<uint32_t>{}(address.port) << 4);
    }
};
}

#endif // PACKET_H
