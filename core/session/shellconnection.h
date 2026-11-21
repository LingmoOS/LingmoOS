// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

namespace Lingmo {

// The session's end of the compositor control channel.
//
// The session, not the compositor, launches the shell. For every launch it
// creates a Wayland socketpair, sends one end to the compositor over
// SCM_RIGHTS — the compositor turns it into a wl_client and treats that
// client, and only that client, as the trusted shell — and hands the other
// end to the new shell daemon as `--desktop-socket <fd>`.
//
// The channel is held open for the life of the session so a shell restart
// needs no compositor restart.
class ShellConnection
{
public:
    ShellConnection() = default;
    ~ShellConnection();

    ShellConnection(const ShellConnection &) = delete;
    ShellConnection &operator=(const ShellConnection &) = delete;

    // Connects to the compositor's control socket, retrying for timeoutMs
    // while the compositor is still creating it.
    bool connectToCompositor(const QString &path, int timeoutMs = 5000);

    bool isConnected() const { return m_fd >= 0; }

    // Creates a Wayland socketpair, gives one end to the compositor and
    // returns the other for the shell process. Returns -1 on failure;
    // the caller owns the returned descriptor.
    int createShellConnection();

private:
    int m_fd = -1;
};

} // namespace Lingmo
