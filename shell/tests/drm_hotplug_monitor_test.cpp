// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/host/drm_display_host_manager_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests the bounded hotplug debounce.

#include <gtest/gtest.h>

#include "backend/drm/drm_hotplug_monitor.h"
#include "loop_test_util.h"

using namespace Lingmo;

TEST(DrmHotplugMonitor, CoalescesEventBurst)
{
    backend::DrmHotplugMonitor monitor;
    int requested = 0;
    const auto subscription = monitor.reprobeRequested.connect([&] { ++requested; });
    monitor.notifyForTest();
    monitor.notifyForTest();
    monitor.notifyForTest();
    ASSERT_TRUE(test::runUntil([&] { return requested > 0; }, std::chrono::milliseconds(500)));
    test::runFor(std::chrono::milliseconds(800));
    EXPECT_EQ(requested, 1);
}
