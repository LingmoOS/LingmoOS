// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "auth/greetd_backend.h"
#include "core/main_loop.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string_view>

using Lingmo::i18n::tr;

namespace Lingmo::Auth {

namespace {

base::json::Value stringArray(const std::vector<std::string> &strings)
{
    base::json::Value::Array array;
    for (const std::string &string : strings)
        array.emplace_back(string);
    return base::json::Value(std::move(array));
}

} // namespace

GreetdBackend::GreetdBackend()
    : m_loop(*core::mainLoop())
    , m_timeout(&m_loop)
{
    m_timeout.setSingleShot(true);
    m_timeout.onTimeout = [this] {
        closeSocket();
        fail();
    };
}

GreetdBackend::~GreetdBackend()
{
    if (m_sessionOpen && m_phase != Phase::Complete)
        send(base::json::Value::object({{"type", "cancel_session"}}));
    closeSocket();
}

void GreetdBackend::start()
{
    const char *path = getenv("GREETD_SOCK");
    sockaddr_un address {};
    if (!path || !*path || strlen(path) >= sizeof(address.sun_path)) {
        fail();
        return;
    }
    closeSocket();
    m_socket = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    if (m_socket < 0
        || connect(m_socket, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0) {
        closeSocket();
        fail();
        return;
    }
    m_watch = m_loop.watch(m_socket, EPOLLIN, [this](uint32_t events) {
        if (events & EPOLLOUT)
            flush();
        if (m_socket >= 0 && (events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
            readResponses();
    });
}

void GreetdBackend::authenticate(const std::string &userName, Secret secret)
{
    if (m_phase == Phase::Authenticating || m_phase == Phase::Starting
        || m_phase == Phase::Complete)
        return;
    if (userName.empty() || secret.isEmpty() || m_socket < 0) {
        fail();
        return;
    }
    m_userName = userName;
    m_pending = std::move(secret);
    m_retry = true;
    if (m_phase == Phase::Cancelling)
        return;
    if (m_sessionOpen) {
        m_phase = Phase::Cancelling;
        send(base::json::Value::object({{"type", "cancel_session"}}));
    } else {
        createSession();
    }
}

void GreetdBackend::createSession()
{
    m_retry = false;
    m_sessionOpen = true;
    m_phase = Phase::Authenticating;
    send(base::json::Value::object({{"type", "create_session"}, {"username", m_userName}}));
}

void GreetdBackend::reset()
{
    m_pending.clear();
    m_retry = false;
    if (m_phase == Phase::Idle && m_sessionOpen) {
        m_phase = Phase::Cancelling;
        send(base::json::Value::object({{"type", "cancel_session"}}));
    }
}

void GreetdBackend::send(const base::json::Value &request)
{
    if (m_socket < 0) {
        fail();
        return;
    }
    std::string payload = request.serialize();
    const uint32_t length = uint32_t(payload.size());
    m_output.append(reinterpret_cast<const char *>(&length), sizeof(length));
    m_output += payload;
    explicit_bzero(payload.data(), payload.size());
    flush();
    m_timeout.start(std::chrono::milliseconds(30000));
}

void GreetdBackend::flush()
{
    while (!m_output.empty() && m_socket >= 0) {
        const ssize_t written = ::send(m_socket, m_output.data(), m_output.size(), MSG_NOSIGNAL);
        if (written < 0 && errno == EINTR)
            continue;
        if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        if (written <= 0) {
            closeSocket();
            fail();
            return;
        }
        explicit_bzero(m_output.data(), size_t(written));
        m_output.erase(0, size_t(written));
    }
    if (m_socket >= 0)
        m_loop.setWatchEvents(m_watch, m_output.empty() ? EPOLLIN : EPOLLIN | EPOLLOUT);
}

void GreetdBackend::readResponses()
{
    char chunk[4096];
    while (m_socket >= 0) {
        const ssize_t received = recv(m_socket, chunk, sizeof(chunk), 0);
        if (received < 0 && errno == EINTR)
            continue;
        if (received < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        if (received <= 0) {
            closeSocket();
            fail();
            return;
        }
        m_buffer.append(chunk, size_t(received));
    }
    while (m_buffer.size() >= 4) {
        uint32_t length;
        memcpy(&length, m_buffer.data(), 4);
        if (length > 65536) {
            closeSocket();
            fail();
            return;
        }
        if (m_buffer.size() < 4 + size_t(length))
            return;
        const std::optional<base::json::Value> document =
                base::json::Value::parse(std::string_view(m_buffer).substr(4, length));
        m_buffer.erase(0, 4 + size_t(length));
        m_timeout.stop();
        if (!document || !document->isObject()) {
            fail();
            return;
        }
        handleResponse(*document);
    }
}

void GreetdBackend::handleResponse(const base::json::Value &response)
{
    const std::string type = response["type"].toString();
    if (type == "error") {
        fail();
        return;
    }
    if (type == "success") {
        if (m_phase == Phase::Cancelling) {
            m_sessionOpen = false;
            m_phase = Phase::Idle;
            if (m_retry)
                createSession();
        } else if (m_phase == Phase::Authenticating) {
            m_pending.clear();
            m_phase = Phase::Starting;
            // The command and environment are for greetd; lingmo-dm starts
            // the session its id names and sets the environment itself.
            base::json::Value start = base::json::Value::object(
                    {{"cmd",
                      stringArray(m_session.command.empty()
                                          ? std::vector<std::string> {"lingmo-session"}
                                          : m_session.command)},
                     {"env", stringArray(m_session.environment)}});
            if (!m_session.id.empty())
                start.insert("session", m_session.id);
            start.insert("type", "start_session");
            send(start);
        } else if (m_phase == Phase::Starting) {
            m_phase = Phase::Complete;
            succeeded();
        }
        return;
    }
    if (type != "auth_message" || m_phase != Phase::Authenticating) {
        fail();
        return;
    }
    const std::string kind = response["auth_message_type"].toString();
    base::json::Value answer;
    if (kind == "secret" && !m_pending.isEmpty()) {
        answer = base::json::Value(std::string_view(m_pending.constData(), m_pending.size()));
        m_pending.clear();
    } else if (kind == "visible") {
        answer = m_userName;
    } else if (kind != "info" && kind != "error") {
        fail();
        return;
    }
    send(base::json::Value::object(
            {{"response", std::move(answer)}, {"type", "post_auth_message_response"}}));
}

void GreetdBackend::closeSocket()
{
    if (m_watch)
        m_loop.unwatch(std::exchange(m_watch, 0));
    if (m_socket >= 0)
        close(std::exchange(m_socket, -1));
    explicit_bzero(m_output.data(), m_output.size());
    m_output.clear();
}

void GreetdBackend::fail()
{
    m_timeout.stop();
    m_pending.clear();
    m_retry = false;
    if (m_phase == Phase::Complete)
        return;
    m_phase = Phase::Idle;
    failed(tr("Sign-in failed"));
}

} // namespace Lingmo::Auth
