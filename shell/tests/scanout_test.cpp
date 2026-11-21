// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_overlay_validator.cc (TestPageFlip)
//           chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane_manager_atomic.cc
//           (Commit)
//           chromium/src/ui/ozone/platform/drm/gpu/drm_framebuffer.cc (AddFramebuffer)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// The display-controller half of direct scanout: importing client DMA-BUFs into
// DRM framebuffers, validating plane configurations with an atomic TEST_ONLY,
// and falling back to composition whenever any of that does not hold.
//
// Adapted for Lingmo: exercises the corresponding behavior with native
// compositor test doubles and APIs.

#include <algorithm>
#include <cmath>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <gbm.h>
#include <gtest/gtest.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "backend/drm/drm_framebuffer.h"
#include "backend/drm/drm_output.h"
#include "backend/drm/drm_plane.h"
#include "backend/drm/presentation_tracker.h"
#include "backend/drm/scanout_controller.h"
#include "fake_drm_device.h"
#include "render/scanout.h"
#include "scene/overlay_candidate.h"
#include "scene/surface_node.h"

namespace Lingmo {

namespace {

constexpr base::Size kOutputSize {1920, 1080};
constexpr uint32_t kCrtcId = 7;
constexpr uint32_t kConnectorId = 3;
constexpr uint32_t kPrimaryPlaneId = 10;
constexpr uint32_t kOverlayPlaneId = 20;
constexpr uint32_t kSecondOverlayPlaneId = 30;
constexpr uint64_t kTiled = 0x0100000000000001ull;

// Property ids the fake hands back; only their identity matters.
enum PropertyId : uint32_t {
    kFbId = 1,
    kCrtcIdProp,
    kSrcX,
    kSrcY,
    kSrcW,
    kSrcH,
    kCrtcX,
    kCrtcY,
    kCrtcW,
    kCrtcH,
    kInFenceFd,
    kType,
    kModeId,
    kActive,
    kOutFencePtr,
    kConnectorCrtcId,
};

backend::DrmPropertyMap planeProperties()
{
    backend::DrmPropertyMap properties;
    properties.insert("FB_ID", kFbId, 0);
    properties.insert("CRTC_ID", kCrtcIdProp, 0);
    properties.insert("SRC_X", kSrcX, 0);
    properties.insert("SRC_Y", kSrcY, 0);
    properties.insert("SRC_W", kSrcW, 0);
    properties.insert("SRC_H", kSrcH, 0);
    properties.insert("CRTC_X", kCrtcX, 0);
    properties.insert("CRTC_Y", kCrtcY, 0);
    properties.insert("CRTC_W", kCrtcW, 0);
    properties.insert("CRTC_H", kCrtcH, 0);
    properties.insert("IN_FENCE_FD", kInFenceFd, 0);
    return properties;
}

backend::DrmPlane makePlane(uint32_t id, uint32_t type,
                            std::map<uint32_t, std::vector<uint64_t>> formats)
{
    backend::DrmPlane plane;
    plane.id = id;
    plane.type = type;
    plane.possibleCrtcs = 0b1;
    plane.properties = planeProperties();
    plane.properties.insert("zpos", 100, id);
    plane.formats = std::move(formats);
    return plane;
}

backend::ScanoutController::Config makeConfig(size_t overlayCount = 2)
{
    const std::map<uint32_t, std::vector<uint64_t>> formats = {
            {DRM_FORMAT_XRGB8888, {DRM_FORMAT_MOD_LINEAR}},
            {DRM_FORMAT_ARGB8888, {DRM_FORMAT_MOD_LINEAR}},
    };

    backend::ScanoutController::Config config;
    config.crtcId = kCrtcId;
    config.crtcIndex = 0;
    config.connectorId = kConnectorId;
    config.modeSize = kOutputSize;
    config.crtcProperties.insert("MODE_ID", kModeId, 0);
    config.crtcProperties.insert("ACTIVE", kActive, 0);
    config.crtcProperties.insert("OUT_FENCE_PTR", kOutFencePtr, 0);
    config.connectorProperties.insert("CRTC_ID", kConnectorCrtcId, 0);

    config.planes.push_back(makePlane(kPrimaryPlaneId, DRM_PLANE_TYPE_PRIMARY, formats));
    if (overlayCount > 0)
        config.planes.push_back(makePlane(kOverlayPlaneId, DRM_PLANE_TYPE_OVERLAY, formats));
    if (overlayCount > 1)
        config.planes.push_back(makePlane(kSecondOverlayPlaneId, DRM_PLANE_TYPE_OVERLAY, formats));
    return config;
}

// A client buffer. The plane fds are real pipe ends so the import path can be
// exercised end to end; the fake device only ever compares them by value.
class TestBuffer final : public render::ScanoutBuffer
{
public:
    explicit TestBuffer(const base::Size &size, uint32_t format = DRM_FORMAT_XRGB8888,
                        uint64_t modifier = DRM_FORMAT_MOD_LINEAR)
    {
        m_attributes.size = size;
        m_attributes.format = format;
        m_attributes.modifier = modifier;
        int fds[2] = {-1, -1};
        EXPECT_EQ(::pipe2(fds, O_CLOEXEC), 0);
        ::close(fds[1]);
        m_attributes.planes.push_back(
                render::DmaBufPlane {ScopedFd(fds[0]), 0, static_cast<uint32_t>(size.width() * 4)});
    }

    const render::DmaBufAttributes &scanoutAttributes() const override { return m_attributes; }

    void onScanoutAcquired() override { ++scanoutCount; }

    void onScanoutReleased(render::SyncFence) override { --scanoutCount; }

    int scanoutCount = 0;

private:
    render::DmaBufAttributes m_attributes;
};

scene::OverlayCandidate makeCandidate(const std::shared_ptr<TestBuffer> &buffer,
                                      const base::Rect &displayRect, int zOrder, bool opaque = true)
{
    scene::OverlayCandidate candidate;
    candidate.buffer = buffer;
    candidate.displayRect = displayRect;
    candidate.sourceRect = base::Rect(buffer->scanoutAttributes().size);
    candidate.opaque = opaque;
    candidate.zOrder = zOrder;
    return candidate;
}

// Stands in for the framebuffer composition would produce.
backend::PlaneContent compositedContent(uint32_t framebufferId = 55)
{
    backend::PlaneContent content;
    content.planeId = kPrimaryPlaneId;
    content.framebufferId = framebufferId;
    content.format = DRM_FORMAT_XRGB8888;
    content.modifier = DRM_FORMAT_MOD_LINEAR;
    content.displayRect = {0, 0, kOutputSize.width(), kOutputSize.height()};
    content.sourceRect = {0, 0, kOutputSize.width(), kOutputSize.height()};
    return content;
}

// Reads back a property value the controller programmed for a plane.
uint64_t propertyValue(const backend::AtomicRequest &request, uint32_t objectId,
                       uint32_t propertyId, bool *found = nullptr)
{
    for (const backend::AtomicProperty &property : request.properties) {
        if (property.objectId == objectId && property.propertyId == propertyId) {
            if (found)
                *found = true;
            return property.value;
        }
    }
    if (found)
        *found = false;
    return 0;
}

class ScanoutTest : public ::testing::Test
{
protected:
    void SetUp() override { reset(); }

