// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "auth/login_sessions.h"
#include "auth/secret.h"
#include "base/signal.h"

#include <string>

namespace Lingmo::Auth {

class AuthBackend
{
public:
    AuthBackend() = default;
    AuthBackend(const AuthBackend &) = delete;
    AuthBackend &operator=(const AuthBackend &) = delete;
    virtual ~AuthBackend() = default;

    // Who the screen offers to authenticate as. Empty means the UI should ask.
    virtual std::string defaultUserName() const = 0;

    // Open the conversation with the authentication service. Called once, after
    // the UI exists, so a prompt or an early error has somewhere to go.
    virtual void start() = 0;

    // Check one credential. The Secret is moved in and dies with the backend's
    // conversation; there is no copy left for the caller to worry about.
    virtual void authenticate(const std::string &userName, Secret secret) = 0;

    // Drop any conversation in progress and return to the initial state. Must
    // leave no credential behind.
    virtual void reset() = 0;

    virtual void accept() = 0;

    // The session a login starts, chosen before authenticate(). Only a login
    // backend starts one; the others ignore it.
    virtual void setSession(const LoginSession &) { }

    // The credential was accepted. accept() may now be called, exactly once.
    base::Signal<> succeeded;

    // The credential was rejected, or the conversation failed. The UI clears
    // the field and shows `message`, which never contains the credential.
    base::Signal<const std::string &> failed;
};

} // namespace Lingmo::Auth

