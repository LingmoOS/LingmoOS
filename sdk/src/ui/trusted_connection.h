// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ui/geometry.h"

namespace Lingmo::ui {

namespace gpu {
class EglContext;
}

namespace wayland {
class Connection;
}

// A second Wayland connection the session hands over as a socket descriptor,
// on which the compositor offers the trusted lingmo_shell_v1: the desktop
// window lives there (Window(TrustedConnection&)), everything else on the
// application's own connection. It shares the application's loop, has its
// own GPU context, and goes after every window made on it.
class TrustedConnection
{
public:
    // Takes the descriptor; null when nothing answers on it or the
    // application does not run on Wayland.
    static std::unique_ptr<TrustedConnection> fromFd(int fd);
    ~TrustedConnection();

    TrustedConnection(const TrustedConnection&) = delete;
    TrustedConnection& operator=(const TrustedConnection&) = delete;

    struct Output
    {
        // The wl_output name, which Window::setDesktopRole takes.
        std::string name;
        // In the compositor's global logical coordinates.
        PointF position;
        SizeF logicalSize;
    };
    std::vector<Output> outputs() const;
    // After an output came, went or changed.
    std::function<void()> onOutputsChanged;
    // Whether the compositor offers the shell interface here.
    bool hasShell() const;

private:
    friend class Window;

    TrustedConnection() = default;

    std::unique_ptr<wayland::Connection> m_connection;
    std::unique_ptr<gpu::EglContext> m_gpu;
};

} // namespace Lingmo::ui
