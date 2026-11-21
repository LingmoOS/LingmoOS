// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/screen_manager_unittest.cc
//           chromium/src/ui/ozone/platform/drm/gpu/drm_gpu_display_manager_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: tests cover the device-wide route planner, bounded
// MST mode fallback, and atomic commit publication boundary.

#include <gtest/gtest.h>
#include <xf86drmMode.h>

#include "backend/drm/drm_commit_coordinator.h"
#include "backend/drm/drm_configuration_planner.h"
#include "backend/drm/drm_mode_fallback_planner.h"
#include "fake_drm_device.h"

using namespace Lingmo;

TEST(DrmConfigurationPlanner, BacktracksAcrossAsymmetricCrtcCompatibility)
{
    const std::vector<backend::DrmRouteCandidate> routes = {{10, 0b11, 100}, {20, 0b01, 0}};
    const std::vector<backend::DrmCrtcCandidate> crtcs = {{100, 0, {1000}}, {200, 1, {2000}}};
    const auto plan = backend::DrmConfigurationPlanner::assign(routes, crtcs);
    ASSERT_TRUE(plan);
    ASSERT_EQ(plan->size(), 2);
    EXPECT_EQ(plan->at(0).crtcId, 200u);
    EXPECT_EQ(plan->at(1).crtcId, 100u);
    EXPECT_NE(plan->at(0).primaryPlaneId, plan->at(1).primaryPlaneId);
}

TEST(DrmModeFallbackPlanner, DowngradesOnlyTheFailedMstBranchWithinABound)
{
    using Mode = backend::DrmModeCandidate;
    const std::vector<backend::DrmFallbackHead> heads = {
            {10,
             "eDP-1",
             "internal",
             true,
             {Mode {"panel", 2560, 1600, 60000, 250000, true}}},
            {20,
             "DP-1-1",
             "DP-1",
             false,
             {Mode {"4k", 3840, 2160, 60000, 600000, true},
              Mode {"1080p", 1920, 1080, 60000, 148500, false}}},
            {30,
             "HDMI-A-1",
             "HDMI-A-1",
             false,
             {Mode {"1440p", 2560, 1440, 60000, 241500, true},
              Mode {"720p", 1280, 720, 60000, 74250, false}}}};
    const auto plan = backend::DrmModeFallbackPlanner::choose(
            heads,
            [](const std::vector<Mode> &modes) {
                if (modes.at(1).pixelClockKHz > 200000)
                    return backend::DrmFallbackTestResult {
                            false, {"DP-1"}, "MST link bandwidth"};
                return backend::DrmFallbackTestResult {true, {}, "tested"};
            },
            4);
    ASSERT_TRUE(plan);
    EXPECT_EQ(plan->modes.at(0).key, "panel");
    EXPECT_EQ(plan->modes.at(1).key, "1080p");
    EXPECT_EQ(plan->modes.at(2).key, "1440p");
    EXPECT_EQ(plan->downgradedConnectors, std::vector<std::string> {"DP-1-1"});
    EXPECT_LE(plan->attempts, 4);
}

TEST(DrmConfigurationPlanner, RejectsSharedOrInsufficientResources)
{
    const std::vector<backend::DrmRouteCandidate> routes = {{10, 0b11, 0}, {20, 0b11, 0}};
    const std::vector<backend::DrmCrtcCandidate> crtcs = {{100, 0, {1000}}, {200, 1, {1000}}};
    EXPECT_FALSE(backend::DrmConfigurationPlanner::assign(routes, crtcs));
}

