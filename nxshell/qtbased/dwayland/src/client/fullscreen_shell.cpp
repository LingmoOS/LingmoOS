/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fullscreen_shell.h"
#include "output.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <QDebug>
// wayland
#include <wayland-client-protocol.h>
#include <wayland-fullscreen-shell-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN FullscreenShell::Private
{
public:
    Private(FullscreenShell *q);
    void setup(_wl_fullscreen_shell *shell);

    WaylandPointer<_wl_fullscreen_shell, _wl_fullscreen_shell_release> shell;
    EventQueue *queue = nullptr;
    bool capabilityArbitraryModes = false;
    bool capabilityCursorPlane = false;

private:
    void handleCapabilities(uint32_t capability);
    static void capabilitiesAnnounce(void *data, struct _wl_fullscreen_shell *shell, uint32_t capability);
    static _wl_fullscreen_shell_listener s_fullscreenShellListener;
    FullscreenShell *q;
};

_wl_fullscreen_shell_listener FullscreenShell::Private::s_fullscreenShellListener = {FullscreenShell::Private::capabilitiesAnnounce};

FullscreenShell::Private::Private(FullscreenShell *q)
    : q(q)
{
}

void FullscreenShell::Private::setup(_wl_fullscreen_shell *s)
{
    Q_ASSERT(!shell);
    Q_ASSERT(s);
    shell.setup(s);
    _wl_fullscreen_shell_add_listener(shell, &s_fullscreenShellListener, this);
}

void FullscreenShell::Private::capabilitiesAnnounce(void *data, _wl_fullscreen_shell *shell, uint32_t capability)
{
    auto s = reinterpret_cast<FullscreenShell::Private *>(data);
    Q_ASSERT(shell == s->shell);
    s->handleCapabilities(capability);
}

void FullscreenShell::Private::handleCapabilities(uint32_t capability)
{
    if (capability & _WL_FULLSCREEN_SHELL_CAPABILITY_ARBITRARY_MODES) {
        capabilityArbitraryModes = true;
        Q_EMIT q->capabilityArbitraryModesChanged(capabilityArbitraryModes);
    }
    if (capability & _WL_FULLSCREEN_SHELL_CAPABILITY_CURSOR_PLANE) {
        capabilityCursorPlane = true;
        Q_EMIT q->capabilityCursorPlaneChanged(capabilityCursorPlane);
    }
}

FullscreenShell::FullscreenShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

FullscreenShell::~FullscreenShell()
{
    release();
}

void FullscreenShell::release()
{
    d->shell.release();
}

void FullscreenShell::destroy()
{
    d->shell.destroy();
}

void FullscreenShell::setup(_wl_fullscreen_shell *shell)
{
    d->setup(shell);
}

EventQueue *FullscreenShell::eventQueue() const
{
    return d->queue;
}

void FullscreenShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void FullscreenShell::present(wl_surface *surface, wl_output *output)
{
    Q_ASSERT(d->shell);
    _wl_fullscreen_shell_present_surface(d->shell, surface, _WL_FULLSCREEN_SHELL_PRESENT_METHOD_DEFAULT, output);
}

void FullscreenShell::present(Surface *surface, Output *output)
{
    Q_ASSERT(surface);
    Q_ASSERT(output);
    present(*surface, *output);
}

bool FullscreenShell::isValid() const
{
    return d->shell.isValid();
}

bool FullscreenShell::hasCapabilityArbitraryModes() const
{
    return d->capabilityArbitraryModes;
}

bool FullscreenShell::hasCapabilityCursorPlane() const
{
    return d->capabilityCursorPlane;
}

}
}