    void reset(size_t overlayCount = 2)
    {
        m_device = std::make_unique<testing::FakeDrmDevice>();
        m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                    makeConfig(overlayCount));
    }

    std::unique_ptr<testing::FakeDrmDevice> m_device;
    std::unique_ptr<backend::ScanoutController> m_controller;
};

} // namespace

TEST_F(ScanoutTest, ThePlaneCountsComeFromTheHardware)
{
    EXPECT_TRUE(m_controller->hasPrimaryPlane());
    EXPECT_EQ(m_controller->maxOverlays(), 2u);

    reset(/*overlayCount=*/0);
    EXPECT_TRUE(m_controller->hasPrimaryPlane());
    EXPECT_EQ(m_controller->maxOverlays(), 0u);
}

TEST_F(ScanoutTest, AFullscreenClientBufferTakesThePrimaryPlane)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_TRUE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[0].planeId, kPrimaryPlaneId);
    EXPECT_EQ(planes[0].buffer, buffer);
    // Validation is exactly one TEST_ONLY commit and nothing else.
    EXPECT_EQ(m_device->testCommitCount(), 1u);
    EXPECT_EQ(m_device->realCommitCount(), 0u);
}

TEST_F(ScanoutTest, TheImportPreservesTheClientsFormatModifierAndLayout)
{
    auto buffer = std::make_shared<TestBuffer>(base::Size(640, 480), DRM_FORMAT_ARGB8888, kTiled);
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(100, 50, 640, 480), 1));

    m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(m_device->addFramebufferCalls.size(), 1u);
    const backend::FramebufferParams &params = m_device->addFramebufferCalls[0];
    EXPECT_EQ(params.width, 640u);
    EXPECT_EQ(params.height, 480u);
    EXPECT_EQ(params.format, DRM_FORMAT_ARGB8888);
    EXPECT_EQ(params.modifier, kTiled);
    EXPECT_EQ(params.planeCount, 1u);
    EXPECT_EQ(params.strides[0], 640u * 4);
    EXPECT_EQ(params.offsets[0], 0u);
}

TEST_F(ScanoutTest, AnUnsupportedFormatIsNeverEvenTested)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize, DRM_FORMAT_NV12);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_FALSE(planes[0].isDirectScanout());
    // No plane accepts the format, so there is nothing worth asking the kernel.
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}

TEST_F(ScanoutTest, AnUnsupportedModifierIsNeverEvenTested)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize, DRM_FORMAT_XRGB8888, kTiled);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_FALSE(planes[0].isDirectScanout());
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}

TEST_F(ScanoutTest, ABufferThatCannotBeImportedFallsBackToComposition)
{
    m_device->primeFails = true;
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_FALSE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[0].framebufferId, 55u);
}

TEST_F(ScanoutTest, AFailedImportIsNotRetriedEveryFrame)
{
    m_device->primeFails = true;
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan first;
    first.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    m_controller->validate(first, compositedContent(), 0);
    const size_t attempts = m_device->addFramebufferCalls.size();

    m_device->primeFails = false;
    scene::OverlayPlan second;
    second.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    m_controller->validate(second, compositedContent(), 0);

    // The failure is a property of the buffer; retrying it costs an ioctl a
    // frame for as long as the client keeps the buffer alive.
    EXPECT_EQ(m_device->addFramebufferCalls.size(), attempts);
}

TEST_F(ScanoutTest, ABufferIsImportedOnceAndTheFramebufferReused)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    for (int frame = 0; frame < 3; ++frame) {
        scene::OverlayPlan plan;
        plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
        ASSERT_TRUE(m_controller->validate(plan, compositedContent(), 0)[0].isDirectScanout());
    }

    EXPECT_EQ(m_device->addFramebufferCalls.size(), 1u);
    EXPECT_EQ(m_device->liveFramebuffers.size(), 1u);
}

TEST_F(ScanoutTest, TheFramebufferAndItsHandleDieWithTheClientBuffer)
{
    {
        auto buffer = std::make_shared<TestBuffer>(kOutputSize);
        scene::OverlayPlan plan;
        plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
        ASSERT_TRUE(m_controller->validate(plan, compositedContent(), 0)[0].isDirectScanout());
        EXPECT_EQ(m_device->liveFramebuffers.size(), 1u);
        EXPECT_EQ(m_device->openHandles.size(), 1u);
    }

    // The client went away mid-scanout: nothing may leak, and nothing may be
    // left registered with the kernel.
    EXPECT_TRUE(m_device->liveFramebuffers.empty());
    EXPECT_TRUE(m_device->openHandles.empty());
    EXPECT_EQ(m_device->closedHandles.size(), 1u);
}

TEST_F(ScanoutTest, AFailedAtomicTestFallsBackToComposition)
{
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &) {
        return false;
    };
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_FALSE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[0].framebufferId, 55u);
    EXPECT_EQ(m_controller->testFailures(), 1u);
}

TEST_F(ScanoutTest, TheTopmostOverlayIsDroppedUntilTheConfigurationPasses)
{
    // Upstream's TestPageFlip loop: the hardware runs out of resources for the
    // last plane, so it is given up and the rest is retested.
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &commit) {
        size_t planeCount = 0;
        for (const backend::AtomicProperty &property : commit.request.properties) {
            if (property.propertyId == kFbId && property.value != 0)
                ++planeCount;
        }
        return planeCount <= 2;
    };

    auto first = std::make_shared<TestBuffer>(base::Size(320, 240));
    auto second = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(first, base::Rect(0, 0, 320, 240), 1));
    plan.overlays.push_back(makeCandidate(second, base::Rect(400, 0, 320, 240), 2));

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 2u);
    EXPECT_FALSE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[1].buffer, first);
    EXPECT_EQ(m_device->testCommitCount(), 2u);
}

TEST_F(ScanoutTest, DirectScanoutSurvivesAnOverlayThatCouldNotBeValidated)
{
    // The hardware cannot drive both planes, but the fullscreen buffer on its
    // own is still worth having: dropping the overlay must not drop it too.
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &commit) {
        size_t planeCount = 0;
        for (const backend::AtomicProperty &property : commit.request.properties) {
            if (property.propertyId == kFbId && property.value != 0)
                ++planeCount;
        }
        return planeCount <= 1;
    };

    auto fullscreen = std::make_shared<TestBuffer>(kOutputSize);
    auto small = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(fullscreen, base::Rect(base::Point(0, 0), kOutputSize), 0);
    plan.overlays.push_back(makeCandidate(small, base::Rect(0, 0, 320, 240), 1));

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_TRUE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[0].buffer, fullscreen);
    EXPECT_EQ(m_device->testCommitCount(), 2u);
}

