// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/serial_tracker.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_SERIAL_TRACKER_H_
#define LINGMO_WAYLAND_SERIAL_TRACKER_H_

#include <array>
#include <chrono>
#include <cstdint>

#include "wayland/resource_reference.h"
#include <optional>
#include <string>
#include <vector>

struct wl_display;

namespace Lingmo::Wayland {

class SerialTracker
{
public:
    enum EventType {
        POINTER_ENTER,
        POINTER_LEAVE,
        POINTER_LEFT_BUTTON_DOWN,
        POINTER_LEFT_BUTTON_UP,
        POINTER_MIDDLE_BUTTON_DOWN,
        POINTER_MIDDLE_BUTTON_UP,
        POINTER_RIGHT_BUTTON_DOWN,
        POINTER_RIGHT_BUTTON_UP,
        POINTER_FORWARD_BUTTON_DOWN,
        POINTER_FORWARD_BUTTON_UP,
        POINTER_BACK_BUTTON_DOWN,
        POINTER_BACK_BUTTON_UP,
        TOUCH_DOWN,
        TOUCH_UP,
        OTHER_EVENT,
    };

    static std::string toString(EventType type);

    explicit SerialTracker(wl_display *display);
    SerialTracker(const SerialTracker &) = delete;
    SerialTracker &operator=(const SerialTracker &) = delete;
    ~SerialTracker();

    // After shutdown, nextSerial() returns 0.
    void shutdown();

    uint32_t nextSerial(EventType type);

    // If there exists a serial for key already, returns it. Or, it creates
    // a new serial, and returns it.
    uint32_t maybeNextKeySerial();

    // Resets the stored key serial, so that the next maybeNextKeySerial() call
    // will generate a new serial.
    void resetKeySerial();

    // Get the EventType for a serial number, or nullopt if the serial number
    // was never sent or is too old.
    std::optional<EventType> eventType(uint32_t serial) const;

    std::string toString() const;

    void recordActivationSerial(uint32_t serial, wl_resource *recipient);
    bool consumeActivationSerial(uint32_t serial, wl_client *client);

private:
    struct ActivationSerial {
        ResourceReference recipient;
        uint32_t serial = 0;
        std::chrono::steady_clock::time_point time;
    };

    std::array<ActivationSerial, 1024> m_activationSerials;
    wl_display *m_display;

    // EventTypes are stored in a circular buffer, because serial numbers are
    // issued sequentially and we only want to store the most recent events.
    std::vector<EventType> m_events;

    // [m_minEvent, m_maxEvent) is a half-open interval containing the range of
    // valid serial numbers. Note that as serial numbers are allowed to wrap
    // around the 32 bit space, we cannot assume that m_maxEvent >= m_minEvent.
    uint32_t m_minEvent = 1;
    uint32_t m_maxEvent = 1;

    std::optional<uint32_t> m_keySerial;
};

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_SERIAL_TRACKER_H_
