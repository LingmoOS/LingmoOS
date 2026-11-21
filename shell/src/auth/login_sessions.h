// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::Auth {

// A Wayland session the login screen offers: a wayland-sessions entry, read as
// lingmo-dm reads it (lingmo-dm/daemon/sessions.cpp), so both agree on
// what exists.
struct LoginSession {
    std::string id;
    std::string name;
    std::vector<std::string> command;
    // What pam_systemd and the session need for a daemon that, like greetd,
    // sets none of it; lingmo-dm sets its own from the id.
    std::vector<std::string> environment;
};

std::vector<LoginSession> findLoginSessions(const std::vector<std::string> &directories);

// /usr/local/share and /usr/share, as lingmo-dm looks.
std::vector<std::string> loginSessionDirectories();
// Replaces those directories; empty restores them.
void setLoginSessionDirectoriesForTesting(std::vector<std::string> directories);

// The accounts a person logs in with, in UID order: UIDs within login.defs'
// UID_MIN..UID_MAX that have a login shell, as sddm's user list takes them.
std::vector<std::string> findLoginUsers();

// lingmo-dm's record of the last login, from the file it keeps for the login
// screen to read. Empty under any other daemon.
std::string lastLoginUser();
std::string lastLoginSession(const std::string &user);

// The copy of the user's [Appearance] settings and picture that lingmo-dm
// keeps for the login screen; a user without one shows the default.
std::string loginBackgroundFile(const std::string &user);

// Splits a command line on white space, with double quotes around an
// argument holding spaces and three in a row standing for one.
std::vector<std::string> splitCommand(std::string_view command);

} // namespace Lingmo::Auth

