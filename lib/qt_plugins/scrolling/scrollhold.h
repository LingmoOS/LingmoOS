// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_SCROLLHOLD_H
#define LINGMO_SCROLLHOLD_H

#include <QObject>

#include <cstdint>

struct wl_pointer;
struct wl_registry;
struct wl_surface;
struct zwp_pointer_gesture_hold_v1;
struct zwp_pointer_gestures_v1;

namespace Lingmo {

// Fingers resting on the touchpad stop the coasts in the window under the
// pointer, before they move far enough to start a scroll.
class ScrollHold : public QObject
{
    Q_OBJECT

public:
    static void watch();
    ~ScrollHold() override;

private:
    explicit ScrollHold(QObject *parent = nullptr);
    void updatePointer();
    static void global(void *data, wl_registry *registry, uint32_t name, const char *interface,
                       uint32_t version);
    static void globalRemove(void *data, wl_registry *, uint32_t name);
    static void begin(void *data, zwp_pointer_gesture_hold_v1 *, uint32_t serial, uint32_t time,
                      wl_surface *surface, uint32_t fingers);
    static void end(void *data, zwp_pointer_gesture_hold_v1 *, uint32_t serial, uint32_t time,
                    int32_t cancelled);

    wl_registry *m_registry = nullptr;
    zwp_pointer_gestures_v1 *m_gestures = nullptr;
    zwp_pointer_gesture_hold_v1 *m_hold = nullptr;
    wl_pointer *m_pointer = nullptr;
    uint32_t m_name = 0;
};

} // namespace Lingmo

#endif // LINGMO_SCROLLHOLD_H
