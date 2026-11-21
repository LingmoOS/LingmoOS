// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QString>

#include "base/process.h"

namespace Lingmo {

class ShellConnection;

// The shell process (file manager daemon) which draws the desktop.
//
// Its desktop window lives on a privileged Wayland connection the session
// obtained from the compositor: the socketpair end is inherited and named on
// the command line as `--desktop-socket <fd>`. WAYLAND_DISPLAY names the
// ordinary public socket, which its other windows and everything it launches
// use.
class ShellProcess final
{
public:
    ShellProcess();
    ~ShellProcess();

    // Runs `program --daemon [--desktop-socket <fd>]`. `connection` may be
    // null, in which case the daemon runs without a desktop.
    // Returns false when already running or cannot be started.
    bool startForSession(const QString &program, const QString &socket,
                         ShellConnection *connection);
    void stop();

    bool isRunning() const { return m_process.isRunning(); }

    std::function<void(int code, bool crashed)> onFinished;

private:
    Lingmo::Base::Process m_process;
    QString m_program;
};

} // namespace Lingmo
