// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_FAKE_BACKEND_H
#define LINGMO_TESTS_FAKE_BACKEND_H

#include "auth/auth_backend.h"

#include <string>

// A backend that answers whatever the test tells it to, and counts the things
// that must happen exactly once.
class FakeBackend : public Lingmo::auth::AuthBackend
{

public:
    std::string defaultUserName() const override { return "tester"; }

    void start() override { ++startCount; }

    void authenticate(const std::string &userName, Lingmo::auth::Secret secret) override
    {
        lastUser = userName;
        // Recorded so a test can prove the credential arrived; a real backend
        // would never copy it anywhere that outlives the attempt.
        lastSecret = std::string(secret.constData(), secret.size());
        ++authenticateCount;
    }

    void reset() override { ++resetCount; }

    void accept() override { ++acceptCount; }

    void setSession(const Lingmo::auth::LoginSession &session) override { lastSession = session.id; }

    // Drive the controller the way a real service would.
    void succeed() { succeeded(); }

    void reject(const std::string &message = "Incorrect password") { failed(message); }

    int startCount = 0;
    int authenticateCount = 0;
    int resetCount = 0;
    int acceptCount = 0;
    std::string lastUser;
    std::string lastSecret;
    std::string lastSession;
};

#endif // LINGMO_TESTS_FAKE_BACKEND_H
