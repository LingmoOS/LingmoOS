// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "auth/auth_backend.h"
#include "base/json.h"
#include "base/timer.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::Auth {
class GreetdBackend : public AuthBackend
{
public:
    GreetdBackend();
    ~GreetdBackend() override;

    // Whoever logged in last, as lingmo-dm records it.
    std::string defaultUserName() const override { return lastLoginUser(); }

    void start() override;
    void authenticate(const std::string &userName, Secret secret) override;
    void reset() override;

    void accept() override { }

    void setSession(const LoginSession &session) override { m_session = session; }

private:
    enum class Phase {
        Idle,
        Authenticating,
        Cancelling,
        Starting,
        Complete
    };
    // Members are written in the order given, not sorted.
    void send(const base::json::Value &request);
    void flush();
    void readResponses();
    void handleResponse(const base::json::Value &response);
    void createSession();
    void closeSocket();
    void fail();
    base::EventLoop &m_loop;
    int m_socket = -1;
    int m_watch = 0;
    base::Timer m_timeout;
    std::string m_buffer;
    // Written but not sent yet; wiped as it goes, since it may hold a secret.
    std::string m_output;
    std::string m_userName;
    Secret m_pending;
    LoginSession m_session;
    Phase m_phase = Phase::Idle;
    bool m_sessionOpen = false;
    bool m_retry = false;
};
} // namespace Lingmo::Auth

