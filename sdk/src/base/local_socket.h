// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace Lingmo::base {

class EventLoop;

// A Unix stream socket client on a loop: input is collected as it arrives,
// output is queued until the socket takes it.
class LocalSocket
{
public:
    // On the calling thread's loop when none is given.
    explicit LocalSocket(EventLoop* loop = nullptr);
    ~LocalSocket();

    LocalSocket(const LocalSocket&) = delete;
    LocalSocket& operator=(const LocalSocket&) = delete;

    // Unix sockets connect at once; false with errorText() when not.
    bool connectToPath(const std::string& path);
    // Takes over a connected socket.
    bool adopt(int fd);
    bool isConnected() const { return m_fd >= 0; }
    int fd() const { return m_fd; }

    // Queued; written as the socket takes it. False when not connected.
    bool write(std::string_view bytes);
    size_t bytesToWrite() const { return m_output.size(); }
    // Everything received so far, taken out.
    std::string readAll();
    size_t bytesAvailable() const { return m_input.size(); }

    // Closes at once, dropping queued output; no onDisconnected follows.
    void abort();

    const std::string& errorText() const { return m_error; }

    // Each may destroy the socket.
    std::function<void()> onReadyRead;
    // The peer closed the connection; the socket is closed now.
    std::function<void()> onDisconnected;
    // Reading or writing failed (errorText() says why); the socket is closed.
    std::function<void()> onError;

private:
    void handleEvents(uint32_t events);
    bool flush();
    void updateWatch();
    void fail(int error);

    EventLoop& m_loop;
    int m_fd = -1;
    int m_watch = 0;
    std::string m_input;
    std::string m_output;
    std::string m_error;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

// A listening Unix stream socket on a loop, handing out each connection it
// accepts. The socket file's mode and owner are the caller's to set.
class LocalServer
{
public:
    // On the calling thread's loop when none is given.
    explicit LocalServer(EventLoop* loop = nullptr);
    // Closes, as close().
    ~LocalServer();

    LocalServer(const LocalServer&) = delete;
    LocalServer& operator=(const LocalServer&) = delete;

    // Removes a socket file left at the path by a server that is gone
    // first; false with errorText() when it cannot listen there, as while
    // another server listens on it.
    bool listen(const std::string& path);
    // Stops listening and removes the socket file.
    void close();
    bool isListening() const { return m_fd >= 0; }
    int fd() const { return m_fd; }
    const std::string& path() const { return m_path; }
    const std::string& errorText() const { return m_error; }

    // A client connected: its descriptor, non-blocking and close-on-exec, is
    // the handler's (LocalSocket::adopt() takes it); closed when unset. May
    // destroy the server.
    std::function<void(int fd)> onNewConnection;

private:
    void acceptPending();

    EventLoop& m_loop;
    int m_fd = -1;
    int m_watch = 0;
    std::string m_path;
    std::string m_error;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::base
