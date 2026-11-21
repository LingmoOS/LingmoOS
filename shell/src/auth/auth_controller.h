// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <vector>

#include "auth/login_sessions.h"
#include "base/signal.h"

namespace Lingmo::Auth {

class AuthBackend;

class AuthController
{
public:
    enum Mode {
        Login,
        Lock,
    };

    enum State {
        Idle,
        Authenticating,
        Failed,
        Succeeded,
    };

    AuthController(Mode mode, AuthBackend *backend);
    AuthController(const AuthController &) = delete;
    AuthController &operator=(const AuthController &) = delete;
    ~AuthController();

    Mode mode() const { return m_mode; }
    State state() const { return m_state; }

    std::string userName() const { return m_userName; }
    std::string displayName() const { return m_displayName; }
    std::string iconSource() const { return m_iconSource; }
    std::string errorMessage() const { return m_errorMessage; }

    bool busy() const { return m_state == Authenticating; }
    bool failed() const { return m_state == Failed; }
    bool succeeded() const { return m_state == Succeeded; }

    bool canSelectUser() const { return m_mode == Login; }

    // Login only: the accounts to log in as, by display name, and which one.
    // With a single account there is nothing to pick; with none found the
    // name is typed instead.
    std::vector<std::string> userNames() const;
    // Their pictures, in the same order; empty where an account has none.
    std::vector<std::string> userIcons() const;
    int userIndex() const;
    void setUserIndex(int index);
    bool canPickUser() const { return m_users.size() > 1; }
    bool typesUserName() const { return m_mode == Login && m_users.empty(); }
    void setUsers(std::vector<std::string> users);

    // Login only: the wayland-sessions the login can start, and which one.
    // Choosing a user picks the session that user logged in with last.
    std::vector<std::string> sessionNames() const;
    int sessionIndex() const { return m_sessionIndex; }
    void setSessionIndex(int index);
    bool canSelectSession() const { return m_sessions.size() > 1; }
    void setSessions(std::vector<LoginSession> sessions);

    void setUserName(const std::string &name);

    void start();
    // Wipes the password it is given once it holds it as a Secret.
    void authenticate(std::string password);
    // Back to Idle: clears the error. Called when the user
    // starts typing again after a rejection.
    void reset();

    base::Signal<> stateChanged;
    base::Signal<> userChanged;
    base::Signal<> sessionChanged;
    base::Signal<> sessionsChanged;
    base::Signal<> usersChanged;
    // The credential was rejected. The view shakes and clears its field; it
    // does not decide anything about the session.
    base::Signal<> authenticationFailed;
    // The backend confirmed, and has been told to proceed. Emitted once.
    base::Signal<> authenticationSucceeded;

private:
    void setState(State state, const std::string &errorMessage = {});
    void resolveUser();
    void selectSessionFor(const std::string &userName);

    Mode m_mode;
    AuthBackend *m_backend = nullptr;
    State m_state = Idle;
    std::string m_userName;
    std::string m_displayName;
    std::string m_iconSource;
    std::string m_errorMessage;
    std::vector<std::string> m_users;
    std::vector<LoginSession> m_sessions;
    int m_sessionIndex = -1;
    // accept() is irreversible - it unlocks the session or starts one - so it
    // is guarded rather than merely called from a single place.
    bool m_accepted = false;
    std::vector<base::Subscription> m_backendSubscriptions;
};

} // namespace Lingmo::Auth

