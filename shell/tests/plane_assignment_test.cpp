// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane_manager.cc
//           (IsCompatible, AssignOverlayPlanes)
//           chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane_manager_unittest.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Plane compatibility and assignment.
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <gtest/gtest.h>

#include <drm_fourcc.h>
#include <xf86drmMode.h>

#include "backend/drm/drm_plane.h"
#include "backend/drm/plane_assignment.h"

namespace Lingmo::Backend {

namespace {

constexpr uint32_t kXrgb = DRM_FORMAT_XRGB8888;
constexpr uint32_t kNv12 = DRM_FORMAT_NV12;
constexpr uint64_t kTiled = 0x0100000000000001ull;

DrmPlane makePlane(uint32_t id, uint32_t type, uint32_t possibleCrtcs,
                   std::map<uint32_t, std::vector<uint64_t>> formats)
{
    DrmPlane plane;
    plane.id = id;
    plane.type = type;
    plane.possibleCrtcs = possibleCrtcs;
    plane.formats = std::move(formats);
    return plane;
}

// A primary plane and two overlays, all on CRTC index 0.
std::vector<DrmPlane> typicalPlanes()
{
    return {
        makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR, kTiled}}}),
        makePlane(20, DRM_PLANE_TYPE_OVERLAY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}}),
        makePlane(30, DRM_PLANE_TYPE_OVERLAY, 0b1,
                  {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}, {kNv12, {DRM_FORMAT_MOD_LINEAR}}}),
        makePlane(40, DRM_PLANE_TYPE_CURSOR, 0b1, {{DRM_FORMAT_ARGB8888, {}}}),
    };
}

PlaneRequest primary(uint32_t format = kXrgb, uint64_t modifier = DRM_FORMAT_MOD_LINEAR)
{
    return PlaneRequest {format, modifier, true};
}

PlaneRequest overlay(uint32_t format = kXrgb, uint64_t modifier = DRM_FORMAT_MOD_LINEAR)
{
    return PlaneRequest {format, modifier, false};
}

} // namespace

TEST(DrmPlaneTest, AFormatThePlaneDoesNotListIsUnsupported)
{
    const DrmPlane plane =
        makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}});

    EXPECT_TRUE(plane.supportsFormat(kXrgb, DRM_FORMAT_MOD_LINEAR));
    EXPECT_FALSE(plane.supportsFormat(kNv12, DRM_FORMAT_MOD_LINEAR));
}

TEST(DrmPlaneTest, AModifierThePlaneDoesNotListIsUnsupported)
{
    const DrmPlane plane =
        makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}});

    EXPECT_FALSE(plane.supportsFormat(kXrgb, kTiled));
}

TEST(DrmPlaneTest, AnImplicitModifierIsLeftToTheDriver)
{
    const DrmPlane plane = makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {kTiled}}});

    // Only the driver knows what an implicitly modified buffer really is, so
    // the answer comes from the atomic test rather than from here.
    EXPECT_TRUE(plane.supportsFormat(kXrgb, DRM_FORMAT_MOD_INVALID));
}

TEST(DrmPlaneTest, WithoutAnInFormatsBlobOnlyImplicitModifiersAreAccepted)
{
    const DrmPlane plane = makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {}}});

    EXPECT_TRUE(plane.supportsFormat(kXrgb, DRM_FORMAT_MOD_INVALID));
    EXPECT_TRUE(plane.supportsFormat(kXrgb, DRM_FORMAT_MOD_LINEAR));
    EXPECT_FALSE(plane.supportsFormat(kXrgb, kTiled));
}

TEST(DrmPlaneTest, APlaneOnAnotherCrtcIsUnusable)
{
    const DrmPlane plane = makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b10, {{kXrgb, {}}});

    EXPECT_FALSE(plane.canUseForCrtc(0));
    EXPECT_TRUE(plane.canUseForCrtc(1));
}

