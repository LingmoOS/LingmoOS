// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shellconnection.h"

#include "logs.h"

#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

namespace Lingmo {

// One byte of payload so the ancillary data has a datagram to ride on.
static bool sendFd(int socket, int fd)
{
    char payload = 'S';
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

    cmsghdr *header = CMSG_FIRSTHDR(&message);
    header->cmsg_level = SOL_SOCKET;
    header->cmsg_type = SCM_RIGHTS;
    header->cmsg_len = CMSG_LEN(sizeof(int));
    std::memcpy(CMSG_DATA(header), &fd, sizeof(fd));

    return sendmsg(socket, &message, MSG_NOSIGNAL) == 1;
}

ShellConnection::~ShellConnection()
{
    if (m_fd >= 0)
        close(m_fd);
}

bool ShellConnection::connectToCompositor(const QString &path, int timeoutMs)
{
    sockaddr_un address {};
    address.sun_family = AF_UNIX;
    const QByteArray pathBytes = path.toLocal8Bit();
    if (size_t(pathBytes.size()) >= sizeof(address.sun_path)) {
        LM_WARNING(sessionLog()) << "Compositor control socket path is too long:" << path;
        return false;
    }
    std::memcpy(address.sun_path, pathBytes.constData(), size_t(pathBytes.size()));

    // The compositor creates the socket while it is starting; the ready
    // notification only says the Wayland display is up.
    const auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::milliseconds(timeoutMs);
    while (true) {
        const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (fd < 0)
            return false;

        if (::connect(fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0) {
            m_fd = fd;
            break;
        }
        close(fd);

        if (std::chrono::steady_clock::now() >= deadline) {
            LM_WARNING(sessionLog()) << "Cannot reach the compositor control socket" << path;
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return true;
}

int ShellConnection::createShellConnection()
{
    if (m_fd < 0)
        return -1;

    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) != 0) {
        LM_WARNING(sessionLog()) << "Cannot create the shell Wayland socketpair";
        return -1;
    }

    const bool sent = sendFd(m_fd, pair[0]);
    close(pair[0]);
    if (!sent) {
        LM_WARNING(sessionLog()) << "Cannot pass the shell connection to the compositor";
        close(pair[1]);
        return -1;
    }

    return pair[1];
}

} // namespace Lingmo