TEST_F(ScanoutTest, WithoutOverlayPlanesNothingIsPromotedAboveThePrimary)
{
    reset(/*overlayCount=*/0);
    auto buffer = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(0, 0, 320, 240), 1));

    const auto planes = m_controller->validate(plan, compositedContent(), 0);

    ASSERT_EQ(planes.size(), 1u);
    EXPECT_FALSE(planes[0].isDirectScanout());
}

TEST_F(ScanoutTest, WithoutAComposedFrameOverlaysAreNotAttempted)
{
    // The very first frame has nothing on the primary plane to test against.
    auto buffer = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(0, 0, 320, 240), 1));

    const auto planes = m_controller->validate(plan, backend::PlaneContent(), 0);

    EXPECT_TRUE(planes.empty());
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}

TEST_F(ScanoutTest, TheCommitProgramsTheGeometryTheCandidateAskedFor)
{
    auto buffer = std::make_shared<TestBuffer>(base::Size(640, 480));
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(100, 50, 640, 480), 1));
    const auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_EQ(planes.size(), 2u);

    ASSERT_TRUE(m_controller->commit(planes, /*modeset=*/false, nullptr, 0, nullptr));

    const backend::AtomicRequest &request = m_device->commits.back().request;
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcX), 100u);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcY), 50u);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcW), 640u);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcH), 480u);
    // SRC_* are 16.16 fixed point.
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kSrcW), 640ull << 16);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kSrcH), 480ull << 16);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcIdProp), kCrtcId);
}

TEST_F(ScanoutTest, AnAcquireFenceIsHandedToKmsRatherThanWaitedOn)
{
    int fds[2] = {-1, -1};
    ASSERT_EQ(::pipe2(fds, O_CLOEXEC), 0);
    ::close(fds[1]);

    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    plan.primary.acquireFence = render::SyncFence(ScopedFd(fds[0]));

    const auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_TRUE(planes[0].isDirectScanout());
    ASSERT_TRUE(m_controller->commit(planes, false, nullptr, 0, nullptr));

    bool found = false;
    const uint64_t value =
            propertyValue(m_device->commits.back().request, kPrimaryPlaneId, kInFenceFd, &found);
    EXPECT_TRUE(found);
    // The plane keeps its own duplicate so the frame can be committed again
    // after the plan is gone; it must still refer to the same pipe.
    const int handed = static_cast<int>(value);
    EXPECT_NE(handed, fds[0]);
    struct stat original = {};
    struct stat duplicate = {};
    ASSERT_EQ(::fstat(fds[0], &original), 0);
    ASSERT_EQ(::fstat(handed, &duplicate), 0);
    EXPECT_EQ(original.st_dev, duplicate.st_dev);
    EXPECT_EQ(original.st_ino, duplicate.st_ino);
}

TEST_F(ScanoutTest, WithoutAnAcquireFenceNoInFencePropertyIsSet)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    const auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_TRUE(m_controller->commit(planes, false, nullptr, 0, nullptr));

    bool found = false;
    propertyValue(m_device->commits.back().request, kPrimaryPlaneId, kInFenceFd, &found);
    EXPECT_FALSE(found);
}

TEST_F(ScanoutTest, APlaneThatFallsOutOfUseIsSwitchedOff)
{
    auto buffer = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(0, 0, 320, 240), 1));
    auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_EQ(planes.size(), 2u);
    ASSERT_TRUE(m_controller->commit(planes, false, nullptr, 0, nullptr));

    // The next frame composites everything again. Leaving the overlay alone
    // would keep a buffer nothing owns any more on screen.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));

    const backend::AtomicRequest &request = m_device->commits.back().request;
    bool found = false;
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kFbId, &found), 0u);
    EXPECT_TRUE(found);
    EXPECT_EQ(propertyValue(request, kOverlayPlaneId, kCrtcIdProp), 0u);
}

TEST_F(ScanoutTest, ScanoutIsGivenUpAndTakenBackWithoutAStaleBuffer)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan scanoutPlan;
    scanoutPlan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    auto planes = m_controller->validate(scanoutPlan, compositedContent(), 0);
    ASSERT_TRUE(planes[0].isDirectScanout());
    ASSERT_TRUE(m_controller->commit(planes, false, nullptr, 0, nullptr));
    const uint32_t clientFramebuffer = planes[0].framebufferId;

    // Back to composition: the primary plane must be handed the composited
    // framebuffer in the same commit, never left showing the client's.
    ASSERT_TRUE(m_controller->commit({compositedContent(77)}, false, nullptr, 0, nullptr));
    EXPECT_EQ(propertyValue(m_device->commits.back().request, kPrimaryPlaneId, kFbId), 77u);

    // And back to scanout, on the framebuffer that was cached all along.
    scene::OverlayPlan again;
    again.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    planes = m_controller->validate(again, compositedContent(77), 0);
    ASSERT_TRUE(planes[0].isDirectScanout());
    EXPECT_EQ(planes[0].framebufferId, clientFramebuffer);
    EXPECT_EQ(m_device->addFramebufferCalls.size(), 1u);
}

TEST_F(ScanoutTest, AModesetCommitsBlockingAndWithoutAnEvent)
{
    ASSERT_TRUE(
            m_controller->commit({compositedContent()}, /*modeset=*/true, nullptr, 42, nullptr));

    const testing::FakeDrmDevice::Commit &commit = m_device->commits.back();
    EXPECT_TRUE(commit.flags & DRM_MODE_ATOMIC_ALLOW_MODESET);
    EXPECT_FALSE(commit.flags & DRM_MODE_PAGE_FLIP_EVENT);
    // Several drivers reject ALLOW_MODESET together with a non-blocking flip.
    EXPECT_FALSE(commit.flags & DRM_MODE_ATOMIC_NONBLOCK);
    EXPECT_FALSE(commit.hadUserData);
    EXPECT_EQ(propertyValue(commit.request, kCrtcId, kModeId), 42u);
    EXPECT_EQ(propertyValue(commit.request, kCrtcId, kActive), 1u);
}

