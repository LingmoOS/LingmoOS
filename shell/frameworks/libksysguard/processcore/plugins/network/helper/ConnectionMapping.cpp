/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ConnectionMapping.h"

#include <cstring>
#include <charconv>
#include <fstream>
#include <iostream>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>

#include <netlink/msg.h>
#include <netlink/netlink.h>

using namespace std::string_literals;

template<typename Key, typename Value>
inline void cleanupOldEntries(const std::unordered_set<Key> &keys, std::unordered_map<Key, Value> &map)
{
    for (auto itr = map.begin(); itr != map.end();) {
        if (keys.find(itr->first) == keys.end()) {
            itr = map.erase(itr);
        } else {
            itr++;
        }
    }
}

ConnectionMapping::inode_t toInode(const std::string_view &view)
{
    ConnectionMapping::inode_t value;
    if (auto status = std::from_chars(view.data(), view.data() + view.length(), value); status.ec == std::errc()) {
        return value;
    }
    return std::numeric_limits<ConnectionMapping::inode_t>::max();
}

int parseInetDiagMesg(struct nl_msg *msg, void *arg)
{
    auto self = static_cast<ConnectionMapping *>(arg);
    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    auto inetDiagMsg = static_cast<inet_diag_msg *>(nlmsg_data(nlh));
    Packet::Address localAddress;
    if (inetDiagMsg->idiag_family == AF_INET) {
        // I expected to need ntohl calls here and bewlow for src but comparing to values gathered
        // by parsing proc they are not needed and even produce wrong results
        localAddress.address[3] = inetDiagMsg->id.idiag_src[0];
    } else if (inetDiagMsg->id.idiag_src[0] == 0 && inetDiagMsg->id.idiag_src[1] == 0 && inetDiagMsg->id.idiag_src[2] == 0xffff0000) {
        // Some applications (like Steam) use ipv6 sockets with ipv4.
        // This results in ipv4 addresses that end up in the tcp6 file.
        // They seem to start with 0000000000000000FFFF0000, so if we
        // detect that, assume it is ipv4-over-ipv6.
        localAddress.address[3] = inetDiagMsg->id.idiag_src[3];

    } else {
        std::memcpy(localAddress.address.data(), inetDiagMsg->id.idiag_src, sizeof(localAddress.address));
    }
    localAddress.port = ntohs(inetDiagMsg->id.idiag_sport);

    if (self->m_newState.addressToInode.find(localAddress) == self->m_newState.addressToInode.end()) {
        // new localAddress is found for which no socket inode is known
        // will trigger pid parsing
        self->m_newState.addressToInode.emplace(localAddress, inetDiagMsg->idiag_inode);
        self->m_newInode = true;
    }

    self->m_seenAddresses.insert(localAddress);
    self->m_seenInodes.insert(inetDiagMsg->idiag_inode);

    return NL_OK;
}

ConnectionMapping::ConnectionMapping()
    : m_running(true)
{
    m_thread = std::thread(&ConnectionMapping::loop, this);
}

ConnectionMapping::~ConnectionMapping()
{
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}


ConnectionMapping::PacketResult ConnectionMapping::pidForPacket(const Packet &packet)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    PacketResult result;

    auto sourceInode = m_oldState.addressToInode.find(packet.sourceAddress());
    auto destInode = m_oldState.addressToInode.find(packet.destinationAddress());

    if (sourceInode == m_oldState.addressToInode.end() && destInode == m_oldState.addressToInode.end()) {
        return result;
    }

    auto inode = m_oldState.addressToInode.end();
    if (sourceInode != m_oldState.addressToInode.end()) {
        result.direction = Packet::Direction::Outbound;
        inode = sourceInode;
    } else {
        result.direction = Packet::Direction::Inbound;
        inode = destInode;
    }

    auto pid = m_oldState.inodeToPid.find((*inode).second);
    if (pid == m_oldState.inodeToPid.end()) {
        result.pid = -1;
    } else {
        result.pid = (*pid).second;
    }
    return result;
}

