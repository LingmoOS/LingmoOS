// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wayland/shell_control_channel.h"

#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "core/logging.h"
#include "core/main_loop.h"

namespace Lingmo::Wayland {

namespace {

// One control message, one fd. The payload byte exists only because a
// zero-length datagram would not carry the ancillary data.
int receiveFd(int socket)
{
    char payload = 0;
    iovec iov {};
    iov.iov_base = &payload;
    iov.iov_len = 1;

    union {
        char buffer[CMSG_SPACE(sizeof(int))];
        cmsghdr alignment;
    } control {};

    msghdr message {};
    message.msg_iov = &iov;
    message.msg_iovlen = 1;
    message.msg_control = control.buffer;
    message.msg_controllen = sizeof(control.buffer);

    const ssize_t received = recvmsg(socket, &message, MSG_CMSG_CLOEXEC);
    if (received < 0)
        return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ? -2 : -1;
    if (received == 0)
        return -1;

    // One descriptor is expected; every other one delivered still belongs to
    // this process and would leak if it were merely skipped.
    int fd = -1;
    for (cmsghdr *header = CMSG_FIRSTHDR(&message); header;
         header = CMSG_NXTHDR(&message, header)) {
        if (header->cmsg_level != SOL_SOCKET || header->cmsg_type != SCM_RIGHTS)
            continue;
        const size_t count = (header->cmsg_len - CMSG_LEN(0)) / sizeof(int);
        for (size_t i = 0; i < count; ++i) {
            int received = -1;
            std::memcpy(&received, CMSG_DATA(header) + i * sizeof(int), sizeof(received));
            if (fd < 0)
                fd = received;
            else
                close(received);
        }
    }
    return fd >= 0 ? fd : -2;
}

} // namespace

ShellControlChannel::ShellControlChannel(base::EventLoop *loop)
    : m_loop(loop ? *loop : *core::mainLoop())
{
}

ShellControlChannel::~ShellControlChannel()
{
    closeControl();
    if (m_listenWatch)
        m_loop.unwatch(m_listenWatch);
    if (m_listenFd >= 0)
        close(m_listenFd);
    if (!m_path.empty())
        unlink(m_path.c_str());
}

bool ShellControlChannel::listen(const std::string &path)
{
    sockaddr_un address {};
    address.sun_family = AF_UNIX;
    if (path.size() >= sizeof(address.sun_path)) {
        LM_WARNING(lingmoWayland) << "Control socket path is too long:" << path.c_str();
        return false;
    }
    std::memcpy(address.sun_path, path.data(), path.size());

    m_listenFd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    if (m_listenFd < 0) {
        LM_WARNING(lingmoWayland) << "Cannot create the shell control socket";
        return false;
    }

    // A socket left behind by a compositor that did not shut down cleanly would
    // make bind fail; nothing else may own this path.
    unlink(path.c_str());
    if (bind(m_listenFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0
        || ::listen(m_listenFd, 4) != 0) {
        LM_WARNING(lingmoWayland) << "Cannot listen on the shell control socket" << path.c_str();
        close(m_listenFd);
        m_listenFd = -1;
        return false;
    }

    m_path = path;
    m_listenWatch = m_loop.watch(m_listenFd, EPOLLIN, [this](uint32_t) {
        onAcceptReady();
    });
    LM_INFO(lingmoWayland) << "Shell control channel listening on" << path.c_str();
    return true;
}

void ShellControlChannel::onAcceptReady()
{
    const int fd = accept4(m_listenFd, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (fd < 0)
        return;

    closeControl();
    m_controlFd = fd;
    m_controlWatch = m_loop.watch(m_controlFd, EPOLLIN, [this](uint32_t) {
        onControlReady();
    });
}

void ShellControlChannel::onControlReady()
{
    const int fd = receiveFd(m_controlFd);
    if (fd == -1) {
        // The session went away or its socket failed. The compositor and its
        // clients carry on.
        closeControl();
        return;
    }
    if (fd == -2)
        return;

    if (connectionReceived)
        connectionReceived(fd);
    else
        close(fd);
}

void ShellControlChannel::closeControl()
{
    if (m_controlWatch) {
        m_loop.unwatch(m_controlWatch);
        m_controlWatch = 0;
    }
    if (m_controlFd >= 0) {
        close(m_controlFd);
        m_controlFd = -1;
    }
}

} // namespace Lingmo::Wayland