TEST_F(ScanoutTest, AModesetClearsWhatAPreviousMasterLeftBehind)
{
    constexpr uint32_t kCtm = 200, kGammaLut = 201, kDegammaLut = 202, kRotation = 210;
    constexpr uint32_t kCursorPlaneId = 40;
    auto config = makeConfig();
    config.crtcProperties.insert("CTM", kCtm, 55);
    config.crtcProperties.insert("GAMMA_LUT", kGammaLut, 56);
    config.crtcProperties.insert("DEGAMMA_LUT", kDegammaLut, 57);
    config.planes.front().properties.insert("rotation", kRotation, DRM_MODE_REFLECT_X);
    config.planes.push_back(makePlane(kCursorPlaneId, DRM_PLANE_TYPE_CURSOR, {}));
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(), config);

    ASSERT_TRUE(
            m_controller->commit({compositedContent()}, /*modeset=*/true, nullptr, 42, nullptr));
    const backend::AtomicRequest &modeset = m_device->commits.back().request;
    for (uint32_t property : {kCtm, kGammaLut, kDegammaLut}) {
        bool found = false;
        EXPECT_EQ(propertyValue(modeset, kCrtcId, property, &found), 0u);
        EXPECT_TRUE(found) << property;
    }
    EXPECT_EQ(propertyValue(modeset, kPrimaryPlaneId, kRotation), uint64_t(DRM_MODE_ROTATE_0));
    for (uint32_t plane : {kOverlayPlaneId, kSecondOverlayPlaneId, kCursorPlaneId}) {
        bool found = false;
        EXPECT_EQ(propertyValue(modeset, plane, kFbId, &found), 0u);
        EXPECT_TRUE(found) << plane;
        EXPECT_EQ(propertyValue(modeset, plane, kCrtcIdProp), 0u);
    }

    // A page flip leaves the colour pipeline and unused planes alone.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 42, nullptr));
    const backend::AtomicRequest &flip = m_device->commits.back().request;
    bool found = true;
    propertyValue(flip, kCrtcId, kCtm, &found);
    EXPECT_FALSE(found);
    propertyValue(flip, kOverlayPlaneId, kFbId, &found);
    EXPECT_FALSE(found);

    // Our own hardware cursor keeps its plane through a modeset.
    m_controller->setHardwareCursor(true);
    ASSERT_TRUE(
            m_controller->commit({compositedContent()}, /*modeset=*/true, nullptr, 42, nullptr));
    propertyValue(m_device->commits.back().request, kCursorPlaneId, kFbId, &found);
    EXPECT_FALSE(found);
}

namespace {

constexpr uint32_t kCtm = 300, kGammaLut = 301, kGammaLutSize = 302, kDegammaLut = 303;

// A CRTC with a CTM, a gamma LUT of `lutSize` entries, both or neither.
backend::ScanoutController::Config colorConfig(bool ctm, uint64_t lutSize)
{
    auto config = makeConfig();
    if (ctm)
        config.crtcProperties.insert("CTM", kCtm, 0);
    if (lutSize) {
        config.crtcProperties.insert("GAMMA_LUT", kGammaLut, 0);
        config.crtcProperties.insert("GAMMA_LUT_SIZE", kGammaLutSize, lutSize);
    }
    config.crtcProperties.insert("DEGAMMA_LUT", kDegammaLut, 0);
    return config;
}

constexpr render::ColorTransform kWarm {1.0f, 0.75f, 0.5f};

} // namespace

TEST_F(ScanoutTest, ATintRidesTheNextPageFlipAsACtm)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 4096));
    ASSERT_TRUE(m_controller->supportsColorTransform());
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));

    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    const backend::AtomicRequest &flip = m_device->commits.back().request;
    EXPECT_TRUE(m_device->commits.back().flags & DRM_MODE_ATOMIC_NONBLOCK);
    bool found = false;
    const uint64_t blob = propertyValue(flip, kCrtcId, kCtm, &found);
    ASSERT_TRUE(found);
    // The gamma LUT is unset beside the CTM, not left as a previous master set it.
    EXPECT_EQ(propertyValue(flip, kCrtcId, kGammaLut, &found), 0u);
    EXPECT_TRUE(found);

    // S31.32 on the diagonal, zero elsewhere.
    const auto ctm = m_device->blobAs<uint64_t>(uint32_t(blob));
    ASSERT_EQ(ctm.size(), 9u);
    EXPECT_EQ(ctm[0], 1ull << 32);
    EXPECT_EQ(ctm[4], 3ull << 30);
    EXPECT_EQ(ctm[8], 1ull << 31);
    for (const size_t i : {1, 2, 3, 5, 6, 7})
        EXPECT_EQ(ctm[i], 0u) << i;

    // Once the CRTC has it, flips leave the colour pipeline alone.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    propertyValue(m_device->commits.back().request, kCrtcId, kCtm, &found);
    EXPECT_FALSE(found);
    EXPECT_FALSE(m_controller->setColorTransform(kWarm));
}

TEST_F(ScanoutTest, AModesetKeepsTheTint)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 4096));
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    ASSERT_TRUE(m_controller->commit({compositedContent()}, true, nullptr, 42, nullptr));

    const backend::AtomicRequest &modeset = m_device->commits.back().request;
    const uint64_t blob = propertyValue(modeset, kCrtcId, kCtm);
    EXPECT_NE(blob, 0u);
    bool found = false;
    EXPECT_EQ(propertyValue(modeset, kCrtcId, kDegammaLut, &found), 0u);
    EXPECT_TRUE(found);

    // A display waking up modesets again, and comes back tinted.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, true, nullptr, 42, nullptr));
    EXPECT_EQ(propertyValue(m_device->commits.back().request, kCrtcId, kCtm), blob);
}

TEST_F(ScanoutTest, WithoutACtmTheTintGoesIntoTheGammaLut)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(false, 5));
    ASSERT_TRUE(m_controller->supportsColorTransform());
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));

    const uint64_t blob = propertyValue(m_device->commits.back().request, kCrtcId, kGammaLut);
    const auto lut = m_device->blobAs<drm_color_lut>(uint32_t(blob));
    ASSERT_EQ(lut.size(), 5u);
    for (size_t i = 0; i < lut.size(); ++i) {
        const double x = double(i) / 4;
        EXPECT_EQ(lut[i].red, uint16_t(std::lround(x * 65535))) << i;
        EXPECT_EQ(lut[i].green, uint16_t(std::lround(x * 0.75 * 65535))) << i;
        EXPECT_EQ(lut[i].blue, uint16_t(std::lround(x * 0.5 * 65535))) << i;
    }
}

TEST_F(ScanoutTest, WithoutColourPropertiesNothingIsTinted)
{
    EXPECT_FALSE(m_controller->supportsColorTransform());
    EXPECT_FALSE(m_controller->setColorTransform(kWarm));

    // A gamma LUT of one entry cannot hold a line.
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(false, 1));
    EXPECT_FALSE(m_controller->supportsColorTransform());
}

TEST_F(ScanoutTest, TheIdentityUnsetsTheCtmAndFreesTheOldBlob)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 0));
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    const uint32_t blob = uint32_t(propertyValue(m_device->commits.back().request, kCrtcId, kCtm));

    ASSERT_TRUE(m_controller->setColorTransform({}));
    EXPECT_NE(std::find(m_device->destroyedBlobs.begin(), m_device->destroyedBlobs.end(), blob),
              m_device->destroyedBlobs.end());
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    bool found = false;
    EXPECT_EQ(propertyValue(m_device->commits.back().request, kCrtcId, kCtm, &found), 0u);
    EXPECT_TRUE(found);
}

