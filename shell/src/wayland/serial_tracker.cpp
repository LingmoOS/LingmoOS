// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/serial_tracker.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "serial_tracker.h"

#include <wayland-server-core.h>

#include <sstream>

#include "core/logging.h"

namespace Lingmo::Wayland {

namespace {

// Number of previous events to retain information about.
constexpr uint32_t kMaxEventsTracked = 1024;

} // namespace

std::string SerialTracker::toString(EventType type)
{
    switch (type) {
    case POINTER_ENTER:
        return "pointer_enter";
    case POINTER_LEAVE:
        return "pointer_leave";
    case POINTER_LEFT_BUTTON_DOWN:
        return "left_button_down";
    case POINTER_LEFT_BUTTON_UP:
        return "left_button_up";
    case POINTER_MIDDLE_BUTTON_DOWN:
        return "middle_button_down";
    case POINTER_MIDDLE_BUTTON_UP:
        return "middle_button_up";
    case POINTER_RIGHT_BUTTON_DOWN:
        return "right_button_down";
    case POINTER_RIGHT_BUTTON_UP:
        return "right_button_up";
    case POINTER_FORWARD_BUTTON_DOWN:
        return "forward_button_down";
    case POINTER_FORWARD_BUTTON_UP:
        return "forward_button_up";
    case POINTER_BACK_BUTTON_DOWN:
        return "back_button_down";
    case POINTER_BACK_BUTTON_UP:
        return "back_button_up";
    case TOUCH_DOWN:
        return "touch_down";
    case TOUCH_UP:
        return "touch_up";
    case OTHER_EVENT:
        return "other event";
    }
    return "unknown";
}

SerialTracker::SerialTracker(wl_display *display)
    : m_display(display)
    , m_events(kMaxEventsTracked)
{
}

SerialTracker::~SerialTracker() = default;

void SerialTracker::shutdown()
{
    m_display = nullptr;
    for (auto &entry : m_activationSerials)
        entry.recipient.reset(nullptr);
}

uint32_t SerialTracker::nextSerial(EventType type)
{
    if (!m_display)
        return 0;

    uint32_t serial = wl_display_next_serial(m_display);
    m_events[serial % kMaxEventsTracked] = type;
    m_maxEvent = serial + 1;
    if ((m_maxEvent - m_minEvent) > kMaxEventsTracked)
        m_minEvent = m_maxEvent - kMaxEventsTracked;

    return serial;
}

std::optional<SerialTracker::EventType> SerialTracker::eventType(uint32_t serial) const
{
    if (m_maxEvent < m_minEvent) {
        // The valid range has partially overflowed the 32 bit space, so we
        // should only reject if the serial number is in neither the upper nor
        // lower parts of the space.
        if (!((serial < m_maxEvent) || (serial >= m_minEvent))) {
            LM_WARNING(lingmoWayland) << "No event type for serial" << serial
                                  << "- it is in the void range";
            return std::nullopt;
        }
    } else {
        // Normal, non-overflowed case. Reject the serial number if it isn't in
        // the interval.
        if (!((serial < m_maxEvent) && (serial >= m_minEvent))) {
            LM_WARNING(lingmoWayland) << "No event type for serial" << serial
                                  << "- it is outside of the range";
            return std::nullopt;
        }
    }

    return m_events[serial % kMaxEventsTracked];
}

uint32_t SerialTracker::maybeNextKeySerial()
{
    if (!m_keySerial.has_value())
        m_keySerial = nextSerial(OTHER_EVENT);
    return m_keySerial.value();
}

void SerialTracker::resetKeySerial()
{
    m_keySerial = std::nullopt;
}

void SerialTracker::recordActivationSerial(uint32_t serial, wl_resource *recipient)
{
    auto &entry = m_activationSerials[serial % m_activationSerials.size()];
    entry.recipient.reset(recipient);
    entry.serial = serial;
    entry.time = std::chrono::steady_clock::now();
}

bool SerialTracker::consumeActivationSerial(uint32_t serial, wl_client *client)
{
    auto &entry = m_activationSerials[serial % m_activationSerials.size()];
    if (!m_display || entry.serial != serial || !entry.recipient.get()
        || wl_resource_get_client(entry.recipient.get()) != client
        || std::chrono::steady_clock::now() - entry.time > std::chrono::seconds(5))
        return false;
    entry.recipient.reset(nullptr);
    return true;
}

std::string SerialTracker::toString() const
{
    std::ostringstream ss;
    ss << "min=" << m_minEvent << ", max=" << m_maxEvent;
    return ss.str();
}

} // namespace Lingmo::Wayland