TEST(PlaneAssignmentTest, ThePrimaryPlaneTakesTheZeroOrderRequest)
{
    const auto assignments = assignPlanes(typicalPlanes(), 0, {primary()});

    ASSERT_EQ(assignments.size(), 1u);
    EXPECT_EQ(assignments[0].requestIndex, 0u);
    EXPECT_EQ(assignments[0].planeId, 10u);
}

TEST(PlaneAssignmentTest, OverlaysTakeOverlayPlanesInOrder)
{
    const auto assignments = assignPlanes(typicalPlanes(), 0, {primary(), overlay(), overlay()});

    ASSERT_EQ(assignments.size(), 3u);
    EXPECT_EQ(assignments[0].planeId, 10u);
    // Ascending plane ids preserve the z order the caller asked for.
    EXPECT_EQ(assignments[1].planeId, 20u);
    EXPECT_EQ(assignments[2].planeId, 30u);
}

TEST(PlaneAssignmentTest, MoreOverlaysThanPlanesFails)
{
    const auto assignments =
        assignPlanes(typicalPlanes(), 0, {primary(), overlay(), overlay(), overlay()});

    // A partial assignment is never useful: the caller would have to composite
    // the leftovers and plan again from scratch.
    EXPECT_TRUE(assignments.empty());
}

TEST(PlaneAssignmentTest, AnUnsupportedFormatIsRejected)
{
    // Only plane 30 handles NV12, and it cannot be the primary.
    EXPECT_TRUE(assignPlanes(typicalPlanes(), 0, {primary(kNv12)}).empty());

    const auto assignments = assignPlanes(typicalPlanes(), 0, {primary(), overlay(kNv12)});
    ASSERT_EQ(assignments.size(), 2u);
    EXPECT_EQ(assignments[1].planeId, 30u);
}

TEST(PlaneAssignmentTest, AnUnsupportedModifierIsRejected)
{
    // The primary plane takes the tiled buffer; no overlay plane does.
    EXPECT_FALSE(assignPlanes(typicalPlanes(), 0, {primary(kXrgb, kTiled)}).empty());
    EXPECT_TRUE(
        assignPlanes(typicalPlanes(), 0, {primary(), overlay(kXrgb, kTiled)}).empty());
}

TEST(PlaneAssignmentTest, TheCursorPlaneIsNeverHandedOut)
{
    // The cursor is driven through the legacy cursor ioctls, so its plane must
    // stay out of the overlay pool even when the format would fit.
    const std::vector<DrmPlane> planes = {
        makePlane(10, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}}),
        makePlane(40, DRM_PLANE_TYPE_CURSOR, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}}),
    };

    EXPECT_TRUE(assignPlanes(planes, 0, {primary(), overlay()}).empty());
}

TEST(PlaneAssignmentTest, PlanesBelongingToAnotherCrtcAreNotUsed)
{
    EXPECT_TRUE(assignPlanes(typicalPlanes(), 1, {primary()}).empty());
}

TEST(PlaneAssignmentTest, APrimaryPlaneAfterTheOverlaysIsStillFound)
{
    // possible_crtcs and plane ids are driver-assigned; the primary is not
    // guaranteed to come first, and reaching it must not strand the overlays.
    const std::vector<DrmPlane> planes = {
        makePlane(10, DRM_PLANE_TYPE_OVERLAY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}}),
        makePlane(20, DRM_PLANE_TYPE_PRIMARY, 0b1, {{kXrgb, {DRM_FORMAT_MOD_LINEAR}}}),
    };

    const auto assignments = assignPlanes(planes, 0, {primary(), overlay()});

    ASSERT_EQ(assignments.size(), 2u);
    EXPECT_EQ(assignments[0].planeId, 20u);
    EXPECT_EQ(assignments[1].planeId, 10u);
}

TEST(PlaneAssignmentTest, NoRequestsAssignNoPlanes)
{
    EXPECT_TRUE(assignPlanes(typicalPlanes(), 0, {}).empty());
}

} // namespace Lingmo::Backend