TEST(DrmCommitCoordinator, TestAndCommitAreAtomicFromPublishedStatesPerspective)
{
    Lingmo::testing::FakeDrmDevice device;
    backend::DrmCommitCoordinator coordinator(&device);
    backend::AtomicRequest first;
    first.add(1, 2, 3);
    ASSERT_TRUE(coordinator.testAndCommit(first, DRM_MODE_ATOMIC_ALLOW_MODESET));
    EXPECT_EQ(coordinator.revision(), 1u);
    ASSERT_TRUE(coordinator.appliedRequest());

    backend::AtomicRequest rejected;
    rejected.add(4, 5, 6);
    device.commitFilter = [](const auto &commit) {
        return !(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    EXPECT_FALSE(coordinator.testAndCommit(rejected, DRM_MODE_ATOMIC_ALLOW_MODESET));
    EXPECT_EQ(coordinator.revision(), 1u);
    EXPECT_EQ(coordinator.appliedRequest()->properties.front().objectId, 1u);
    EXPECT_EQ(device.realCommitCount(), 1u);
}

TEST(DrmCommitCoordinator, FailedRealCommitDoesNotPublishCandidate)
{
    Lingmo::testing::FakeDrmDevice device;
    backend::DrmCommitCoordinator coordinator(&device);
    backend::AtomicRequest request;
    request.add(1, 2, 3);
    device.commitFilter = [](const auto &commit) {
        return static_cast<bool>(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    EXPECT_FALSE(coordinator.testAndCommit(request));
    EXPECT_FALSE(coordinator.appliedRequest());
    EXPECT_EQ(coordinator.revision(), 0u);
    EXPECT_EQ(device.testCommitCount(), 1u);
    EXPECT_EQ(device.realCommitCount(), 1u);
}

TEST(DrmCommitCoordinator, InvalidationAfterTestKeepsThePreviousRequest)
{
    Lingmo::testing::FakeDrmDevice device;
    backend::DrmCommitCoordinator coordinator(&device);
    backend::AtomicRequest first;
    first.add(1, 2, 3);
    ASSERT_TRUE(coordinator.testAndCommit(first));
    bool valid = true;
    device.commitFilter = [&](const auto &) {
        valid = false;
        return true;
    };
    backend::AtomicRequest next;
    next.add(1, 2, 4);
    EXPECT_EQ(coordinator.execute(next, false, 0, nullptr,
                                  [&] {
                                      return valid;
                                  }),
              backend::DrmCommitStatus::Invalidated);
    EXPECT_EQ(device.realCommitCount(), 1u);
    EXPECT_EQ(coordinator.revision(), 1u);
    EXPECT_EQ(coordinator.appliedRequest()->properties.front().value, 3u);
}

TEST(DrmCommitCoordinator, RejectsConflictingPropertiesBeforeTesting)
{
    Lingmo::testing::FakeDrmDevice device;
    backend::DrmCommitCoordinator coordinator(&device);
    backend::AtomicRequest request;
    request.add(1, 2, 3);
    request.add(1, 2, 4);
    EXPECT_EQ(coordinator.execute(request, false), backend::DrmCommitStatus::InvalidRequest);
    EXPECT_TRUE(device.commits.empty());
}

TEST(DrmCommitCoordinator, TestOnlyDoesNotPublishAndStripsPageFlipEventFlag)
{
    Lingmo::testing::FakeDrmDevice device;
    backend::DrmCommitCoordinator coordinator(&device);
    backend::AtomicRequest request;
    request.add(1, 2, 3);
    EXPECT_EQ(coordinator.execute(request, true, DRM_MODE_PAGE_FLIP_EVENT),
              backend::DrmCommitStatus::Success);
    EXPECT_FALSE(coordinator.appliedRequest());
    ASSERT_EQ(device.commits.size(), 1u);
    EXPECT_FALSE(device.commits.front().flags & DRM_MODE_PAGE_FLIP_EVENT);
    EXPECT_EQ(device.commits.front().userData, nullptr);
}

TEST(DrmConfigurationPlanner, EnumeratesDistinctResourceAlternativesWithinTheBound)
{
    const std::vector<backend::DrmRouteCandidate> routes = {{10, 0b11, 100}, {20, 0b11, 200}};
    const std::vector<backend::DrmCrtcCandidate> crtcs = {{100, 0, {1000}}, {200, 1, {2000}}};
    const auto first = backend::DrmConfigurationPlanner::assign(routes, crtcs, 0);
    const auto next = backend::DrmConfigurationPlanner::assign(routes, crtcs, 1);
    ASSERT_TRUE(first);
    ASSERT_TRUE(next);
    EXPECT_EQ(first->front().crtcId, 100u);
    EXPECT_EQ(next->front().crtcId, 200u);
    EXPECT_NE(next->front().primaryPlaneId, next->back().primaryPlaneId);
    EXPECT_FALSE(backend::DrmConfigurationPlanner::assign(routes, crtcs, 2));
    EXPECT_FALSE(backend::DrmConfigurationPlanner::assign(routes, crtcs, 8));
}