TEST_F(ScanoutTest, APlaneTestCarriesThePendingTint)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 0));
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);

    ASSERT_EQ(m_controller->validate(plan, compositedContent(), 0).size(), 1u);
    const testing::FakeDrmDevice::Commit &test = m_device->commits.back();
    EXPECT_TRUE(test.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    EXPECT_NE(propertyValue(test.request, kCrtcId, kCtm), 0u);
}

TEST_F(ScanoutTest, AFailedFlipKeepsTheTintForTheNextOne)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 0));
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &) {
        return false;
    };
    EXPECT_FALSE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));

    m_device->commitFilter = nullptr;
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    EXPECT_NE(propertyValue(m_device->commits.back().request, kCrtcId, kCtm), 0u);
    EXPECT_TRUE(m_controller->supportsColorTransform());
}

TEST_F(ScanoutTest, ATintTheDriverRefusesIsDroppedRatherThanTheFrame)
{
    m_controller = std::make_unique<backend::ScanoutController>(m_device.get(),
                                                                colorConfig(true, 0));
    ASSERT_TRUE(m_controller->setColorTransform(kWarm));
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &commit) {
        return std::none_of(commit.request.properties.begin(), commit.request.properties.end(),
                            [](const backend::AtomicProperty &property) {
                                return property.objectId == kCrtcId
                                        && property.propertyId == kCtm && property.value != 0;
                            });
    };

    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    bool found = false;
    EXPECT_EQ(propertyValue(m_device->commits.back().request, kCrtcId, kCtm, &found), 0u);
    EXPECT_TRUE(found);
    EXPECT_FALSE(m_controller->supportsColorTransform());
    EXPECT_FALSE(m_controller->setColorTransform({1.0f, 0.5f, 0.0f}));
}

TEST_F(ScanoutTest, APageFlipCarriesItsCompletionToken)
{
    int token = 0;
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, &token, 0, nullptr));

    const testing::FakeDrmDevice::Commit &commit = m_device->commits.back();
    EXPECT_TRUE(commit.flags & DRM_MODE_PAGE_FLIP_EVENT);
    EXPECT_TRUE(commit.flags & DRM_MODE_ATOMIC_NONBLOCK);
    EXPECT_TRUE(commit.hadUserData);
}

TEST_F(ScanoutTest, ARequestedReleaseFenceAsksTheCrtcForOne)
{
    render::SyncFence releaseFence;
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, &releaseFence));

    bool found = false;
    propertyValue(m_device->commits.back().request, kCrtcId, kOutFencePtr, &found);
    EXPECT_TRUE(found);
    // The fake never writes an fd back, so no fence comes out - which is the
    // same fallback a driver without OUT_FENCE_PTR produces.
    EXPECT_FALSE(releaseFence.isValid());
}

TEST_F(ScanoutTest, AFailedCommitReportsFailureAndKeepsNoState)
{
    m_device->commitFilter = [](const testing::FakeDrmDevice::Commit &commit) {
        return (commit.flags & DRM_MODE_ATOMIC_TEST_ONLY) != 0;
    };

    EXPECT_FALSE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));

    // Nothing was programmed, so the next commit has no plane to switch off.
    m_device->commitFilter = nullptr;
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));
    bool found = false;
    propertyValue(m_device->commits.back().request, kOverlayPlaneId, kFbId, &found);
    EXPECT_FALSE(found);
}

TEST_F(ScanoutTest, DisablingTurnsTheCrtcAndItsPlanesOff)
{
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 0, nullptr));

    ASSERT_TRUE(m_controller->disable());

    const backend::AtomicRequest &request = m_device->commits.back().request;
    EXPECT_EQ(propertyValue(request, kCrtcId, kActive), 0u);
    EXPECT_EQ(propertyValue(request, kCrtcId, kModeId), 0u);
    EXPECT_EQ(propertyValue(request, kConnectorId, kConnectorCrtcId), 0u);
    EXPECT_EQ(propertyValue(request, kPrimaryPlaneId, kFbId), 0u);
}

TEST_F(ScanoutTest, TwoFramebuffersOverTheSameBufferShareOneHandle)
{
    // drmPrimeFDToHandle hands out one handle per underlying buffer; closing it
    // while a second framebuffer still references it would be a driver-visible
    // use-after-free.
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    const render::DmaBufAttributes &attributes = buffer->scanoutAttributes();

    std::unique_ptr<backend::DrmFramebuffer> first =
            backend::DrmFramebuffer::fromDmaBuf(m_device.get(), attributes);
    std::unique_ptr<backend::DrmFramebuffer> second =
            backend::DrmFramebuffer::fromDmaBuf(m_device.get(), attributes);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    first.reset();
    EXPECT_TRUE(m_device->closedHandles.empty());
    second.reset();
    EXPECT_EQ(m_device->closedHandles.size(), 1u);
}

TEST_F(ScanoutTest, ABufferOnAPlaneIsHeldUntilTheFrameRetires)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_TRUE(planes[0].isDirectScanout());

    // The output takes a scanout reference at commit and gives it back when the
    // flip that replaced the frame completes.
    for (backend::PlaneContent &content : planes)
        content.buffer->onScanoutAcquired();
    EXPECT_EQ(buffer->scanoutCount, 1);

    for (backend::PlaneContent &content : planes)
        content.buffer->onScanoutReleased(render::SyncFence());
    EXPECT_EQ(buffer->scanoutCount, 0);
}

namespace {

// The pending/on-screen frame pair DrmOutput keeps, in the same sequence it
// drives them, so the retention rules can be exercised without GBM or EGL.
struct FrameQueue {
    std::vector<backend::PlaneContent> pending;
    std::vector<backend::PlaneContent> onScreen;

    void submit(std::vector<backend::PlaneContent> planes)
    {
        // A frame replaced before it ever flipped never reached the display.
        release(pending);
        for (backend::PlaneContent &content : planes) {
            if (content.buffer)
                content.buffer->onScanoutAcquired();
        }
        pending = std::move(planes);
    }

    void flip()
    {
        release(onScreen);
        onScreen = std::move(pending);
        pending.clear();
    }

