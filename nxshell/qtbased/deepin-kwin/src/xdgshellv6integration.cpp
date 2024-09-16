/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xdgshellv6integration.h"
#include "wayland/display.h"
#include "wayland/xdgshell_v6_interface.h"
#include "wayland_server.h"
#include "workspace.h"
#include "xdgshellv6window.h"

using namespace KWaylandServer;

namespace KWin
{

/**
 * The WaylandXdgShellV6Integration class is a factory class for xdg-shell windows.
 *
 * The xdg-shell protocol defines two surface roles - xdg_toplevel and xdg_popup. On the
 * compositor side, those roles are represented by XdgToplevelV6Window and XdgPopupV6Window,
 * respectively.
 *
 * WaylandXdgShellV6Integration monitors for new xdg_toplevel and xdg_popup objects. If it
 * detects one, it will create an XdgToplevelV6Window or XdgPopupV6Window based on the current
 * surface role of the underlying xdg_surface object.
 */

XdgShellV6Integration::XdgShellV6Integration(QObject *parent)
    : WaylandShellIntegration(parent)
{
    XdgShellV6Interface *shell = new XdgShellV6Interface(waylandServer()->display(), this);

    connect(shell, &XdgShellV6Interface::toplevelCreated,
            this, &XdgShellV6Integration::registerXdgToplevelV6);
    connect(shell, &XdgShellV6Interface::popupCreated,
            this, &XdgShellV6Integration::registerXdgPopupV6);
}

void XdgShellV6Integration::registerXdgToplevelV6(XdgToplevelV6Interface *toplevel)
{
    // Note that the window is going to be destroyed and immediately re-created when the
    // underlying surface is unmapped. XdgToplevelWindow is re-created right away since
    // we don't want too loose any window requests that are allowed to be sent prior to
    // the first initial commit, e.g. set_maximized or set_fullscreen.
    connect(toplevel, &XdgToplevelV6Interface::resetOccurred,
            this, [this, toplevel] {
                createXdgToplevelV6Window(toplevel);
            });

    createXdgToplevelV6Window(toplevel);
}

void XdgShellV6Integration::createXdgToplevelV6Window(XdgToplevelV6Interface *toplevel)
{
    if (!workspace()) {
        qCWarning(KWIN_CORE, "An xdg-toplevel surface has been created while the compositor "
                             "is still not fully initialized. That is a compositor bug!");
        return;
    }

    Q_EMIT windowCreated(new XdgToplevelV6Window(toplevel));
}

void XdgShellV6Integration::registerXdgPopupV6(XdgPopupV6Interface *popup)
{
    if (!workspace()) {
        qCWarning(KWIN_CORE, "An xdg-popup surface has been created while the compositor is "
                             "still not fully initialized. That is a compositor bug!");
        return;
    }

    Q_EMIT windowCreated(new XdgPopupV6Window(popup));
}

} // namespace KWin
