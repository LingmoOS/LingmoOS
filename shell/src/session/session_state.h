// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Session state follows Ash at revision 7acd40ddefff49c9d36144999aafdbb674dbd07a.

#ifndef LINGMO_SESSION_SESSION_STATE_H_
#define LINGMO_SESSION_SESSION_STATE_H_

namespace Lingmo::Session {

enum class SessionState {
    Active,
    // Login has no user desktop to return to.
    Login,
    Locking,
    Locked,
    Unlocking,
};

// Both transitions remain blocked until secure presentation completes.
constexpr bool isSessionBlocked(SessionState state)
{
    return state != SessionState::Active;
}

const char *toString(SessionState state);

class SessionObserver
{
public:
    virtual ~SessionObserver() = default;

    virtual void onSessionStateChanged(SessionState state) = 0;

    // The outputs the lock covers were added, removed, moved or rescaled.
    virtual void onSessionOutputsChanged() { }
};

} // namespace Lingmo::Session

#endif // LINGMO_SESSION_SESSION_STATE_H_