    void releaseAll()
    {
        release(pending);
        release(onScreen);
    }

private:
    static void release(std::vector<backend::PlaneContent> &planes)
    {
        for (backend::PlaneContent &content : planes) {
            if (content.buffer)
                content.buffer->onScanoutReleased(render::SyncFence());
        }
        planes.clear();
    }
};

TEST_F(ScanoutTest, PresentationWaitsForAcceptedDirectScanoutCompletion)
{
    backend::PresentationTracker tracker;
    std::vector<render::PresentationResult> results;
    auto feedback = std::make_shared<render::PresentationContent>();
    feedback->add([&](const auto &result) {
        results.push_back(result);
    });
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::SurfaceNode node;
    node.setPresentationContent(feedback);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(), kOutputSize), 0);
    plan.primary.node = &node;
    auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_EQ(planes.size(), 1u);
    ASSERT_TRUE(planes[0].isDirectScanout());
    EXPECT_TRUE(results.empty());
    EXPECT_FALSE(tracker.complete(1, 123, 456789, 42, true, 75, nullptr));
    int eventToken = 1;
    ASSERT_TRUE(m_controller->commit(planes, false, &eventToken, 0, nullptr));
    render::PresentationSubmission submission;
    for (const auto &plane : planes)
        if (plane.node)
            submission.add(plane.node->presentationContent(), true);
    tracker.submitted(1, std::move(submission));
    buffer->onScanoutAcquired();
    EXPECT_TRUE(results.empty());
    ASSERT_TRUE(tracker.complete(1, 123, 456789, 42, true, 75, nullptr));
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].flags, 15u);
    EXPECT_EQ(results[0].nanoseconds, 456789000u);
    EXPECT_EQ(buffer->scanoutCount, 1);
    buffer->onScanoutReleased({});
    EXPECT_EQ(buffer->scanoutCount, 0);
}

TEST_F(ScanoutTest, RejectedPromotionRetriesCompositionWithoutZeroCopy)
{
    backend::PresentationTracker tracker;
    std::vector<render::PresentationResult> results;
    auto feedback = std::make_shared<render::PresentationContent>();
    feedback->add([&](const auto &result) {
        results.push_back(result);
    });
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(), kOutputSize), 0);
    auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_TRUE(planes[0].isDirectScanout());
    m_device->commitFilter = [](const auto &) {
        return false;
    };
    int token = 1;
    ASSERT_FALSE(m_controller->commit(planes, false, &token, 0, nullptr));
    EXPECT_TRUE(results.empty());
    m_device->commitFilter = {};
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, &token, 0, nullptr));
    render::PresentationSubmission submission;
    submission.add(feedback, false);
    tracker.submitted(1, std::move(submission));
    EXPECT_TRUE(results.empty());
    tracker.complete(1, 123, 456789, 42, true, 59.94, nullptr);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].flags, 7u);
}

TEST_F(ScanoutTest, MixedOverlayCompletionAndInitialModesetEventToken)
{
    backend::PresentationTracker tracker;
    std::vector<render::PresentationResult> results;
    auto composed = std::make_shared<render::PresentationContent>();
    auto promoted = std::make_shared<render::PresentationContent>();
    composed->add([&](const auto &result) {
        results.push_back(result);
    });
    promoted->add([&](const auto &result) {
        results.push_back(result);
    });
    auto buffer = std::make_shared<TestBuffer>(base::Size(320, 240));
    scene::SurfaceNode node;
    node.setPresentationContent(promoted);
    scene::OverlayPlan plan;
    plan.overlays.push_back(makeCandidate(buffer, base::Rect(0, 0, 320, 240), 1));
    plan.overlays.back().node = &node;
    auto planes = m_controller->validate(plan, compositedContent(), 0);
    ASSERT_EQ(planes.size(), 2u);
    int token = 1;
    ASSERT_TRUE(m_controller->commit(planes, false, &token, 0, nullptr));
    render::PresentationSubmission submission;
    submission.add(composed, false);
    submission.add(planes[1].node->presentationContent(), true);
    tracker.submitted(1, std::move(submission));
    tracker.complete(1, 123, 456789, 42, true, 60, nullptr);
    ASSERT_EQ(results.size(), 2u);
    EXPECT_EQ(results[0].flags, 7u);
    EXPECT_EQ(results[1].flags, 15u);
    ASSERT_TRUE(m_controller->commit({compositedContent()}, true, &token, 0, nullptr));
    EXPECT_TRUE(m_device->commits.back().flags & DRM_MODE_PAGE_FLIP_EVENT);
    EXPECT_TRUE(m_device->commits.back().hadUserData);
}
} // namespace

TEST_F(ScanoutTest, ADelayedPageFlipKeepsBothFramesBuffersHeld)
{
    auto first = std::make_shared<TestBuffer>(kOutputSize);
    auto second = std::make_shared<TestBuffer>(kOutputSize);
    FrameQueue queue;

    scene::OverlayPlan firstPlan;
    firstPlan.primary = makeCandidate(first, base::Rect(base::Point(0, 0), kOutputSize), 0);
    queue.submit(m_controller->validate(firstPlan, compositedContent(), 0));
    queue.flip();
    EXPECT_EQ(first->scanoutCount, 1);

    // The flip for the second frame has not arrived yet, so the display may
    // still be showing either buffer and neither may go back to its client.
    scene::OverlayPlan secondPlan;
    secondPlan.primary = makeCandidate(second, base::Rect(base::Point(0, 0), kOutputSize), 0);
    queue.submit(m_controller->validate(secondPlan, compositedContent(), 0));
    EXPECT_EQ(first->scanoutCount, 1);
    EXPECT_EQ(second->scanoutCount, 1);

    queue.flip();
    EXPECT_EQ(first->scanoutCount, 0);
    EXPECT_EQ(second->scanoutCount, 1);

    queue.releaseAll();
    EXPECT_EQ(second->scanoutCount, 0);
}

TEST_F(ScanoutTest, SuspendAndResumeKeepTheScannedOutBufferHeld)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    FrameQueue queue;
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    queue.submit(m_controller->validate(plan, compositedContent(), 0));
    queue.flip();

    // The session went away. The buffer is still what the CRTC will come back
    // to, so it stays held across the whole suspend.
    ASSERT_TRUE(m_controller->disable());
    EXPECT_EQ(buffer->scanoutCount, 1);

    // Resuming re-commits the retained configuration as a blocking modeset,
    // which is what avoids needing a third buffer to come back on screen.
    ASSERT_TRUE(m_controller->commit(queue.onScreen, /*modeset=*/true, nullptr, 42, nullptr));
    EXPECT_EQ(buffer->scanoutCount, 1);
    EXPECT_EQ(propertyValue(m_device->commits.back().request, kPrimaryPlaneId, kFbId),
              queue.onScreen[0].framebufferId);

    queue.releaseAll();
    EXPECT_EQ(buffer->scanoutCount, 0);
}

TEST_F(ScanoutTest, AClientDestroyedMidScanoutLeavesItsBufferOnTheScreen)
{
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    TestBuffer *raw = buffer.get();
    FrameQueue queue;
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(0, 0), kOutputSize), 0);
    queue.submit(m_controller->validate(plan, compositedContent(), 0));
    queue.flip();

    // Everything the compositor held goes away: the surface, the wl_buffer,
    // the client. The plane still references the storage.
    plan = scene::OverlayPlan();
    buffer.reset();
    EXPECT_EQ(raw->scanoutCount, 1);
    EXPECT_EQ(m_device->liveFramebuffers.size(), 1u);

    // Only retiring the frame lets the buffer and its framebuffer go.
    queue.releaseAll();
    EXPECT_TRUE(m_device->liveFramebuffers.empty());
    EXPECT_TRUE(m_device->openHandles.empty());
}

} // namespace Lingmo

