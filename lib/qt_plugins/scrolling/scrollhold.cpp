// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollhold.h"
#include "scroller.h"

#include <QGuiApplication>
#include <QWindow>
#include <QtGui/qguiapplication_platform.h>
#include <qpa/qplatformnativeinterface.h>
#include <wayland-client.h>

#include "pointer-gestures-unstable-v1-client-protocol.h"

#include <cstring>

namespace Lingmo {

static wl_surface *surfaceOf(QWindow *window)
{
    if (!window->handle())
        return nullptr;
    auto *native = qGuiApp->platformNativeInterface();
    return native ? static_cast<wl_surface *>(
                            native->nativeResourceForWindow(QByteArrayLiteral("surface"), window))
                  : nullptr;
}

void ScrollHold::watch()
{
    static ScrollHold *hold = nullptr;
    if (!hold && qGuiApp && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        hold = new ScrollHold(qGuiApp);
}

ScrollHold::ScrollHold(QObject *parent)
    : QObject(parent)
{
    auto *native = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    m_registry = wl_display_get_registry(native->display());
    static const wl_registry_listener listener = {global, globalRemove};
    wl_registry_add_listener(m_registry, &listener, this);
    wl_display_flush(native->display());
    // Qt replaces its wl_pointer when the seat loses and regains one.
    connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &ScrollHold::updatePointer);
}

ScrollHold::~ScrollHold()
{
    if (m_hold)
        zwp_pointer_gesture_hold_v1_destroy(m_hold);
    if (m_gestures)
        zwp_pointer_gestures_v1_release(m_gestures);
    if (m_registry)
        wl_registry_destroy(m_registry);
}

void ScrollHold::global(void *data, wl_registry *registry, uint32_t name, const char *interface,
                         uint32_t version)
{
    auto *self = static_cast<ScrollHold *>(data);
    if (std::strcmp(interface, zwp_pointer_gestures_v1_interface.name) != 0 || version < 3
        || self->m_gestures)
        return;
    self->m_gestures = static_cast<zwp_pointer_gestures_v1 *>(
            wl_registry_bind(registry, name, &zwp_pointer_gestures_v1_interface, 3));
    self->m_name = name;
    self->updatePointer();
}

void ScrollHold::globalRemove(void *data, wl_registry *, uint32_t name)
{
    auto *self = static_cast<ScrollHold *>(data);
    if (name != self->m_name || !self->m_gestures)
        return;
    if (self->m_hold)
        zwp_pointer_gesture_hold_v1_destroy(self->m_hold);
    zwp_pointer_gestures_v1_release(self->m_gestures);
    self->m_hold = nullptr;
    self->m_gestures = nullptr;
    self->m_pointer = nullptr;
    self->m_name = 0;
}

void ScrollHold::updatePointer()
{
    auto *native = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    wl_pointer *pointer = native ? native->pointer() : nullptr;
    if (!m_gestures || pointer == m_pointer)
        return;

    if (m_hold)
        zwp_pointer_gesture_hold_v1_destroy(m_hold);
    m_hold = nullptr;
    m_pointer = pointer;
    if (!m_pointer)
        return;
    m_hold = zwp_pointer_gestures_v1_get_hold_gesture(m_gestures, m_pointer);
    static const zwp_pointer_gesture_hold_v1_listener listener = {begin, end};
    zwp_pointer_gesture_hold_v1_add_listener(m_hold, &listener, this);
}

void ScrollHold::begin(void *, zwp_pointer_gesture_hold_v1 *, uint32_t, uint32_t,
                        wl_surface *surface, uint32_t)
{
    if (!surface)
        return;
    for (QWindow *window : QGuiApplication::allWindows()) {
        if (surfaceOf(window) == surface) {
            Scroller::stopCoasts(window);
            return;
        }
    }
}

void ScrollHold::end(void *, zwp_pointer_gesture_hold_v1 *, uint32_t, uint32_t, int32_t)
{
}

} // namespace Lingmo
