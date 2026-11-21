// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

// Transient systemd scopes for launched applications, named
// app-lingmo-<AppID>-<pid>.scope as the desktop entry specification asks,
// which is how xdg-desktop-portal learns the app id of a host application.
namespace ApplicationScope {

// The unit name, with the app id escaped the way systemd escapes unit names.
std::string unitName(const std::string &appId, uint32_t pid);

// Moves the process into its own scope under the user manager. Failure is
// logged and otherwise ignored: the application runs either way.
void adopt(const std::string &appId, uint32_t pid);

// Sends SIGKILL to every process left in the scope. A scope that already went
// with its last process is no failure.
void killAll(const std::string &unitName);

} // namespace ApplicationScope
