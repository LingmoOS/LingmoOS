// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_pointer_delegate.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WAYLAND_POINTER_DELEGATE_H_
#define LINGMO_WAYLAND_PROTOCOLS_WAYLAND_POINTER_DELEGATE_H_

#include <array>
#include <optional>
#include <wayland-server-core.h>

#include "clipboard/selection.h"
#include "input/pointer_delegate.h"

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::Wayland {

class SerialTracker;

// Pointer delegate that accepts events for surfaces owned by the same client as
// the wl_pointer resource.
class WaylandPointerDelegate : public input::PointerDelegate
{
public:
    WaylandPointerDelegate(wl_resource* pointerResource, SerialTracker* serialTracker,
                           clipboard::SelectionController* selection = nullptr,
                           input::Seat* seat = nullptr);
    WaylandPointerDelegate(const WaylandPointerDelegate&) = delete;
    WaylandPointerDelegate& operator=(const WaylandPointerDelegate&) = delete;
    ~WaylandPointerDelegate() override;

    // input::PointerDelegate:
    void onPointerDestroying(input::Pointer* pointer) override;
    bool canAcceptPointerEventsForSurface(compositor::Surface* surface) const override;
    void onPointerEnter(compositor::Surface* surface, const base::PointF& location,
                        input::ButtonFlags pressedButtonFlags) override;
    void onPointerLeave(compositor::Surface* surface) override;
    void onPointerMotion(input::Timestamp timestamp, const base::PointF& location) override;
    void onPointerButton(input::Timestamp timestamp, input::ButtonFlags changedButtonFlags,
                         bool pressed) override;
    void onPointerScroll(input::Timestamp timestamp, const base::PointF& offset,
                         bool discrete) override;
    void onFingerScrollStop(input::Timestamp timestamp) override;
    void onPointerFrame() override;

    bool acceptsCursorSerial(uint32_t serial) const { return m_enterSerial == serial; }

private:
    input::Seat* m_seat;
    std::optional<uint32_t> m_enterSerial;
    wl_client* client() const;
    void sendWheelSteps(uint32_t axis, double value);

    // Fractions of a wheel click not yet reported to a pre-version-8 client,
    // indexed by wl_pointer_axis.
    std::array<int, 2> m_pendingValue120 {};

    wl_resource* const m_pointerResource;
    SerialTracker* const m_serialTracker;
    base::WeakPtr<clipboard::SelectionController> m_selection;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WAYLAND_POINTER_DELEGATE_H_
