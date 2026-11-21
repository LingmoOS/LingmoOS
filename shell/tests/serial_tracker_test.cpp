// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from chromium/src/components/exo/wayland/serial_tracker_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// The upstream test drives the tracker through a full Exo server and an event
// generator. This one exercises the same semantics directly against a bare
// wl_display.

#include "wayland/serial_tracker.h"

#include <gtest/gtest.h>
#include <wayland-server-core.h>

namespace Lingmo::Wayland {
namespace {

constexpr uint32_t kMaxEventsTracked = 1024;

class SerialTrackerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_display = wl_display_create();
        ASSERT_TRUE(m_display);
        m_tracker = std::make_unique<SerialTracker>(m_display);
    }

    void TearDown() override
    {
        m_tracker.reset();
        wl_display_destroy(m_display);
    }

    wl_display* m_display = nullptr;
    std::unique_ptr<SerialTracker> m_tracker;
};

TEST_F(SerialTrackerTest, SerialsAreSequentialAndCarryTheirEventType)
{
    const uint32_t down = m_tracker->nextSerial(SerialTracker::POINTER_LEFT_BUTTON_DOWN);
    const uint32_t up = m_tracker->nextSerial(SerialTracker::POINTER_LEFT_BUTTON_UP);
    const uint32_t touch = m_tracker->nextSerial(SerialTracker::TOUCH_DOWN);

    EXPECT_EQ(down + 1, up);
    EXPECT_EQ(up + 1, touch);

    EXPECT_EQ(SerialTracker::POINTER_LEFT_BUTTON_DOWN, m_tracker->eventType(down));
    EXPECT_EQ(SerialTracker::POINTER_LEFT_BUTTON_UP, m_tracker->eventType(up));
    EXPECT_EQ(SerialTracker::TOUCH_DOWN, m_tracker->eventType(touch));
}

TEST_F(SerialTrackerTest, ButtonsAreTrackedSeparately)
{
    const uint32_t left = m_tracker->nextSerial(SerialTracker::POINTER_LEFT_BUTTON_DOWN);
    const uint32_t middle = m_tracker->nextSerial(SerialTracker::POINTER_MIDDLE_BUTTON_DOWN);
    const uint32_t right = m_tracker->nextSerial(SerialTracker::POINTER_RIGHT_BUTTON_DOWN);

    EXPECT_EQ(SerialTracker::POINTER_LEFT_BUTTON_DOWN, m_tracker->eventType(left));
    EXPECT_EQ(SerialTracker::POINTER_MIDDLE_BUTTON_DOWN, m_tracker->eventType(middle));
    EXPECT_EQ(SerialTracker::POINTER_RIGHT_BUTTON_DOWN, m_tracker->eventType(right));

    // Releasing one button leaves the others' serials untouched.
    const uint32_t leftUp = m_tracker->nextSerial(SerialTracker::POINTER_LEFT_BUTTON_UP);
    EXPECT_EQ(SerialTracker::POINTER_LEFT_BUTTON_UP, m_tracker->eventType(leftUp));
    EXPECT_EQ(SerialTracker::POINTER_MIDDLE_BUTTON_DOWN, m_tracker->eventType(middle));
    EXPECT_EQ(SerialTracker::POINTER_RIGHT_BUTTON_DOWN, m_tracker->eventType(right));
}

TEST_F(SerialTrackerTest, UnissuedSerialsAreRejected)
{
    // Nothing has been issued yet, so the valid range is empty.
    EXPECT_EQ(std::nullopt, m_tracker->eventType(0));
    EXPECT_EQ(std::nullopt, m_tracker->eventType(1));

    const uint32_t serial = m_tracker->nextSerial(SerialTracker::OTHER_EVENT);
    EXPECT_NE(std::nullopt, m_tracker->eventType(serial));
    // Not issued yet.
    EXPECT_EQ(std::nullopt, m_tracker->eventType(serial + 1));
    // Predates the first serial.
    EXPECT_EQ(std::nullopt, m_tracker->eventType(serial - 1));
}

TEST_F(SerialTrackerTest, OldSerialsFallOutOfTheTrackedWindow)
{
    const uint32_t first = m_tracker->nextSerial(SerialTracker::TOUCH_DOWN);
    for (uint32_t i = 0; i < kMaxEventsTracked; ++i)
        m_tracker->nextSerial(SerialTracker::OTHER_EVENT);

    // The circular buffer has wrapped past `first`.
    EXPECT_EQ(std::nullopt, m_tracker->eventType(first));

    const uint32_t last = m_tracker->nextSerial(SerialTracker::TOUCH_UP);
    EXPECT_EQ(SerialTracker::TOUCH_UP, m_tracker->eventType(last));
    EXPECT_EQ(SerialTracker::OTHER_EVENT, m_tracker->eventType(last - 1));
}

TEST_F(SerialTrackerTest, KeySerialIsReusedUntilReset)
{
    const uint32_t key = m_tracker->maybeNextKeySerial();
    EXPECT_EQ(key, m_tracker->maybeNextKeySerial());
    EXPECT_EQ(SerialTracker::OTHER_EVENT, m_tracker->eventType(key));

    m_tracker->resetKeySerial();
    const uint32_t newKey = m_tracker->maybeNextKeySerial();
    EXPECT_NE(key, newKey);
    EXPECT_EQ(newKey, m_tracker->maybeNextKeySerial());
}

TEST_F(SerialTrackerTest, ShutdownStopsIssuingSerials)
{
    EXPECT_NE(0u, m_tracker->nextSerial(SerialTracker::OTHER_EVENT));
    m_tracker->shutdown();
    EXPECT_EQ(0u, m_tracker->nextSerial(SerialTracker::OTHER_EVENT));
}

TEST_F(SerialTrackerTest, ToStringCoversEveryEventType)
{
    EXPECT_EQ("pointer_enter", SerialTracker::toString(SerialTracker::POINTER_ENTER));
    EXPECT_EQ("left_button_down", SerialTracker::toString(SerialTracker::POINTER_LEFT_BUTTON_DOWN));
    EXPECT_EQ("touch_up", SerialTracker::toString(SerialTracker::TOUCH_UP));
    EXPECT_EQ("other event", SerialTracker::toString(SerialTracker::OTHER_EVENT));

    m_tracker->nextSerial(SerialTracker::OTHER_EVENT);
    EXPECT_EQ("min=1, max=2", m_tracker->toString());
}

} // namespace
} // namespace Lingmo::Wayland
