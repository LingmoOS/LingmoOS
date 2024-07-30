/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CONNECTIONMAPPING_H
#define CONNECTIONMAPPING_H

#include <atomic>
#include <mutex>
#include <regex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <netlink/socket.h>

#include "Packet.h"

struct nl_msg;

/**
 * @todo write docs
 */
class ConnectionMapping
{
public:
    using inode_t = uint32_t;
    using pid_t = int;

    struct PacketResult {
        pid_t pid = 0;
        Packet::Direction direction;
    };

    ConnectionMapping();
    ~ConnectionMapping();

    PacketResult pidForPacket(const Packet &packet);

private:
    struct State {
        State &operator=(const State &other)
        {
            addressToInode = other.addressToInode;
            inodeToPid = other.inodeToPid;

            return *this;
        }

        std::unordered_map<Packet::Address, inode_t> addressToInode;
        std::unordered_map<inode_t, pid_t> inodeToPid;
    };

    void loop();
    bool dumpSockets(nl_sock *socket);
    bool dumpSockets(nl_sock *socket, int inet_family, int protocol);
    void parsePid();

    State m_oldState;
    State m_newState;

    bool m_newInode = false;
    std::unordered_set<Packet::Address> m_seenAddresses;
    std::unordered_set<inode_t> m_seenInodes;

    std::thread m_thread;
    std::atomic_bool m_running;
    std::mutex m_mutex;

    friend int parseInetDiagMesg(struct nl_msg *msg, void *arg);
};

#endif // CONNECTIONMAPPING_H