TEST(CrossGpuScanout, CpuTransferCopiesRowsAndRetiresFramebufferBeforeDumbBuffer)
{
    Lingmo::testing::FakeDrmDevice device;
    const uint8_t pixels[] = {1, 2,  3,  4,  5,  6,  7,  8,  99, 99, 99, 99,
                              9, 10, 11, 12, 13, 14, 15, 16, 99, 99, 99, 99};
    std::vector<uint8_t> copied;
    device.beforeUnmap = [&](const void *data) {
        const auto *bytes = static_cast<const uint8_t *>(data);
        copied.assign(bytes, bytes + 16);
    };
    auto framebuffer = Lingmo::backend::DrmFramebuffer::fromPixels(&device, {2, 2}, pixels, 12);
    ASSERT_TRUE(framebuffer);
    EXPECT_EQ(copied,
              std::vector<uint8_t>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}));
    EXPECT_EQ(device.liveDumbBuffers, 1);
    EXPECT_EQ(device.liveMappings, 0);
    EXPECT_EQ(framebuffer->modifier(), uint64_t(DRM_FORMAT_MOD_LINEAR));
    framebuffer.reset();
    EXPECT_EQ(device.liveDumbBuffers, 0);
    EXPECT_TRUE(device.liveFramebuffers.empty());
}

TEST(CrossGpuScanout, EachBufferIsImportedOrCopiedOnceAndRetiredWithIt)
{
    // The buffers come from the rendering GPU; the scanout device is the fake.
    const int fd = ::open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (fd < 0)
        GTEST_SKIP() << "No render node to allocate buffers on";
    gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        ::close(fd);
        GTEST_SKIP() << "No GBM on the render node";
    }
    gbm_bo *bo = gbm_bo_create(gbm, 4, 4, GBM_FORMAT_XRGB8888,
                               GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    Lingmo::testing::FakeDrmDevice device;
    if (bo) {
        auto *imported = Lingmo::backend::DrmFramebuffer::importForBuffer(&device, bo);
        ASSERT_NE(imported, nullptr);
        EXPECT_EQ(Lingmo::backend::DrmFramebuffer::importForBuffer(&device, bo), imported);
        EXPECT_EQ(device.addFramebufferCalls.size(), 1u);
        // A copy replaces the import, and is filled again rather than made again.
        const std::vector<uint8_t> pixels(4 * 4 * 4, 7);
        auto *copy = Lingmo::backend::DrmFramebuffer::copyForBuffer(&device, bo, pixels.data(),
                                                                      16);
        ASSERT_NE(copy, nullptr);
        EXPECT_EQ(Lingmo::backend::DrmFramebuffer::copyForBuffer(&device, bo, pixels.data(), 16),
                  copy);
        EXPECT_EQ(device.addFramebufferCalls.size(), 2u);
        EXPECT_EQ(device.liveFramebuffers.size(), 1u);
        EXPECT_EQ(device.liveDumbBuffers, 1);
        gbm_bo_destroy(bo);
        EXPECT_TRUE(device.liveFramebuffers.empty());
        EXPECT_EQ(device.liveDumbBuffers, 0);
    }
    gbm_device_destroy(gbm);
    ::close(fd);
    ASSERT_NE(bo, nullptr);
}

TEST(SharedFileScanout, AClientBufferTakesTheRenderersHandleAndNeverClosesIt)
{
    // KMS and the renderer share the card's file, so the renderer's import of a
    // client buffer holds the GEM handle KMS gets for it. The render node plays
    // that file; the fake device is its KMS side.
    const int fd = ::open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
    if (fd < 0)
        GTEST_SKIP() << "No render node to allocate buffers on";
    gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        ::close(fd);
        GTEST_SKIP() << "No GBM on the render node";
    }
    gbm_bo *bo = gbm_bo_create(gbm, 64, 64, GBM_FORMAT_XRGB8888,
                               GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING | GBM_BO_USE_LINEAR);
    Lingmo::testing::FakeDrmDevice device;
    device.gbm = gbm;
    if (bo) {
        Lingmo::render::DmaBufAttributes attributes;
        attributes.size = Lingmo::base::Size(gbm_bo_get_width(bo), gbm_bo_get_height(bo));
        attributes.format = gbm_bo_get_format(bo);
        attributes.modifier = gbm_bo_get_modifier(bo);
        for (int plane = 0; plane < gbm_bo_get_plane_count(bo); ++plane)
            attributes.planes.push_back({Lingmo::ScopedFd(gbm_bo_get_fd_for_plane(bo, plane)),
                                         gbm_bo_get_offset(bo, plane),
                                         gbm_bo_get_stride_for_plane(bo, plane)});
        const uint32_t handle = gbm_bo_get_handle(bo).u32;
        const auto handleIsOpen = [&] {
            int exported = -1;
            if (drmPrimeHandleToFD(fd, handle, DRM_CLOEXEC, &exported) != 0)
                return false;
            ::close(exported);
            return true;
        };

        auto framebuffer = Lingmo::backend::DrmFramebuffer::fromDmaBuf(&device, attributes);
        ASSERT_TRUE(framebuffer);
        ASSERT_EQ(device.addFramebufferCalls.size(), 1u);
        EXPECT_EQ(device.addFramebufferCalls.back().handles[0], handle);
        EXPECT_TRUE(device.openHandles.empty());
        framebuffer.reset();
        EXPECT_TRUE(device.liveFramebuffers.empty());
        EXPECT_TRUE(device.closedHandles.empty());
        EXPECT_TRUE(handleIsOpen());

        // A buffer KMS rejects must not take the handle with it either.
        device.addFramebufferFails = true;
        EXPECT_FALSE(Lingmo::backend::DrmFramebuffer::fromDmaBuf(&device, attributes));
        EXPECT_TRUE(device.closedHandles.empty());
        EXPECT_TRUE(handleIsOpen());
        gbm_bo_destroy(bo);
    }
    gbm_device_destroy(gbm);
    ::close(fd);
    ASSERT_NE(bo, nullptr);
}

TEST(CrossGpuScanout, CpuTransferRejectsOversizeAndCleansFailedFramebufferAllocation)
{
    Lingmo::testing::FakeDrmDevice device;
    uint8_t pixels[16] = {};
    EXPECT_FALSE(
            Lingmo::backend::DrmFramebuffer::fromPixels(&device, {5120, 2880}, pixels, 5120 * 4));
    EXPECT_EQ(device.liveDumbBuffers, 0);
    EXPECT_TRUE(device.addFramebufferCalls.empty());
    device.addFramebufferFails = true;
    EXPECT_FALSE(Lingmo::backend::DrmFramebuffer::fromPixels(&device, {2, 2}, pixels, 8));
    EXPECT_EQ(device.liveDumbBuffers, 0);
    EXPECT_EQ(device.liveMappings, 0);
}