void ConnectionMapping::loop()
{
    std::unique_ptr<nl_sock, decltype(&nl_socket_free)> socket{nl_socket_alloc(), nl_socket_free};

    nl_connect(socket.get(), NETLINK_SOCK_DIAG);
    nl_socket_modify_cb(socket.get(), NL_CB_VALID, NL_CB_CUSTOM, &parseInetDiagMesg, this);

    while (m_running) {
        m_seenAddresses.clear();
        m_seenInodes.clear();

        dumpSockets(socket.get());

        if (m_newInode) {
            parsePid();
            m_newInode = false;
        }

        cleanupOldEntries(m_seenAddresses, m_newState.addressToInode);
        cleanupOldEntries(m_seenInodes, m_newState.inodeToPid);

        {
            std::lock_guard<std::mutex> lock{m_mutex};
            m_oldState = m_newState;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool ConnectionMapping::dumpSockets(nl_sock *socket)
{
    for (auto family : {AF_INET, AF_INET6}) {
        for (auto protocol : {IPPROTO_TCP, IPPROTO_UDP}) {
            if (!dumpSockets(socket, family, protocol)) {
                return false;
            }
        }
    }
    return true;
}

bool ConnectionMapping::dumpSockets(nl_sock *socket, int inet_family, int protocol)
{
    inet_diag_req_v2 inet_request;
    inet_request.id = {};
    inet_request.sdiag_family = inet_family;
    inet_request.sdiag_protocol = protocol;
    inet_request.idiag_states = -1;
    if (nl_send_simple(socket, SOCK_DIAG_BY_FAMILY, NLM_F_DUMP | NLM_F_REQUEST, &inet_request, sizeof(inet_diag_req_v2)) < 0) {
        return false;
    }
    if (nl_recvmsgs_default(socket) != 0) {
        return false;
    }
    return true;
}

void ConnectionMapping::parsePid()
{
    auto dir = opendir("/proc");

    std::array<char, 100> buffer;

    auto fdPath = "/proc/%/fd"s;
    // Ensure the string has enough space to accommodate large PIDs
    fdPath.reserve(30);

    // The only way to get a list of PIDs is to list the contents of /proc.
    // Any directory with a numeric name corresponds to a process and its PID.
    dirent *entry = nullptr;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) {
            continue;
        }

        if (entry->d_name[0] < '0' || entry->d_name[0] > '9') {
            continue;
        }

        // We need to list the contents of a subdirectory of the PID directory.
        // To avoid multiple allocations we reserve the string above and reuse
        // it here.
        fdPath.replace(6, fdPath.find_last_of('/') - 6, entry->d_name);

        auto fdDir = opendir(fdPath.data());
        if (fdDir == NULL) {
            continue;
        }

        dirent *fd = nullptr;
        while ((fd = readdir(fdDir))) {
            if (fd->d_type != DT_LNK) {
                continue;
            }

            // /proc/PID/fd contains symlinks for each open fd in the process.
            // The symlink target contains information about what the fd is about.
            auto size = readlinkat(dirfd(fdDir), fd->d_name, buffer.data(), 99);
            if (size < 0) {
                continue;
            }
            buffer[size] = '\0';

            auto view = std::string_view(buffer.data(), 100);

            // In this case, we are only interested in sockets, for which the
            // symlink target starts with 'socket:', followed by the inode
            // number in square brackets.
            if (view.compare(0, 7, "socket:") != 0) {
                continue;
            }

            // Strip off the leading "socket:" part and the opening bracket,
            // then convert that to an inode number.
            auto inode = toInode(view.substr(8));
            if (inode != std::numeric_limits<inode_t>::max()) {
                m_newState.inodeToPid[inode] = std::stoi(entry->d_name);
            }
        }

        closedir(fdDir);
    }

    closedir(dir);
}
