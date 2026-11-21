// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/local_socket.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <utility>
#include <unistd.h>

#include "base/event_loop.h"

namespace Lingmo::base {

LocalSocket::LocalSocket(EventLoop* loop)
    : m_loop(loop ? *loop : *EventLoop::current())
{
}

LocalSocket::~LocalSocket()
{
    *m_alive = false;
    abort();
}

bool LocalSocket::connectToPath(const std::string& path)
{
    abort();
    m_error.clear();
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if (path.empty() || path.size() >= sizeof(address.sun_path)) {
        m_error = "Invalid socket path";
        return false;
    }
    std::memcpy(address.sun_path, path.c_str(), path.size() + 1);
    const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        m_error = std::strerror(errno);
        return false;
    }
    int result;
    do {
        result = ::connect(fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        m_error = std::strerror(errno);
        ::close(fd);
        return false;
    }
    return adopt(fd);
}

bool LocalSocket::adopt(int fd)
{
    abort();
    if (fd < 0)
        return false;
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    m_fd = fd;
    m_watch = m_loop.watch(m_fd, EPOLLIN | EPOLLRDHUP, [this](uint32_t events) { handleEvents(events); });
    return true;
}

bool LocalSocket::write(std::string_view bytes)
{
    if (m_fd < 0)
        return false;
    m_output.append(bytes);
    return flush();
}

std::string LocalSocket::readAll()
{
    return std::exchange(m_input, {});
}

void LocalSocket::abort()
{
    if (m_watch)
        m_loop.unwatch(m_watch);
    m_watch = 0;
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
    m_output.clear();
}

void LocalSocket::fail(int error)
{
    m_error = std::strerror(error);
    abort();
    if (auto callback = onError)
        callback();
}

bool LocalSocket::flush()
{
    while (!m_output.empty()) {
        const ssize_t n = ::send(m_fd, m_output.data(), m_output.size(), MSG_NOSIGNAL);
        if (n > 0) {
            m_output.erase(0, size_t(n));
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        // Reported from the loop, so that write() never calls back.
        const int error = n < 0 ? errno : EPIPE;
        std::weak_ptr<bool> alive = m_alive;
        m_loop.post([this, alive, error] {
            if (const auto flag = alive.lock(); flag && *flag && m_fd >= 0)
                fail(error);
        });
        return false;
    }
    updateWatch();
    return true;
}

void LocalSocket::updateWatch()
{
    if (m_watch)
        m_loop.setWatchEvents(m_watch, EPOLLIN | EPOLLRDHUP | (m_output.empty() ? 0u : uint32_t(EPOLLOUT)));
}

void LocalSocket::handleEvents(uint32_t events)
{
    const std::shared_ptr<bool> alive = m_alive;
    if (events & EPOLLOUT)
        flush();
    if (m_fd < 0)
        return;

    bool received = false;
    bool closed = false;
    char buffer[16384];
    while (true) {
        const ssize_t n = ::recv(m_fd, buffer, sizeof(buffer), 0);
        if (n > 0) {
            m_input.append(buffer, size_t(n));
            received = true;
            continue;
        }
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        if (n < 0) {
            if (received && onReadyRead) {
                onReadyRead();
                if (!*alive || m_fd < 0)
                    return;
            }
            fail(errno);
            return;
        }
        closed = true;
        break;
    }
    if (received) {
        if (auto callback = onReadyRead)
            callback();
        if (!*alive)
            return;
    }
    if (closed && m_fd >= 0) {
        abort();
        if (auto callback = onDisconnected)
            callback();
    }
}

LocalServer::LocalServer(EventLoop* loop)
    : m_loop(loop ? *loop : *EventLoop::current())
{
}

LocalServer::~LocalServer()
{
    *m_alive = false;
    close();
}

bool LocalServer::listen(const std::string& path)
{
    close();
    m_error.clear();
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if (path.empty() || path.size() >= sizeof(address.sun_path)) {
        m_error = "Invalid socket path";
        return false;
    }
    std::memcpy(address.sun_path, path.c_str(), path.size() + 1);
    // A socket file nobody listens on any more, as after a crash. One that
    // another server still listens on stays its own.
    struct stat status{};
    if (::lstat(path.c_str(), &status) == 0 && S_ISSOCK(status.st_mode)) {
        const int probe = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        // EAGAIN: the listener's backlog is full.
        const bool listening = probe >= 0
                && (::connect(probe, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0
                    || errno == EAGAIN);
        if (probe >= 0)
            ::close(probe);
        if (listening) {
            m_error = std::strerror(EADDRINUSE);
            return false;
        }
        ::unlink(path.c_str());
    }
    const int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        m_error = std::strerror(errno);
        return false;
    }
    if (::bind(fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0
        || ::listen(fd, SOMAXCONN) != 0) {
        m_error = std::strerror(errno);
        ::close(fd);
        return false;
    }
    m_fd = fd;
    m_path = path;
    m_watch = m_loop.watch(m_fd, EPOLLIN, [this](uint32_t) { acceptPending(); });
    return true;
}

void LocalServer::close()
{
    if (m_fd < 0)
        return;
    m_loop.unwatch(m_watch);
    m_watch = 0;
    ::close(m_fd);
    m_fd = -1;
    ::unlink(m_path.c_str());
    m_path.clear();
}

void LocalServer::acceptPending()
{
    const std::weak_ptr<bool> alive = m_alive;
    while (m_fd >= 0) {
        const int client = ::accept4(m_fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client < 0) {
            if (errno == EINTR || errno == ECONNABORTED)
                continue;
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                m_error = std::strerror(errno);
            return;
        }
        if (!onNewConnection) {
            ::close(client);
            continue;
        }
        onNewConnection(client);
        if (alive.expired())
            return;
    }
}

} // namespace Lingmo::base