namespace Lingmo {
TEST_F(ScanoutTest, ExplicitAcquireWithoutPlaneFencePropertyFallsBack)
{
    auto config = makeConfig();
    config.planes.front().properties.insert("IN_FENCE_FD", 0, 0);
    backend::ScanoutController controller(m_device.get(), std::move(config));
    auto buffer = std::make_shared<TestBuffer>(kOutputSize);
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(buffer, base::Rect(base::Point(), kOutputSize), 0);
    plan.primary.acquireFence =
            render::SyncFence(ScopedFd(dup(buffer->scanoutAttributes().planes[0].fd.get())));
    const auto result = controller.validate(plan, compositedContent(), 42);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_FALSE(result.front().isDirectScanout());
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}

TEST_F(ScanoutTest, PlaneIdOrderCannotOverrideHardwareStacking)
{
    auto config = makeConfig();
    config.planes[1].properties.insert("zpos", 100, 0);
    backend::ScanoutController controller(m_device.get(), std::move(config));
    scene::OverlayPlan plan;
    plan.overlays.push_back(
            makeCandidate(std::make_shared<TestBuffer>(base::Size(64, 64)), {0, 0, 64, 64}, 1));
    const auto result = controller.validate(plan, compositedContent(), 42);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_FALSE(result.front().isDirectScanout());
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}

TEST_F(ScanoutTest, NonIdentityPlaneStateFallsBackEvenIfKernelWouldAccept)
{
    for (const auto &property : {std::string("rotation"), std::string("alpha")}) {
        auto config = makeConfig();
        config.planes.front().properties.insert(property, 100, 0);
        backend::ScanoutController controller(m_device.get(), std::move(config));
        scene::OverlayPlan plan;
        plan.primary = makeCandidate(std::make_shared<TestBuffer>(kOutputSize),
                                     base::Rect(base::Point(), kOutputSize), 0);
        const auto result = controller.validate(plan, compositedContent(), 42);
        ASSERT_EQ(result.size(), 1u);
        EXPECT_FALSE(result.front().isDirectScanout());
    }
}
} // namespace Lingmo

namespace Lingmo {
TEST_F(ScanoutTest, YuvNeedsColorStateEvenWhenAdvertisedByThePlane)
{
    auto config = makeConfig();
    config.planes.front().formats[DRM_FORMAT_NV12] = {DRM_FORMAT_MOD_LINEAR};
    backend::ScanoutController controller(m_device.get(), std::move(config));
    scene::OverlayPlan plan;
    plan.primary = makeCandidate(std::make_shared<TestBuffer>(kOutputSize, DRM_FORMAT_NV12),
                                 base::Rect(base::Point(), kOutputSize), 0);
    const auto result = controller.validate(plan, compositedContent(), 42);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_FALSE(result.front().isDirectScanout());
    EXPECT_EQ(m_device->testCommitCount(), 0u);
}
} // namespace Lingmo

namespace Lingmo {
namespace {

// A secure commit's request must leave nothing of the previous session's frame
// programmed: the whole point is that the shield is not merely painted over an
// application buffer that is still on a plane.
void expectPlaneDisabled(const backend::AtomicRequest &request, uint32_t planeId)
{
    bool foundFb = false;
    bool foundCrtc = false;
    EXPECT_EQ(propertyValue(request, planeId, kFbId, &foundFb), 0u)
            << "plane " << planeId << " still has a framebuffer";
    EXPECT_EQ(propertyValue(request, planeId, kCrtcIdProp, &foundCrtc), 0u)
            << "plane " << planeId << " is still attached to the CRTC";
    EXPECT_TRUE(foundFb) << "plane " << planeId << " was not programmed at all";
    EXPECT_TRUE(foundCrtc) << "plane " << planeId << " was not programmed at all";
}

} // namespace

TEST_F(ScanoutTest, ASecureCommitDisablesTheOverlayPlanesOfThePreviousFrame)
{
    // A frame of the active session: a client buffer on an overlay above the
    // composited desktop.
    scene::OverlayPlan plan;
    plan.overlays.push_back(
            makeCandidate(std::make_shared<TestBuffer>(base::Size(64, 64)), base::Rect(10, 10, 64, 64), 1));
    const auto session = m_controller->validate(plan, compositedContent(), 42);
    ASSERT_EQ(session.size(), 2u);
    ASSERT_TRUE(m_controller->commit(session, false, nullptr, 42, nullptr));

    // Locking: composited primary only, committed securely.
    const size_t before = m_device->commits.size();
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 42, nullptr, true));
    ASSERT_EQ(m_device->commits.size(), before + 1);

    const auto &request = m_device->commits.back().request;
    EXPECT_EQ(propertyValue(request, kPrimaryPlaneId, kFbId), 55u);
    expectPlaneDisabled(request, kOverlayPlaneId);
    expectPlaneDisabled(request, kSecondOverlayPlaneId);
}

TEST_F(ScanoutTest, ASecureCommitDisablesPlanesThisControllerNeverProgrammed)
{
    // Nothing has been committed, so the controller's own record of active
    // planes is empty. A plane left live by a modeset, a previous compositor or
    // a commit that failed still has to be cleared.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 42, nullptr, true));

    const auto &request = m_device->commits.back().request;
    expectPlaneDisabled(request, kOverlayPlaneId);
    expectPlaneDisabled(request, kSecondOverlayPlaneId);
}

TEST_F(ScanoutTest, AnOrdinaryCommitStillOnlyDisablesWhatItProgrammed)
{
    // The secure path is additional, not a replacement: an unlocked session
    // must not pay for programming every plane on every frame.
    ASSERT_TRUE(m_controller->commit({compositedContent()}, false, nullptr, 42, nullptr));

    const auto &request = m_device->commits.back().request;
    bool found = false;
    propertyValue(request, kOverlayPlaneId, kFbId, &found);
    EXPECT_FALSE(found);
}

TEST_F(ScanoutTest, ASecureDirectScanoutConfigurationIsNotSecure)
{
    // Belt and braces for the DrmOutput predicate: a client buffer on the
    // primary plane is an application plane however the commit was flagged.
    backend::PlaneContent direct = compositedContent();
    direct.buffer = std::make_shared<TestBuffer>(kOutputSize);
    EXPECT_FALSE(backend::DrmOutput::isSecureConfiguration({direct}));
    EXPECT_TRUE(backend::DrmOutput::isSecureConfiguration({compositedContent()}));
    EXPECT_FALSE(
            backend::DrmOutput::isSecureConfiguration({compositedContent(), compositedContent()}));
}

} // namespace Lingmo
