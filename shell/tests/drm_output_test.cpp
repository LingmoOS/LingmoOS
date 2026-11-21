// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/drm/drm_mode_fallback_planner.h"
#include "backend/drm/drm_output.h"
#include "base/strings.h"
#include "egl_test_util.h"
#include "fake_drm_device.h"
#include "loop_test_util.h"
#include "presentation_wire_fixture.h"
#include "wayland/protocols/image_capture.h"
#include "wayland/server_util.h"
#include <GLES2/gl2.h>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <drm_fourcc.h>
#include <ext-image-capture-source-v1-client-protocol.h>
#include <ext-image-copy-capture-v1-client-protocol.h>
#include <fcntl.h>
#include <gbm.h>
#include <optional>
#include <set>
#include <string>
#include <sys/eventfd.h>
#include <sys/mman.h>

namespace {
using Output = backend::DrmOutput;

// RGBA bytes of a `size` image in one colour.
std::vector<uint8_t> solidRgba(const base::Size &size, const std::array<uint8_t, 4> &color)
{
    std::vector<uint8_t> pixels;
    for (int64_t i = 0; i < size.area(); ++i)
        pixels.insert(pixels.end(), color.begin(), color.end());
    return pixels;
}

class ScanoutStorage : public render::ScanoutBuffer
{
public:
    explicit ScanoutStorage(base::Size size = {16, 16})
    {
        attributes.size = size;
        attributes.format = DRM_FORMAT_XRGB8888;
        attributes.modifier = DRM_FORMAT_MOD_LINEAR;
        attributes.planes.push_back({ScopedFd(testsupport::makeMemfd(1024)), 0, 64});
    }

    const render::DmaBufAttributes &scanoutAttributes() const override { return attributes; }

    void onScanoutAcquired() override { ++references; }

    void onScanoutReleased(render::SyncFence fence) override
    {
        --references;
        ++releases;
        retiredFence = std::move(fence);
    }

    render::DmaBufAttributes attributes;
    render::SyncFence retiredFence;
    int references = 0, releases = 0;
};

// Reads back a property value a commit programmed for an object.
uint64_t propertyValue(const backend::AtomicRequest &request, uint32_t objectId,
                       uint32_t propertyId, bool *found = nullptr)
{
    if (found)
        *found = false;
    for (const backend::AtomicProperty &property : request.properties) {
        if (property.objectId == objectId && property.propertyId == propertyId) {
            if (found)
                *found = true;
            return property.value;
        }
    }
    return 0;
}

class DrmOutputTest : public PresentationProtocolTest
{
protected:
    Lingmo::testing::FakeDrmDevice device;
    ScopedFd gbmFd;
    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> gbmDevice {nullptr,
                                                                          gbm_device_destroy};

    struct NativeTarget {
        EGLSurface eglSurface;
        std::shared_ptr<gbm_surface> surface;
    };

    // Keep native windows alive until all EGL targets and locked buffers retire.
    std::vector<NativeTarget> nativeTargets;
    base::Size acquiredSize;
    std::unique_ptr<render::EglRenderer> egl;
    std::unique_ptr<Output> drm;
    std::vector<base::Subscription> outputSubscriptions;
    Output::Config initialConfig;
    std::vector<Output::FlipEvent> events;
    std::function<bool(const Lingmo::testing::FakeDrmDevice::Commit &)> accept;
    bool active = true;
    ScopedFd outFence;
    std::chrono::steady_clock::duration clockOffset {};
    bool acquireFails = false;
    // The first pixel every acquired buffer must have; any byte order, so a
    // grey is the colour to expect.
    std::optional<uint8_t> checkPixel;
    // The first pixel of the last acquired buffer, red, green and blue, while asked for.
    bool recordPixel = false;
    std::array<uint8_t, 3> lastPixel {};
    int acquired = 0, released = 0, pacing = 0, prepared = 0;
    double scale = 1;
    std::shared_ptr<ScanoutStorage> storage;
    base::Subscription repaintNeeded;

    void SetUp() override
    {
        PresentationProtocolTest::SetUp();
        ASSERT_FALSE(HasFatalFailure());
        const char *devicePath = getenv("LINGMO_TEST_DRM_DEVICE");
        if (!devicePath || !*devicePath) {
            egl = render::EglRendererTestAccess::create();
        } else {
            gbmFd.reset(open(devicePath, O_RDWR | O_CLOEXEC));
            ASSERT_TRUE(gbmFd.isValid());
            gbmDevice.reset(gbm_create_device(gbmFd.get()));
            ASSERT_TRUE(gbmDevice);
            egl = render::EglRenderer::create(gbmDevice.get(), DRM_FORMAT_XRGB8888);
        }
        ASSERT_TRUE(egl);
        Output::Config config;
        config.name = "injected-kms";
        config.connectorId = 3;
        config.crtcId = 7;
        config.crtcIndex = 0;
        config.planeId = 10;
        config.mode.hdisplay = config.mode.vdisplay = 16;
        config.mode.htotal = config.mode.vtotal = 20;
        config.mode.clock = 30;
        config.crtcProperties.insert("MODE_ID", 20, 0);
        config.crtcProperties.insert("ACTIVE", 21, 0);
        config.crtcProperties.insert("OUT_FENCE_PTR", 23, 0);
        config.crtcProperties.insert("CTM", 24, 0);
        config.connectorProperties.insert("CRTC_ID", 22, 0);
        backend::DrmPlane primary;
        primary.id = 10;
        primary.type = DRM_PLANE_TYPE_PRIMARY;
        primary.possibleCrtcs = 1;
        uint32_t property = 1;
        for (const auto &name : Output::requiredPlaneProperties())
            primary.properties.insert(name, property++, 0);
        primary.formats[DRM_FORMAT_XRGB8888] = {DRM_FORMAT_MOD_LINEAR};
        primary.properties.insert("zpos", 100, 0);
        config.planes.push_back(primary);
        primary.id = 11;
        primary.type = DRM_PLANE_TYPE_OVERLAY;
        primary.properties.insert("zpos", 100, 1);
        config.planes.push_back(primary);
        initialConfig = config;
        device.maxCursor = {};
        device.commitFilter = [&](const auto &commit) {
            if (accept && !accept(commit))
                return false;
            if (outFence.isValid()) {
                for (const auto &property : commit.request.properties)
                    if (property.objectId == 7 && property.propertyId == 23)
                        *reinterpret_cast<int *>(property.value) = dup(outFence.get());
            }
            if (commit.userData) {
                EXPECT_FALSE(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
                events.push_back(*static_cast<Output::FlipEvent *>(commit.userData));
            }
            return true;
        };
        drm = std::make_unique<Output>(
                &device, config,
                [&] {
                    return active;
                },
                true,
                [&] {
                    return std::chrono::steady_clock::now() + clockOffset;
                });
        auto target = createTarget({16, 16});
        ASSERT_TRUE(target);
        ASSERT_TRUE(drm->init(
                egl.get(), std::move(target),
                [&] {
                    return acquireBuffer();
                },
                [&](base::Size size) {
                    return createTarget(size);
                }));
        output.pixelSize = {16, 16};
        scene.setSize({16, 16});
        drm->presentationOutput = output.presentation;
        drm->presentationSubmitted = [&](const auto &submission) {
            compositor.abandonUnsubmittedPresentation(base::Rect(drm->size()), submission);
        };
        repaintNeeded = compositor.repaintNeeded.connect([output = drm->weakPtr()](bool decoration) {
            if (output)
                output->scheduleFrame(decoration ? Output::Reason::Decoration
                                                 : Output::Reason::Surface);
        });
        outputSubscriptions.push_back(drm->frameDiscarded.connect([&](uint64_t frame) {
            compositor.frameDiscarded(frame);
        }));
        drm->setPrepareCallback([&](render::Renderer &renderer, uint64_t frame) {
            ++prepared;
            renderer.setOutputContext(
                    {output.name, base::Rect(drm->size()), drm->size(), 1, 0, frame});
            compositor.updateTextures(renderer);
            compositor.frameSubmitted(frame);
        });
        drm->setRenderCallback([&](render::Renderer &renderer, uint64_t) {
            scene.render(renderer);
        });
        drm->setScanoutPlanner([&](const base::Size &size, size_t count) {
            return scene::proposeOverlays(*scene.root(), size, count, scale);
        });
        outputSubscriptions.push_back(drm->frameDrawn.connect([&](uint64_t frame, uint64_t) {
            compositor.frameDrawn(wayland::nowInMilliseconds(), frame);
        }));
        outputSubscriptions.push_back(drm->framePresented.connect([&](uint32_t, uint64_t) {
            ++pacing;
        }));
    }

    void TearDown() override
    {
        accept = {};
        repaintNeeded.reset();
        drm.reset();
        storage.reset();
        PresentationProtocolTest::TearDown();
        egl.reset();
        nativeTargets.clear();
    }

    std::unique_ptr<render::EglRenderTarget> createTarget(base::Size size)
    {
        if (gbmDevice) {
            auto surface = std::shared_ptr<gbm_surface>(
                    gbm_surface_create(gbmDevice.get(), size.width(), size.height(),
                                       DRM_FORMAT_XRGB8888,
                                       GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT),
                    gbm_surface_destroy);
            if (!surface)
                return nullptr;
            auto target = egl->createTarget(surface.get(), size);
            if (target)
                nativeTargets.push_back({target->eglSurface(), std::move(surface)});
            return target;
        }
        const EGLint attributes[] = {EGL_WIDTH, size.width(), EGL_HEIGHT, size.height(), EGL_NONE};
        auto surface = eglCreatePbufferSurface(egl->eglDisplay(), egl->eglConfig(), attributes);
        if (surface == EGL_NO_SURFACE)
            return nullptr;
        return std::make_unique<render::EglRenderTarget>(egl->eglDisplay(), surface, nullptr, size);
    }

    std::unique_ptr<Output::CompositedBuffer> acquireBuffer()
    {
        if (acquireFails)
            return nullptr;
        const auto current = eglGetCurrentSurface(EGL_DRAW);
        EGLint width = 0, height = 0;
        if (!eglQuerySurface(egl->eglDisplay(), current, EGL_WIDTH, &width)
            || !eglQuerySurface(egl->eglDisplay(), current, EGL_HEIGHT, &height))
            return nullptr;
        auto buffer = std::make_unique<Output::CompositedBuffer>();
        std::shared_ptr<gbm_surface> surface;
        gbm_bo *front = nullptr;
        if (gbmDevice) {
            for (auto it = nativeTargets.rbegin(); it != nativeTargets.rend(); ++it) {
                if (it->eglSurface == current) {
                    surface = it->surface;
                    break;
                }
            }
            EXPECT_TRUE(surface);
            if (!surface)
                return nullptr;
            front = gbm_surface_lock_front_buffer(surface.get());
            EXPECT_NE(front, nullptr);
            if (!front)
                return nullptr;
            EXPECT_EQ(gbm_bo_get_width(front), uint32_t(width));
            EXPECT_EQ(gbm_bo_get_height(front), uint32_t(height));
        }
        buffer->release = [this, surface, front] {
            if (front)
                gbm_surface_release_buffer(surface.get(), front);
            ++released;
        };
        buffer->primary.framebufferId = 1000 + ++acquired;
        buffer->primary.format = DRM_FORMAT_XRGB8888;
        buffer->primary.modifier = DRM_FORMAT_MOD_LINEAR;
        buffer->primary.sourceRect = {0, 0, width, height};
        acquiredSize = {width, height};
        if (checkPixel || recordPixel) {
            unsigned char pixel[4] = {255, 255, 255, 0};
            if (front) {
                // After swap, the EGL back buffer no longer represents the submitted frame.
                uint32_t stride = 0;
                void *mapping = nullptr;
                auto *data = static_cast<unsigned char *>(
                        gbm_bo_map(front, 0, 0, 1, 1, GBM_BO_TRANSFER_READ, &stride, &mapping));
                EXPECT_NE(data, nullptr);
                if (data) {
                    std::copy_n(data, 4, pixel);
                    gbm_bo_unmap(front, mapping);
                }
            } else {
                glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
                EXPECT_EQ(glGetError(), GLenum(GL_NO_ERROR));
            }
            // XRGB8888 lies blue first in memory; glReadPixels gives red first.
            lastPixel = front ? std::array<uint8_t, 3> {pixel[2], pixel[1], pixel[0]}
                              : std::array<uint8_t, 3> {pixel[0], pixel[1], pixel[2]};
            if (checkPixel) {
                EXPECT_EQ(pixel[0], *checkPixel);
                EXPECT_EQ(pixel[1], *checkPixel);
                EXPECT_EQ(pixel[2], *checkPixel);
            }
        }
        return buffer;
    }

    std::unique_ptr<Output> secondOutput(bool failTarget = false,
                                         backend::DrmDeviceApi *otherDevice = nullptr)
    {
        auto config = initialConfig;
        config.name = "injected-mst-sibling";
        if (!otherDevice) {
            config.connectorId = 4;
            config.crtcId = 8;
            config.crtcIndex = 1;
            config.planeId = 12;
            for (auto &plane : config.planes) {
                plane.id += 2;
                plane.possibleCrtcs = 2;
            }
        }
        auto result = std::make_unique<Output>(
                otherDevice ? otherDevice : &device, config,
                [&] {
                    return active;
                },
                true);
        auto target = createTarget({16, 16});
        if (!target
            || !result->init(
                    egl.get(), std::move(target),
                    [&] {
                        return acquireBuffer();
                    },
                    [this, failTarget](base::Size size) {
                        return failTarget ? std::unique_ptr<render::EglRenderTarget>()
                                          : createTarget(size);
                    }))
            return nullptr;
        return result;
    }

    bool waitEvents(size_t count)
    {
        return test::runUntil([&] { return events.size() >= count; }, std::chrono::milliseconds(3000));
    }

    Feedback &update()
    {
        auto &feedback = request();
        wl_surface_commit(surface);
        EXPECT_GE(roundtrip(), 0);
        return feedback;
    }

    void flip(size_t index, uint32_t sequence = 42)
    {
        ASSERT_LT(index, events.size());
        Output::dispatchPageFlip(events[index], 123, 456789, sequence);
        ASSERT_GE(roundtrip(), 0);
    }

    void promote()
    {
        storage = std::make_shared<ScanoutStorage>();
        compositor.surfaces().front()->node()->setScanoutBuffer(storage);
    }
};

TEST_F(DrmOutputTest, ProbedCursorIsThePreviousMastersCursorPlane)
{
    // What the pointer takes over at startup: an image on this CRTC's cursor
    // plane, whose position is signed, and nothing once the plane is off.
    auto config = initialConfig;
    backend::DrmPlane cursor;
    cursor.id = 12;
    cursor.type = DRM_PLANE_TYPE_CURSOR;
    cursor.possibleCrtcs = 1;
    uint32_t property = 40;
    for (const auto &name : Output::requiredPlaneProperties())
        cursor.properties.insert(name, property++, 0);
    cursor.properties.insert("CRTC_ID", 60, 7);
    cursor.properties.insert("FB_ID", 61, 99);
    cursor.properties.insert("CRTC_X", 62, uint64_t(int64_t(-3)));
    cursor.properties.insert("CRTC_Y", 63, 40);
    const auto probe = [&](const backend::DrmPlane &plane) {
        auto probed = config;
        probed.planes.push_back(plane);
        return Output(&device, probed, [] { return true; }, true).probedCursor();
    };
    EXPECT_EQ(probe(cursor), Lingmo::base::Point(-3, 40));
    auto off = cursor;
    off.properties.insert("FB_ID", 61, 0);
    EXPECT_EQ(probe(off), std::nullopt);
    auto elsewhere = cursor;
    elsewhere.properties.insert("CRTC_ID", 60, 8);
    EXPECT_EQ(probe(elsewhere), std::nullopt);
}

TEST_F(DrmOutputTest, ConfigurationTestDoesNotChangeModeOrRetireScanout)
{
    ASSERT_TRUE(drm->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    const int previousReleased = released;
    ASSERT_TRUE(Output::configure({{drm.get(), mode, true}}, true));
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_EQ(released, previousReleased + 1);
}

TEST_F(DrmOutputTest, ConfigurationFailurePreservesModeAndOldScanout)
{
    ASSERT_TRUE(drm->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    accept = [](const auto &commit) {
        return commit.flags & DRM_MODE_ATOMIC_TEST_ONLY;
    };
    const int previousReleased = released;
    EXPECT_FALSE(Output::configure({{drm.get(), mode, true}}, false));
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_EQ(released, previousReleased + 1);
}

TEST_F(DrmOutputTest, ConfigurationCommitChangesModeAndRetiresOldScanout)
{
    ASSERT_TRUE(drm->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    const int previousReleased = released;
    ASSERT_TRUE(Output::configure({{drm.get(), mode, true}}, false));
    EXPECT_EQ(drm->size(), base::Size(12, 16));
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_EQ(released, previousReleased + 1);
    ASSERT_TRUE(Output::configure({{drm.get(), mode, false}}, false));
    EXPECT_FALSE(drm->isEnabled());
    EXPECT_EQ(released, previousReleased + 2);
}

TEST_F(DrmOutputTest, ReconfigurationAcquiresFromTheRenderedTargetAfterCommitAndRollback)
{
    ASSERT_TRUE(drm->enable());
    flip(0);
    for (bool commit : {true, false}) {
        const base::Size previousSize = drm->size();
        auto mode = drm->mode();
        mode.hdisplay -= 4;
        accept = [commit](const auto &request) {
            return commit || (request.flags & DRM_MODE_ATOMIC_TEST_ONLY);
        };
        EXPECT_EQ(Output::configure({{drm.get(), mode, true}}, false), commit);
        EXPECT_EQ(acquiredSize, base::Size(mode.hdisplay, mode.vdisplay));
        const base::Size appliedSize = commit ? acquiredSize : previousSize;
        EXPECT_EQ(drm->size(), appliedSize);
        accept = {};
        const auto next = events.size();
        const auto previousAcquired = acquired;
        drm->scheduleFrame(Output::Reason::Scene);
        ASSERT_TRUE(waitEvents(next + 1));
        EXPECT_EQ(acquired, previousAcquired + 1);
        EXPECT_EQ(acquiredSize, appliedSize);
        flip(next);
    }
    drm.reset();
    EXPECT_EQ(acquired, released);
}

TEST_F(DrmOutputTest, BlankedScanoutKeepsBlackFrameUntilUnblanked)
{
    ASSERT_TRUE(drm->enable());
    checkPixel = 0;
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true, true}}, false));
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_TRUE(drm->isBlanked());
    const auto commits = device.commits.size();
    const auto buffers = acquired;
    drm->scheduleFrame(Output::Reason::Scene);
    test::processPendingEvents();
    EXPECT_EQ(device.commits.size(), commits);
    EXPECT_EQ(acquired, buffers);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    EXPECT_FALSE(drm->isBlanked());
    EXPECT_TRUE(drm->isEnabled());
}

TEST_F(DrmOutputTest, JointConfigurationTestsAndCommitsBothRoutesBeforeNotifying)
{
    auto second = secondOutput();
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    const std::vector<Output::Configuration> changes = {{drm.get(), mode, true},
                                                        {second.get(), mode, true}};
    int notifications = 0;
    const auto discarded = drm->frameDiscarded.connect([&](uint64_t) {
        ++notifications;
        EXPECT_EQ(drm->size(), base::Size(12, 16));
        EXPECT_EQ(second->size(), base::Size(12, 16));
        backend::DrmCommitStatus status;
        EXPECT_FALSE(Output::configure(changes, false, &status));
        EXPECT_EQ(status, backend::DrmCommitStatus::InvalidRequest);
    });
    device.commits.clear();
    ASSERT_TRUE(Output::configure(changes, false));
    EXPECT_EQ(notifications, 1);
    ASSERT_EQ(device.commits.size(), 2u);
    EXPECT_EQ(device.testCommitCount(), 1u);
    EXPECT_EQ(device.realCommitCount(), 1u);
    const auto &tested = device.commits.front().request.properties;
    const auto &committed = device.commits.back().request.properties;
    ASSERT_EQ(tested.size(), committed.size());
    std::set<uint32_t> objects;
    for (size_t i = 0; i < tested.size(); ++i) {
        objects.insert(tested[i].objectId);
        EXPECT_EQ(tested[i].objectId, committed[i].objectId);
        EXPECT_EQ(tested[i].propertyId, committed[i].propertyId);
        EXPECT_EQ(tested[i].value, committed[i].value);
    }
    for (uint32_t object : {3u, 4u, 7u, 8u, 10u, 12u})
        EXPECT_TRUE(objects.contains(object));
}

TEST_F(DrmOutputTest, JointTestFailureDoesNotCommitOrRetireEitherRoute)
{
    auto second = secondOutput();
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    device.commits.clear();
    const int previousReleased = released;
    accept = [](const auto &commit) {
        return !(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    backend::DrmCommitStatus status;
    EXPECT_FALSE(Output::configure({{drm.get(), mode, true}, {second.get(), mode, true}}, false,
                                   &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::TestFailed);
    EXPECT_EQ(device.realCommitCount(), 0u);
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_EQ(second->size(), base::Size(16, 16));
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_TRUE(second->isEnabled());
    EXPECT_EQ(released, previousReleased + 2);
}

TEST_F(DrmOutputTest, JointRealCommitFailurePreservesBothRoutesAndCanRetry)
{
    auto second = secondOutput();
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    auto mode = drm->mode();
    mode.hdisplay = 12;
    accept = [](const auto &commit) {
        return bool(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    backend::DrmCommitStatus status;
    const auto changes = std::vector<Output::Configuration> {{drm.get(), mode, true},
                                                             {second.get(), mode, true}};
    EXPECT_FALSE(Output::configure(changes, false, &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::CommitFailed);
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_EQ(second->size(), base::Size(16, 16));
    accept = {};
    ASSERT_TRUE(Output::configure(changes, false, &status));
    EXPECT_EQ(drm->size(), base::Size(12, 16));
    EXPECT_EQ(second->size(), base::Size(12, 16));
}

TEST_F(DrmOutputTest, LaterTargetAllocationFailureReleasesPreparedResourcesWithoutIoctl)
{
    auto second = secondOutput(true);
    ASSERT_TRUE(second);
    const int previousReleased = released;
    device.commits.clear();
    backend::DrmCommitStatus status;
    EXPECT_FALSE(Output::configure(
            {{drm.get(), drm->mode(), true}, {second.get(), second->mode(), true}}, false,
            &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::PreparationFailed);
    EXPECT_TRUE(device.commits.empty());
    EXPECT_EQ(released, previousReleased + 1);
}

TEST_F(DrmOutputTest, SessionLossDuringJointTestPreventsRealCommit)
{
    auto second = secondOutput();
    ASSERT_TRUE(second);
    device.commits.clear();
    accept = [&](const auto &commit) {
        if (commit.flags & DRM_MODE_ATOMIC_TEST_ONLY)
            active = false;
        return true;
    };
    backend::DrmCommitStatus status;
    EXPECT_FALSE(Output::configure(
            {{drm.get(), drm->mode(), true}, {second.get(), second->mode(), true}}, false,
            &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::Invalidated);
    EXPECT_EQ(device.realCommitCount(), 0u);
    EXPECT_FALSE(drm->isEnabled());
    EXPECT_FALSE(second->isEnabled());
    active = true;
}

TEST_F(DrmOutputTest, MultiDeviceSuccessTestsEveryDeviceBeforeAnyWriteOrNotification)
{
    Lingmo::testing::FakeDrmDevice other;
    auto second = secondOutput(false, &other);
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    std::vector<std::string> journal;
    const int before = released;
    auto record = [&](const auto &commit, const std::string &name) {
        journal.push_back((commit.flags & DRM_MODE_ATOMIC_TEST_ONLY ? "test-" : "commit-") + name);
        EXPECT_EQ(released, before);
        EXPECT_EQ(drm->size(), base::Size(16, 16));
        EXPECT_EQ(second->size(), base::Size(16, 16));
        return true;
    };
    accept = [&](const auto &commit) {
        return record(commit, "first");
    };
    other.commitFilter = [&](const auto &commit) {
        return record(commit, "second");
    };
    const auto discarded = drm->frameDiscarded.connect([&](uint64_t) {
        EXPECT_EQ(drm->size(), base::Size(12, 16));
        EXPECT_EQ(second->size(), base::Size(12, 16));
    });
    auto mode = drm->mode();
    mode.hdisplay = 12;
    ASSERT_TRUE(Output::configure({{drm.get(), mode, true}, {second.get(), mode, true}}, false));
    EXPECT_EQ(journal, std::vector<std::string>(
                               {"test-first", "test-second", "commit-first", "commit-second"}));
    EXPECT_EQ(released, before + 2);
    accept = {};
    other.commitFilter = {};
}

TEST_F(DrmOutputTest, LaterDeviceTestFailurePreventsEveryNativeCommit)
{
    Lingmo::testing::FakeDrmDevice other;
    auto second = secondOutput(false, &other);
    ASSERT_TRUE(second);
    other.commitFilter = [](const auto &) {
        return false;
    };
    backend::DrmCommitStatus status;
    backend::DrmDeviceApi *failed = nullptr;
    EXPECT_FALSE(Output::configure(
            {{drm.get(), drm->mode(), true}, {second.get(), second->mode(), true}}, false, &status,
            nullptr, &failed));
    EXPECT_EQ(status, backend::DrmCommitStatus::TestFailed);
    EXPECT_EQ(failed, &other);
    EXPECT_EQ(device.testCommitCount(), 1u);
    EXPECT_EQ(other.testCommitCount(), 1u);
    EXPECT_EQ(device.realCommitCount(), 0u);
    EXPECT_EQ(other.realCommitCount(), 0u);
    EXPECT_FALSE(drm->isEnabled());
}

TEST_F(DrmOutputTest, LaterDeviceCommitFailureRestoresEarlierNativeDevice)
{
    Lingmo::testing::FakeDrmDevice other;
    auto second = secondOutput(false, &other);
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    device.commits.clear();
    other.commits.clear();
    auto mode = drm->mode();
    mode.hdisplay = 12;
    other.commitFilter = [](const auto &commit) {
        return bool(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    backend::DrmCommitStatus status;
    std::string diagnostic;
    EXPECT_FALSE(Output::configure({{drm.get(), mode, true}, {second.get(), mode, true}}, false,
                                   &status, &diagnostic));
    EXPECT_EQ(status, backend::DrmCommitStatus::CommitFailed);
    EXPECT_EQ(device.realCommitCount(), 2u);
    EXPECT_EQ(other.realCommitCount(), 1u);
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_EQ(second->size(), base::Size(16, 16));
    EXPECT_NE(diagnostic.find("restored"), std::string::npos);
}

TEST_F(DrmOutputTest, FailedCompensationRetainsActuallyCommittedCandidate)
{
    Lingmo::testing::FakeDrmDevice other;
    auto second = secondOutput(false, &other);
    ASSERT_TRUE(second);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(second->enable());
    device.commits.clear();
    other.commits.clear();
    int writes = 0;
    accept = [&](const auto &commit) {
        return (commit.flags & DRM_MODE_ATOMIC_TEST_ONLY) || ++writes == 1;
    };
    other.commitFilter = [](const auto &commit) {
        return bool(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    auto mode = drm->mode();
    mode.hdisplay = 12;
    backend::DrmCommitStatus status;
    const int before = released;
    EXPECT_FALSE(Output::configure({{drm.get(), mode, true}, {second.get(), mode, true}}, false,
                                   &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::RollbackFailed);
    EXPECT_EQ(drm->size(), base::Size(12, 16));
    EXPECT_EQ(second->size(), base::Size(16, 16));
    EXPECT_EQ(released, before + 2);
    accept = {};
    EXPECT_TRUE(drm->retireForReprobe(true));
    EXPECT_EQ(released, before + 3);
}

TEST_F(DrmOutputTest, InvalidationInsideFinalDeviceCommitCannotPublishSuccess)
{
    Lingmo::testing::FakeDrmDevice other;
    auto second = secondOutput(false, &other);
    ASSERT_TRUE(second);
    other.commitFilter = [&](const auto &commit) {
        if (!(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY))
            active = false;
        return true;
    };
    backend::DrmCommitStatus status;
    EXPECT_FALSE(Output::configure(
            {{drm.get(), drm->mode(), true}, {second.get(), second->mode(), true}}, false,
            &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::Invalidated);
    EXPECT_EQ(device.realCommitCount(), 2u);
    EXPECT_EQ(other.realCommitCount(), 2u);
    EXPECT_FALSE(drm->isEnabled());
    EXPECT_FALSE(second->isEnabled());
    active = true;
}

TEST_F(DrmOutputTest, FallbackTestsCombinedBandwidthAndCommitsTheSelectedNativeModes)
{
    auto second = secondOutput();
    ASSERT_TRUE(second);
    auto high = drm->mode();
    auto low = high;
    low.hdisplay = 12;
    low.clock = 20;
    accept = [](const auto &commit) {
        uint64_t width = 0;
        for (const auto &property : commit.request.properties)
            if ((property.objectId == 10 || property.objectId == 12) && property.propertyId == 9)
                width += property.value;
        return width <= 28;
    };
    ASSERT_TRUE(Output::configure({{drm.get(), high, true}}, true));
    ASSERT_TRUE(Output::configure({{second.get(), high, true}}, true));
    device.commits.clear();
    const std::vector<backend::DrmFallbackHead> heads = {
            {3, "eDP-1", "internal", true, {{"high", 16, 16, 60000, 30, true}}},
            {4,
             "DP-1-1",
             "DP-1",
             false,
             {{"high", 16, 16, 60000, 30, true}, {"low", 12, 16, 60000, 20, false}}}};
    std::vector<Output::Configuration> selected;
    const auto plan = backend::DrmModeFallbackPlanner::choose(
            heads,
            [&](const auto &modes) {
                selected = {{drm.get(), high, true},
                            {second.get(), modes[1].key == "high" ? high : low, true}};
                backend::DrmCommitStatus status;
                const bool accepted = Output::configure(selected, true, &status);
                EXPECT_EQ(drm->size(), base::Size(16, 16));
                EXPECT_EQ(second->size(), base::Size(16, 16));
                EXPECT_EQ(device.realCommitCount(), 0u);
                return backend::DrmFallbackTestResult {
                        accepted,
                        {"DP-1"},
                        backend::drmCommitMessage(status),
                        status == backend::DrmCommitStatus::TestFailed};
            },
            8);
    ASSERT_TRUE(plan);
    EXPECT_EQ(plan->attempts, 2);
    EXPECT_EQ(plan->downgradedConnectors, std::vector<std::string>({"DP-1-1"}));
    ASSERT_TRUE(Output::configure(selected, false));
    EXPECT_EQ(drm->size(), base::Size(16, 16));
    EXPECT_EQ(second->size(), base::Size(12, 16));
    EXPECT_EQ(device.testCommitCount(), 3u);
    EXPECT_EQ(device.realCommitCount(), 1u);
}

TEST_F(DrmOutputTest, DuplicateRoutesAreRejectedBeforeAllocation)
{
    const int previousAcquired = acquired;
    backend::DrmCommitStatus status;
    EXPECT_FALSE(Output::configure({{drm.get(), drm->mode(), true}, {drm.get(), drm->mode(), true}},
                                   false, &status));
    EXPECT_EQ(status, backend::DrmCommitStatus::InvalidRequest);
    EXPECT_EQ(acquired, previousAcquired);
    EXPECT_TRUE(device.commits.empty());
}

TEST_F(DrmOutputTest, InitialModesetHasMeasuredWireCompletionAndSchedulesAgain)
{
    auto *binding = bindOutput();
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(clockId, uint32_t(CLOCK_MONOTONIC));
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    ASSERT_EQ(events.size(), 1u);
    EXPECT_TRUE(device.commits.back().flags & DRM_MODE_PAGE_FLIP_EVENT);
    EXPECT_TRUE(device.commits.back().flags & DRM_MODE_ATOMIC_ALLOW_MODESET);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(a.events.empty());
    EXPECT_EQ(released, 0);
    flip(0);
    EXPECT_EQ(a.events, std::vector<char>({'o', 'p'}));
    EXPECT_EQ(a.outputs, std::vector<wl_output *> {binding});
    EXPECT_EQ(a.secondsHi, 0u);
    EXPECT_EQ(a.secondsLo, 123u);
    EXPECT_EQ(a.ns, 456789000u);
    EXPECT_EQ(a.seqLo, 42u);
    EXPECT_EQ(a.refresh, 13333333u);
    EXPECT_EQ(a.flags, 7u);
    EXPECT_EQ(pacing, 1);
    EXPECT_EQ(released, 0);
    auto &b = update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    EXPECT_TRUE(b.events.empty());
    flip(1, 43);
    EXPECT_EQ(b.events, std::vector<char>({'o', 'p'}));
    EXPECT_EQ(released, 1);
    flip(0);
    flip(1);
    EXPECT_EQ(pacing, 2);
    EXPECT_EQ(b.events.size(), 2u);
}

double slowestDrawMs(const Output &output)
{
    const std::vector<std::string> fields = base::strings::split(output.statsLine(), ' ');
    const auto field = std::find(fields.begin(), fields.end(), "slowest-draw-ms");
    if (field == fields.end() || field + 1 == fields.end())
        return -1;
    return base::strings::toDouble(*(field + 1)).value_or(0);
}

// The scheduler learns what composition takes on this GPU from every frame
// that lands, whether a timestamp query or a fence timed it.
TEST_F(DrmOutputTest, EachLandedFrameTellsTheSchedulerWhenItWasDrawn)
{
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_EQ(slowestDrawMs(*drm), 0);
    for (unsigned frame = 1; frame <= 3; ++frame) {
        update();
        drm->scheduleFrame();
        ASSERT_TRUE(waitEvents(frame + 1));
        // Flips come at once here; a real one lands once the GPU is done.
        glFinish();
        flip(frame, 42 + frame);
    }
    if (!egl->marksDrawn() && !gbmDevice)
        GTEST_SKIP() << "Neither timestamp queries nor native fences";
    EXPECT_GT(slowestDrawMs(*drm), 0);
}

// virtio-gpu and others have neither CTM nor gamma LUT; Ash composites a
// colour matrix there, as the output does here over every frame.
TEST_F(DrmOutputTest, WithoutColourPropertiesTheTintIsBlendedOverEachFrame)
{
    auto config = initialConfig;
    config.crtcProperties = {};
    config.crtcProperties.insert("MODE_ID", 20, 0);
    config.crtcProperties.insert("ACTIVE", 21, 0);
    config.crtcProperties.insert("OUT_FENCE_PTR", 23, 0);
    // The fixture's output drives the same CRTC.
    drm.reset();
    drm = std::make_unique<Output>(
            &device, config,
            [&] {
                return active;
            },
            true);
    ASSERT_TRUE(drm->init(
            egl.get(), createTarget({16, 16}),
            [&] {
                return acquireBuffer();
            },
            [&](base::Size size) {
                return createTarget(size);
            }));
    drm->setRenderCallback([](render::Renderer &renderer, uint64_t) {
        renderer.clear({0.8f, 0.8f, 0.8f, 1});
    });
    recordPixel = true;
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_FALSE(drm->supportsColorTransform());
    EXPECT_EQ(lastPixel, (std::array<uint8_t, 3> {204, 204, 204}));

    drm->setColorTransform({1.0f, 0.5f, 0.25f});
    EXPECT_TRUE(drm->blendsColorTransform());
    ASSERT_TRUE(waitEvents(2));
    EXPECT_EQ(lastPixel, (std::array<uint8_t, 3> {204, 102, 51}));
    // Nothing went to the CRTC for it.
    EXPECT_FALSE(propertyValue(device.commits.back().request, 7, 24));
    flip(1);

    drm->setColorTransform({});
    EXPECT_FALSE(drm->blendsColorTransform());
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(lastPixel, (std::array<uint8_t, 3> {204, 204, 204}));
    flip(2);
}

// virtio-gpu has no vblank: every flip carries sequence 0, and the mode's
// refresh is QEMU's invention. Two such flips switch the output to 60 Hz.
TEST_F(DrmOutputTest, AnOutputWithoutVblankIsPacedAtSixtyHertz)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    flip(0, 0);
    EXPECT_EQ(a.refresh, 13333333u);
    EXPECT_NEAR(drm->pacingRate(), drm->refreshRate(), 0.01);
    auto &b = update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 0);
    EXPECT_EQ(b.refresh, 16666667u);
    EXPECT_EQ(drm->pacingRate(), 60.0);
    EXPECT_NEAR(drm->refreshRate(), 75.0, 0.01);
}

// A frame drawn behind a flip that missed its vblank would land a refresh
// late, and so would every frame begun before the queue drains.
TEST_F(DrmOutputTest, NoFrameIsDrawnBehindALateFlip)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    // One flip on its way to the screen, and it misses its vblank.
    auto &b = update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    auto &c = update();
    drm->scheduleFrame();
    test::runFor(std::chrono::milliseconds(60));
    EXPECT_EQ(events.size(), 2u);
    EXPECT_EQ(prepared, 2);
    EXPECT_TRUE(c.events.empty());
    // Once it lands, the next frame is drawn in its own slot and lands on the
    // following vblank.
    flip(1, 43);
    EXPECT_EQ(b.events, std::vector<char> {'p'});
    EXPECT_EQ(b.seqLo, 43u);
    EXPECT_EQ(released, 1);
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(prepared, 3);
    EXPECT_TRUE(c.events.empty());
    flip(2, 44);
    EXPECT_EQ(c.events, std::vector<char> {'p'});
    EXPECT_EQ(c.seqLo, 44u);
    EXPECT_EQ(released, 2);
    EXPECT_EQ(pacing, 3);
}

// exo completes wl_surface.frame when viz acks the frame it drew
// (Display::DrawAndSwap -> RunDrawCallbacks), not when the flip lands. Held
// for the flip, a client whose frame was queued behind another waited two
// refreshes for its next one.
TEST_F(DrmOutputTest, FrameCallbacksCompleteWhenTheFrameIsDrawnNotWhenItLands)
{
    // The fixture's bare display has no Server to send wl_callback.done
    // through, so the callback is taken where the protocol hands it over.
    const auto requestFrame = [&](int &done) {
        compositor.surfaces().front()->requestFrameCallback(
                [&done](std::optional<uint32_t> time) {
                    if (time)
                        ++done;
                });
    };
    ASSERT_TRUE(drm->enable());
    flip(0);
    int first = 0;
    requestFrame(first);
    update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    EXPECT_EQ(first, 1);
    // The next frame, drawn once that flip lands, completes it before its own
    // flip does.
    int second = 0;
    requestFrame(second);
    update();
    drm->scheduleFrame();
    flip(1, 43);
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(prepared, 3);
    EXPECT_EQ(second, 1);
    flip(2, 44);
    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 1);
}

TEST_F(DrmOutputTest, ALostFlipEventStopsHoldingLaterFrames)
{
    drm->setFlipTimeout(std::chrono::milliseconds(50));
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    auto &b = update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    // B's event never arrives. C is drawn behind it and waits, until the
    // output gives up on B and commits C.
    auto &c = update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(b.events, std::vector<char> {'d'});
    EXPECT_TRUE(c.events.empty());
    EXPECT_EQ(pacing, 2);
    flip(2, 44);
    EXPECT_EQ(c.events, std::vector<char> {'p'});
    EXPECT_EQ(c.seqLo, 44u);
    // B's event arriving late completes nothing twice.
    flip(1, 43);
    EXPECT_EQ(b.events, std::vector<char> {'d'});
    EXPECT_EQ(pacing, 3);
}

TEST_F(DrmOutputTest, AFrameWithNothingToDrawBeginsButIsNeitherDrawnNorFlipped)
{
    std::vector<double> begun;
    bool needed = false;
    drm->setBeginFrameCallback([&](double frameTime) {
        begun.push_back(frameTime);
        return needed;
    });
    const auto settle = [] {
        test::runFor(std::chrono::milliseconds(60));
    };
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    // Enabled, the output still owes the screen a frame of its own.
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1);
    const int drawn = prepared;
    const size_t began = begun.size();
    drm->scheduleFrame();
    settle();
    EXPECT_EQ(begun.size(), began + 1);
    EXPECT_EQ(prepared, drawn);
    EXPECT_EQ(events.size(), 2u);
    needed = true;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    EXPECT_GT(begun.back(), begun.front());
    flip(2);
    // A frame committed under another policy is drawn with or without damage.
    needed = false;
    drm->setSecureScanout(true);
    ASSERT_TRUE(waitEvents(4));
}

// The night light tints through the CRTC: a flip carries the tint when nothing
// on screen changed, and an output that wakes up modesets with it in place.
TEST_F(DrmOutputTest, ATintFlipsWithNothingNewToDrawAndSurvivesAWake)
{
    drm->setBeginFrameCallback([](double) {
        return false;
    });
    const auto settle = [] {
        test::runFor(std::chrono::milliseconds(60));
    };
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    ASSERT_TRUE(drm->supportsColorTransform());

    drm->setColorTransform({1.0f, 0.75f, 0.5f});
    ASSERT_TRUE(waitEvents(2));
    const auto &tinted = device.commits.back();
    EXPECT_FALSE(tinted.flags & DRM_MODE_ATOMIC_ALLOW_MODESET);
    const uint64_t blob = propertyValue(tinted.request, 7, 24);
    EXPECT_NE(blob, 0u);
    flip(1);

    // The same tint again asks for nothing.
    const size_t commits = device.commits.size();
    drm->setColorTransform({1.0f, 0.75f, 0.5f});
    settle();
    EXPECT_EQ(device.commits.size(), commits);

    drm->disable();
    ASSERT_TRUE(drm->enable());
    const auto modeset = std::find_if(device.commits.rbegin(), device.commits.rend(),
                                      [](const auto &commit) {
                                          return (commit.flags & DRM_MODE_ATOMIC_ALLOW_MODESET)
                                                  && !(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY)
                                                  && propertyValue(commit.request, 7, 21) == 1;
                                      });
    ASSERT_NE(modeset, device.commits.rend());
    EXPECT_EQ(propertyValue(modeset->request, 7, 24), blob);
}

TEST_F(DrmOutputTest, DisablingDiscardsTheQueuedFrameAndReleasesItsBuffer)
{
    // Only flips without a vblank stamp leave a frame queued behind another.
    update();
    ASSERT_TRUE(drm->enable());
    flip(0, 0);
    update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 0);
    update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    auto &c = update();
    drm->scheduleFrame();
    test::runFor(std::chrono::milliseconds(60));
    ASSERT_EQ(acquired, 4);
    std::vector<uint64_t> discarded;
    auto connection = drm->frameDiscarded.connect([&](uint64_t frame) {
        discarded.push_back(frame);
    });
    active = false;
    drm->disable();
    EXPECT_EQ(discarded, (std::vector<uint64_t> {events[2].frame, events[2].frame + 1}));
    // Once discarded, a frame owes nothing more.
    drm->disable();
    EXPECT_EQ(discarded.size(), 2u);
    connection.reset();
    EXPECT_EQ(events.size(), 3u);
    // Only the queued frame's buffer is free; the flipped ones are retained
    // until a modeset replaces them. Its content never reached the device, so
    // its feedback stays pending for a later frame rather than being discarded.
    EXPECT_EQ(released, 2);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(c.events.empty());
}

TEST_F(DrmOutputTest, SubmittedContentSurvivesNewerAppliedContent)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    auto &b = update();
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    EXPECT_TRUE(b.events.empty());
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    EXPECT_EQ(b.seqLo, 43u);
}

TEST_F(DrmOutputTest, RetainedResumeAndOldTokensAreIndependent)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    active = false;
    drm->disable();
    auto &b = update();
    drm->scheduleFrame();
    EXPECT_EQ(released, 0);
    active = true;
    ASSERT_TRUE(drm->enable());
    ASSERT_EQ(events.size(), 2u);
    EXPECT_NE(events[0].frame, events[1].frame);
    EXPECT_TRUE(a.events.empty());
    flip(1);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    flip(0);
    EXPECT_EQ(a.events.size(), 1u);
    ASSERT_TRUE(waitEvents(3));
    flip(2, 43);
    EXPECT_EQ(b.events, std::vector<char> {'p'});
}

TEST_F(DrmOutputTest, FailedCommitsKeepRetryableFeedbackAndReleaseAcquiredTargets)
{
    auto &a = update();
    accept = [](const auto &) {
        return false;
    };
    EXPECT_FALSE(drm->enable());
    EXPECT_TRUE(events.empty());
    EXPECT_EQ(acquired, released);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(a.events.empty());
    accept = {};
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
}

TEST_F(DrmOutputTest, OutputDestructionInvalidatesQueuedCompletion)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    drm.reset();
    EXPECT_EQ(acquired, released);
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'d'});
    wl_surface_destroy(surface);
    surface = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(a.events, std::vector<char> {'d'});
}

TEST_F(DrmOutputTest, PromotionFailureFallsBackToComposedFeedback)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &b = update();
    promote();
    accept = [](const auto &commit) {
        if (commit.flags & DRM_MODE_ATOMIC_TEST_ONLY)
            return true;
        for (const auto &p : commit.request.properties)
            if (p.objectId == 10 && p.propertyId == 1 && p.value < 1000)
                return false;
        return true;
    };
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    EXPECT_GT(device.testCommitCount(), 0u);
    EXPECT_EQ(prepared, 2);
    EXPECT_EQ(drm->directScanoutFrames(), 0u);
    EXPECT_TRUE(b.events.empty());
    EXPECT_EQ(storage->references, 0);
    flip(1, 43);
    EXPECT_EQ(b.flags, 7u);
    EXPECT_EQ(b.events, std::vector<char> {'p'});
}

TEST_F(DrmOutputTest, ScaleFallbackAndScanoutRetirementRemainSeparate)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &b = update();
    promote();
    scale = 1.25;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    EXPECT_EQ(b.flags, 7u);
    EXPECT_EQ(storage->references, 0);
    auto &c = update();
    promote();
    scale = 1;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(storage->references, 1);
    flip(2, 44);
    EXPECT_EQ(c.flags, 15u);
    EXPECT_EQ(storage->references, 1);
    EXPECT_EQ(storage->releases, 0);
    scale = 1.25;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(4));
    EXPECT_EQ(storage->references, 1);
    flip(3, 45);
    EXPECT_EQ(storage->references, 0);
    EXPECT_EQ(storage->releases, 1);
}

TEST_F(DrmOutputTest, MixedCompositionAndOverlayHaveDifferentWireFlags)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &background = update();
    auto *backgroundSurface = surface;
    surface = wl_compositor_create_surface(factory);
    int fd = testsupport::makeMemfd(256);
    ASSERT_GE(fd, 0);
    auto *pool = wl_shm_create_pool(shm, fd, 256);
    close(fd);
    auto *overlayBuffer = wl_shm_pool_create_buffer(pool, 0, 8, 8, 32, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    wl_surface_attach(surface, overlayBuffer, 0, 0);
    auto &overlay = update();
    compositor.updateTextures(*egl);
    compositor.surfaces().back()->setPosition({0, 0});
    auto *top = compositor.surfaces().back()->node();
    storage = std::make_shared<ScanoutStorage>(base::Size(8, 8));
    top->setScanoutBuffer(storage);
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    EXPECT_EQ(drm->overlayFrames(), 1u);
    EXPECT_TRUE(background.events.empty());
    EXPECT_TRUE(overlay.events.empty());
    flip(1, 43);
    EXPECT_EQ(background.flags, 7u);
    EXPECT_EQ(overlay.flags, 15u);
    EXPECT_EQ(storage->references, 1);

    const std::array<uint8_t, 4> blue {30, 90, 180, 255};
    const std::vector<uint8_t> color = solidRgba(base::Size(8, 8), blue);
    top->setTexture(egl->createTexture(base::Size(8, 8), color.data()));
    std::vector<std::array<uint8_t, 4>> samples;
    drm->setRenderCallback([&](render::Renderer &renderer, uint64_t) {
        scene.render(renderer);
        uint8_t pixel[4] = {};
        glReadPixels(2, 13, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        samples.push_back({pixel[0], pixel[1], pixel[2], pixel[3]});
    });
    accept = [](const auto &commit) {
        if (commit.flags & DRM_MODE_ATOMIC_TEST_ONLY)
            return true;
        for (const auto &property : commit.request.properties)
            if (property.objectId == 11 && property.propertyId == 1 && property.value)
                return false;
        return true;
    };
    auto &fallback = update();
    const int previousPrepares = prepared;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(prepared, previousPrepares + 1);
    ASSERT_EQ(samples.size(), 2u);
    EXPECT_NE(samples.front(), blue);
    EXPECT_EQ(samples.back(), blue);
    EXPECT_EQ(storage->references, 1);
    flip(2);
    EXPECT_EQ(fallback.flags, 7u);
    EXPECT_EQ(storage->references, 0);
    EXPECT_EQ(drm->overlayFrames(), 1u);
    drm->setRenderCallback({});
    wl_surface_destroy(surface);
    wl_buffer_destroy(overlayBuffer);
    surface = backgroundSurface;
    ASSERT_GE(roundtrip(), 0);
}

TEST_F(DrmOutputTest, RetirementPassesTheReplacementFenceAndKeepsItOwned)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1);
    EXPECT_EQ(storage->references, 1);
    outFence.reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
    ASSERT_TRUE(outFence.isValid());
    scale = 1.25;
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(storage->references, 1);
    EXPECT_FALSE(storage->retiredFence.isValid());
    flip(2);
    EXPECT_EQ(storage->references, 0);
    EXPECT_TRUE(storage->retiredFence.isValid());
    EXPECT_FALSE(storage->retiredFence.isSignaled());
    const uint64_t value = 1;
    ASSERT_EQ(write(outFence.get(), &value, sizeof(value)), sizeof(value));
    EXPECT_TRUE(storage->retiredFence.isSignaled());
}

TEST_F(DrmOutputTest, DisconnectedClientDoesNotInvalidateAcceptedToken)
{
    update();
    ASSERT_TRUE(drm->enable());
    for (auto &f : feedbacks) {
        if (f->proxy)
            wp_presentation_feedback_destroy(f->proxy);
        f->proxy = nullptr;
    }
    wl_surface_destroy(surface);
    surface = nullptr;
    wl_buffer_destroy(buffer);
    buffer = nullptr;
    wp_presentation_destroy(manager);
    manager = nullptr;
    wl_shm_destroy(shm);
    shm = nullptr;
    wl_compositor_destroy(factory);
    factory = nullptr;
    wl_registry_destroy(registry);
    registry = nullptr;
    wl_display_disconnect(client);
    client = nullptr;
    wl_display_destroy_clients(server);
    Output::dispatchPageFlip(events.front(), 123, 456789, 42);
    EXPECT_EQ(pacing, 1);
    EXPECT_EQ(released, 0);
}

TEST_F(DrmOutputTest, MissingModeBlobAndUninitializedOutputAreSafe)
{
    auto &a = update();
    device.blobFails = true;
    EXPECT_FALSE(drm->enable());
    EXPECT_TRUE(events.empty());
    EXPECT_TRUE(a.events.empty());
    Output uninitialized(
            &device, {},
            [] {
                return false;
            },
            true);
    EXPECT_FALSE(uninitialized.enable());
}

TEST_F(DrmOutputTest, ElapsedTimeAndStaleEventsCannotPresentOrRetirePendingContent)
{
    auto &a = update();
    ASSERT_TRUE(drm->enable());
    clockOffset += std::chrono::minutes(5);
    drm->scheduleFrame();
    test::processPendingEvents();
    Output::dispatchPageFlip({drm->weakPtr(), events.front().frame + 100}, 123, 456789, 42);
    ASSERT_GE(roundtrip(), 0);
    EXPECT_TRUE(a.events.empty());
    // The blocking modeset is on screen, so the next frame flips behind it
    // without waiting for its event.
    EXPECT_EQ(events.size(), 2u);
    EXPECT_EQ(released, 0);
    EXPECT_EQ(pacing, 0);
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    flip(1);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
}

TEST_F(DrmOutputTest, OnDemandRetryAfterBothAttemptsFailUsesTheSameContent)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &b = update();
    int failures = 0;
    accept = [&](const auto &commit) {
        if (!(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY) && failures < 2) {
            ++failures;
            return false;
        }
        return true;
    };
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    EXPECT_EQ(failures, 2);
    EXPECT_TRUE(b.events.empty());
    EXPECT_EQ(acquired - released, 2);
    flip(1, 43);
    EXPECT_EQ(b.events, std::vector<char> {'p'});
    EXPECT_EQ(b.flags, 7u);
}

TEST_F(DrmOutputTest, FailedTargetAcquisitionCanBeRetriedOrAbandoned)
{
    auto &a = update();
    acquireFails = true;
    EXPECT_FALSE(drm->enable());
    EXPECT_EQ(acquired, 0);
    EXPECT_TRUE(events.empty());
    EXPECT_TRUE(a.events.empty());
    acquireFails = false;
    ASSERT_TRUE(drm->enable());
    flip(0);
    EXPECT_EQ(a.events, std::vector<char> {'p'});
    auto &b = update();
    wl_surface_destroy(surface);
    surface = nullptr;
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(b.events, std::vector<char> {'d'});
}

TEST_F(DrmOutputTest, ResumeRetainsPendingBufferUntilMeasuredCompletion)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &b = update();
    ASSERT_TRUE(waitEvents(2));
    active = false;
    drm->disable();
    auto &c = update();
    EXPECT_EQ(acquired - released, 2);
    active = true;
    ASSERT_TRUE(drm->enable());
    ASSERT_EQ(events.size(), 3u);
    EXPECT_NE(events[1].frame, events[2].frame);
    EXPECT_EQ(acquired - released, 1);
    EXPECT_TRUE(b.events.empty());
    flip(2, 44);
    EXPECT_EQ(b.events, std::vector<char> {'p'});
    EXPECT_TRUE(c.events.empty());
    flip(1, 43);
    EXPECT_EQ(b.events.size(), 1u);
    ASSERT_TRUE(waitEvents(4));
    flip(3, 45);
    EXPECT_EQ(c.events, std::vector<char> {'p'});
}

TEST_F(DrmOutputTest, NonparticipatingContentIsDiscardedAtSuccessfulSubmission)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    compositor.surfaces().front()->setPosition({100, 100});
    auto &hidden = update();
    ASSERT_TRUE(waitEvents(2));
    ASSERT_GE(roundtrip(), 0);
    EXPECT_EQ(hidden.events, std::vector<char> {'d'});
    flip(1);
    EXPECT_EQ(hidden.events.size(), 1u);
}

} // namespace

TEST_F(DrmOutputTest, HotplugRetirementKeepsRawFlipCookieAliveAfterOutputDestruction)
{
    ASSERT_TRUE(drm->enable());
    update();
    ASSERT_TRUE(waitEvents(1));
    Output::FlipEvent *raw = nullptr;
    for (const auto &commit : device.commits)
        if (commit.userData)
            raw = static_cast<Output::FlipEvent *>(commit.userData);
    ASSERT_NE(raw, nullptr);
    std::vector<std::unique_ptr<Output::FlipEvent>> retired;
    drm->setEventRetirementHandler([&](auto event) {
        retired.push_back(std::move(event));
    });
    ASSERT_TRUE(drm->retireForReprobe(false));
    EXPECT_FALSE(drm->isEnabled());
    const int before = released;
    drm.reset();
    ASSERT_FALSE(retired.empty());
    EXPECT_EQ(retired.front().get(), raw);
    EXPECT_FALSE(raw->output);
    Output::dispatchPageFlip(*raw, 123, 456, 1);
    EXPECT_EQ(released, before);
}

TEST_F(DrmOutputTest, FailedRetirementRetainsScanoutAndRetryOmitsVanishedConnector)
{
    ASSERT_TRUE(drm->enable());
    const int before = released;
    accept = [](const auto &) {
        return false;
    };
    EXPECT_FALSE(drm->retireForReprobe(false));
    EXPECT_EQ(released, before);
    accept = [](const auto &commit) {
        for (const auto &property : commit.request.properties)
            EXPECT_NE(property.objectId, 3u);
        return true;
    };
    EXPECT_TRUE(drm->retireForReprobe(false));
    EXPECT_GT(released, before);
    accept = {};
}

TEST_F(DrmOutputTest, RetirementThatKeepsItsRouteLeavesTheFrameOnScreen)
{
    ASSERT_TRUE(drm->enable());
    const auto commits = device.commits.size();
    const int before = released;
    // The replacement drives the same connector from the same CRTC, and its
    // first commit replaces this frame; turning the route off in between would
    // be a black frame and, on a panel, a power cycle.
    EXPECT_TRUE(drm->retireForReprobe(true, true));
    EXPECT_EQ(device.commits.size(), commits);
    EXPECT_GT(released, before);
    EXPECT_FALSE(drm->isEnabled());
}

TEST_F(DrmOutputTest, RetirementThatKeepsItsRouteStillTurnsOffAClientBuffer)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    ASSERT_EQ(drm->directScanoutFrames(), 1u);
    const auto commits = device.realCommitCount();
    // The client gets its buffer back only once KMS has stopped reading it.
    EXPECT_TRUE(drm->retireForReprobe(true, true));
    ASSERT_EQ(device.realCommitCount(), commits + 1);
    bool found = false;
    EXPECT_EQ(propertyValue(device.commits.back().request, 7, 21, &found), 0u);
    EXPECT_TRUE(found) << "the CRTC was not turned off";
}

TEST_F(DrmOutputTest, ModesetBufferCarriesOverTheFrameTheCrtcShows)
{
    const ScopedFd node(open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC));
    if (!node.isValid())
        GTEST_SKIP() << "No render node for the previous DRM master's buffer";
    std::unique_ptr<gbm_device, decltype(&gbm_device_destroy)> gbm(gbm_create_device(node.get()),
                                                                   gbm_device_destroy);
    std::unique_ptr<gbm_bo, decltype(&gbm_bo_destroy)> bo(
            gbm ? gbm_bo_create(gbm.get(), 16, 16, GBM_FORMAT_XRGB8888,
                                GBM_BO_USE_LINEAR | GBM_BO_USE_RENDERING)
                : nullptr,
            gbm_bo_destroy);
    if (!bo)
        GTEST_SKIP() << "No linear buffer on the render node";
    uint32_t stride = 0;
    void *mapping = nullptr;
    auto *pixels = static_cast<uint8_t *>(
            gbm_bo_map(bo.get(), 0, 0, 16, 16, GBM_BO_TRANSFER_WRITE, &stride, &mapping));
    ASSERT_NE(pixels, nullptr);
    for (int row = 0; row < 16; ++row)
        std::fill_n(pixels + row * stride, 16 * 4, uint8_t(0x80));
    gbm_bo_unmap(bo.get(), mapping);
    const auto shown = [&](base::Size size, render::DmaBufAttributes *attributes) {
        attributes->size = size;
        attributes->format = DRM_FORMAT_XRGB8888;
        attributes->modifier = DRM_FORMAT_MOD_LINEAR;
        attributes->planes.push_back({ScopedFd(gbm_bo_get_fd(bo.get())), 0, stride});
        return true;
    };
    ASSERT_TRUE(drm->enable());
    render::DmaBufAttributes probe;
    shown({16, 16}, &probe);
    if (!egl->importDmaBuf(probe))
        GTEST_SKIP() << "This renderer cannot sample the render node's buffers";

    device.crtcFramebuffer = [&](uint32_t, render::DmaBufAttributes *attributes) {
        return shown({16, 16}, attributes);
    };
    // A test commit leaves the screen alone, so it has nothing to carry over.
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, true));
    EXPECT_TRUE(device.exportedCrtcs.empty());
    checkPixel = 0x80;
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    EXPECT_EQ(device.exportedCrtcs, std::vector<uint32_t> {7});
    EXPECT_TRUE(drm->isEnabled());
    EXPECT_FALSE(drm->takeCarriedFrame());

    // Kept for the startup cover, the copy outlives the modeset buffer.
    drm->setKeepCarriedFrame(true);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    const auto carried = drm->takeCarriedFrame();
    ASSERT_TRUE(carried);
    EXPECT_EQ(carried->size(), base::Size(16, 16));
    EXPECT_FALSE(drm->takeCarriedFrame());
    drm->setKeepCarriedFrame(false);

    // The login screen is blocked from its first frame, yet carries over too.
    drm->setSecureScanout(true);
    drm->setCarryOverWhileSecure(true);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    EXPECT_EQ(device.exportedCrtcs.size(), 3u);
    drm->setSecureScanout(false);
    drm->setCarryOverWhileSecure(false);
    device.exportedCrtcs.clear();

    // A frame of another size, as a mode change leaves it, is not stretched.
    device.crtcFramebuffer = [&](uint32_t, render::DmaBufAttributes *attributes) {
        return shown({8, 8}, attributes);
    };
    checkPixel = 0;
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true, true}}, false));
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    EXPECT_EQ(device.exportedCrtcs.size(), 1u);
}

TEST_F(DrmOutputTest, ModesetOfABlockedOrBlankedOutputStaysBlack)
{
    device.crtcFramebuffer = [](uint32_t, render::DmaBufAttributes *) {
        ADD_FAILURE() << "the CRTC's frame was read";
        return false;
    };
    checkPixel = 0;
    // A blocked session cannot present a frame it did not compose as secure.
    drm->setSecureScanout(true);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure());
    drm->setSecureScanout(false);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true, true}}, false));
    EXPECT_TRUE(drm->isBlanked());
    EXPECT_TRUE(device.exportedCrtcs.empty());
}

TEST_F(DrmOutputTest, PopupDemotionAndReturnRetireOnlyOnReplacementFlip)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1);
    ASSERT_EQ(storage->references, 1);
    auto *popup =
            scene.root()->addChild(std::make_unique<scene::ColorNode>(render::Color {0, 1, 0, 1}));
    popup->setGeometry({0, 0, 8, 8});
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(storage->references, 1);
    flip(2);
    EXPECT_EQ(storage->references, 0);
    popup->setVisible(false);
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(4));
    flip(3);
    EXPECT_EQ(storage->references, 1);
    EXPECT_EQ(drm->directScanoutFrames(), 2u);
}

TEST_F(DrmOutputTest, TestOnlyRejectionComposesTheSameFrameAndCanPromoteAgain)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    auto &feedback = update();
    promote();
    accept = [](const auto &commit) {
        return !(commit.flags & DRM_MODE_ATOMIC_TEST_ONLY);
    };
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1);
    EXPECT_EQ(feedback.flags, 7u);
    EXPECT_EQ(storage->references, 0);
    accept = {};
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    flip(2);
    EXPECT_EQ(storage->references, 1);
}

TEST_F(DrmOutputTest, ScreenshotForcesCompositionUntilPresentedThenResumesStaticScanout)
{
    wayland::ImageCaptureManager capture(
            server,
            [](const wl_client *) {
                return true;
            },
            [&] {
                drm->scheduleFrame();
            });
    auto *captureGlobal = wayland::createOutputGlobal(server, &output);

    struct Client {
        ext_output_image_capture_source_manager_v1 *sources = nullptr;
        ext_image_copy_capture_manager_v1 *copies = nullptr;
        wl_output *output = nullptr;
        bool ready = false;
        bool failed = false;
    } clientState;

    auto *captureRegistry = wl_display_get_registry(client);
    const wl_registry_listener globals = {
            [](void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t) {
                auto &state = *static_cast<Client *>(data);
                if (!strcmp(interface, "ext_output_image_capture_source_manager_v1"))
                    state.sources = static_cast<ext_output_image_capture_source_manager_v1 *>(
                            wl_registry_bind(registry, name,
                                             &ext_output_image_capture_source_manager_v1_interface,
                                             1));
                else if (!strcmp(interface, "ext_image_copy_capture_manager_v1"))
                    state.copies = static_cast<ext_image_copy_capture_manager_v1 *>(
                            wl_registry_bind(registry, name,
                                             &ext_image_copy_capture_manager_v1_interface, 1));
                else if (!strcmp(interface, "wl_output")) {
                    if (state.output)
                        wl_output_destroy(state.output);
                    state.output = static_cast<wl_output *>(
                            wl_registry_bind(registry, name, &wl_output_interface, 2));
                }
            },
            [](void *, wl_registry *, uint32_t) {}};
    wl_registry_add_listener(captureRegistry, &globals, &clientState);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_TRUE(clientState.sources);
    ASSERT_TRUE(clientState.copies);
    drm->setPrepareCallback([&](render::Renderer &renderer, uint64_t frame) {
        renderer.setOutputContext(
                {output.name, {0, 0, 16, 16}, {16, 16}, 1, 0, frame});
        compositor.updateTextures(renderer);
        compositor.frameSubmitted(frame);
    });
    drm->setScanoutPlanner([&](const base::Size &size, size_t count) {
        return capture.hasPending(output.name) ? scene::OverlayPlan()
                                               : scene::proposeOverlays(*scene.root(), size, count);
    });
    drm->setRenderCallback([&](render::Renderer &renderer, uint64_t) {
        scene.render(renderer);
        capture.renderPending(renderer, [&](bool) {
            scene.render(renderer);
        });
    });
    auto presented = drm->imagePresented.connect([&](uint64_t frame, uint64_t time) {
        capture.presented(frame, time);
    });
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1);
    ASSERT_EQ(storage->references, 1);
    const auto directFrames = drm->directScanoutFrames();
    std::printf("Capture transition renderer: %s\n", glGetString(GL_RENDERER));
    std::vector<uint8_t> pattern = solidRgba(base::Size(16, 16), {25, 80, 190, 255});
    const std::array<uint8_t, 4> red {200, 40, 70, 255};
    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 8; ++x)
            std::copy(red.begin(), red.end(), pattern.begin() + (y * 16 + x) * 4);
    compositor.surfaces().front()->node()->setTexture(
            egl->createTexture(base::Size(16, 16), pattern.data()));

    auto *source = ext_output_image_capture_source_manager_v1_create_source(clientState.sources,
                                                                            clientState.output);
    auto *session = ext_image_copy_capture_manager_v1_create_session(clientState.copies, source, 0);
    static const ext_image_copy_capture_session_v1_listener sessionListener = {
            [](void *, auto *, uint32_t, uint32_t) { },
            [](void *, auto *, uint32_t) { },
            [](void *, auto *, wl_array *) { },
            [](void *, auto *, uint32_t, wl_array *) { },
            [](void *, auto *) { },
            [](void *, auto *) { }};
    ext_image_copy_capture_session_v1_add_listener(session, &sessionListener, nullptr);
    auto *frame = ext_image_copy_capture_session_v1_create_frame(session);
    static const ext_image_copy_capture_frame_v1_listener frameListener = {
            [](void *, auto *, uint32_t) {},
            [](void *, auto *, int32_t, int32_t, int32_t, int32_t) {},
            [](void *, auto *, uint32_t, uint32_t, uint32_t) {},
            [](void *data, auto *) {
                static_cast<Client *>(data)->ready = true;
            },
            [](void *data, auto *, uint32_t) {
                static_cast<Client *>(data)->failed = true;
            }};
    ext_image_copy_capture_frame_v1_add_listener(frame, &frameListener, &clientState);
    ScopedFd fd(testsupport::makeMemfd(1024));
    auto *pixels = static_cast<uint32_t *>(
            mmap(nullptr, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd.get(), 0));
    ASSERT_NE(pixels, MAP_FAILED);
    std::fill_n(pixels, 256, 0xdeadbeef);
    auto *pool = wl_shm_create_pool(shm, fd.get(), 1024);
    auto *destination = wl_shm_pool_create_buffer(pool, 0, 16, 16, 64, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    ext_image_copy_capture_frame_v1_attach_buffer(frame, destination);
    ext_image_copy_capture_frame_v1_capture(frame);
    ASSERT_GE(roundtrip(), 0);
    ASSERT_TRUE(capture.hasPending(output.name));
    ASSERT_TRUE(waitEvents(3));
    EXPECT_EQ(drm->directScanoutFrames(), directFrames);
    EXPECT_EQ(storage->references, 1);
    EXPECT_FALSE(clientState.ready);
    flip(2);
    EXPECT_TRUE(clientState.ready);
    EXPECT_FALSE(clientState.failed);
    // The pixels are the client's to read once the frame is ready.
    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            {
                const uint8_t *rgba = pattern.data() + (y * 16 + x) * 4;
                EXPECT_EQ(pixels[y * 16 + x], uint32_t(rgba[3]) << 24 | uint32_t(rgba[0]) << 16
                                                      | uint32_t(rgba[1]) << 8 | rgba[2]);
            }
    EXPECT_FALSE(capture.hasPending(output.name));
    EXPECT_EQ(storage->references, 0);
    ASSERT_TRUE(waitEvents(4));
    flip(3);
    EXPECT_EQ(drm->directScanoutFrames(), directFrames + 1);
    EXPECT_EQ(storage->references, 1);

    presented.reset();
    drm->setScanoutPlanner({});
    drm->setRenderCallback({});
    ext_image_copy_capture_frame_v1_destroy(frame);
    ext_image_copy_capture_session_v1_destroy(session);
    ext_image_capture_source_v1_destroy(source);
    wl_buffer_destroy(destination);
    ext_image_copy_capture_manager_v1_destroy(clientState.copies);
    ext_output_image_capture_source_manager_v1_destroy(clientState.sources);
    wl_output_destroy(clientState.output);
    wl_registry_destroy(captureRegistry);
    ASSERT_GE(roundtrip(), 0);
    wayland::removeOutputGlobal(captureGlobal);
    munmap(pixels, 1024);
}

// --- Session lock: the KMS state behind the secure shield ---------------------

TEST_F(DrmOutputTest, SecureScanoutStopsPromotionAndDisablesTheApplicationPlane)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    ASSERT_EQ(drm->directScanoutFrames(), 1u);
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure() == false);

    // Locking. The client buffer is on the primary plane right now, so the very
    // next commit has to replace it rather than draw the shield somewhere above.
    drm->setSecureScanout(true);
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));

    // Composited, not promoted: the planner is not consulted at all.
    EXPECT_EQ(drm->directScanoutFrames(), 1u);
    const auto &request = device.commits.back().request;
    // The overlay plane the config declares is explicitly cleared, whether or
    // not this output ever programmed it.
    bool found = false;
    EXPECT_EQ(propertyValue(request, 11, 1, &found), 0u);
    EXPECT_TRUE(found) << "the overlay plane was not programmed off";

    // Not secure until the frame is actually on screen.
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
    flip(2, 44);
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure());
}

TEST_F(DrmOutputTest, AFrameDrawnMidLockTransitionIsNotSecure)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    bool settled = false;
    std::vector<double> checked;
    drm->setSecureContentCheck([&](double frameTime) {
        checked.push_back(frameTime);
        return settled;
    });

    // The lock screen is still fading in over the desktop in this frame.
    drm->setSecureScanout(true);
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
    ASSERT_FALSE(checked.empty());
    EXPECT_GE(checked.back(), 0) << "the check needs the frame's own draw time";

    settled = true;
    update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    flip(2, 44);
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure());
}

TEST_F(DrmOutputTest, AFrameInFlightWhenATransitionStartsDoesNotCompleteIt)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    drm->setSecureScanout(true);
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    ASSERT_TRUE(drm->lastPresentedFrameWasSecure());

    // Locked: the panel redraws, and the unlock begins before that frame lands.
    update();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(3));
    drm->withdrawSecurePresentation();
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
    flip(2, 44);
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
}

TEST_F(DrmOutputTest, SecurePresentationIsWithdrawnUntilTheNextFrameLands)
{
    update();
    drm->setSecureScanout(true);
    ASSERT_TRUE(drm->enable());
    ASSERT_TRUE(drm->lastPresentedFrameWasSecure());

    // Unlocking: the policy changed, so what is on screen no longer counts even
    // though the pixels have not moved.
    drm->setSecureScanout(false);
    EXPECT_FALSE(drm->lastPresentedFrameWasSecure());
}

TEST_F(DrmOutputTest, ADarkOutputNeverHoldsTheSecureBarrierOpen)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    drm->setSecureScanout(true);

    // DPMS off. The output produces no frames, so treating it as "not yet
    // secure" would leave a lock waiting forever.
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), false}}, false));
    ASSERT_FALSE(drm->isEnabled());
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure());
}

TEST_F(DrmOutputTest, DpmsWakeWhileLockedComesBackSecureFromItsFirstFrame)
{
    update();
    ASSERT_TRUE(drm->enable());
    flip(0);
    update();
    promote();
    drm->scheduleFrame();
    ASSERT_TRUE(waitEvents(2));
    flip(1, 43);
    ASSERT_EQ(drm->directScanoutFrames(), 1u);

    drm->setSecureScanout(true);
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), false}}, false));
    ASSERT_FALSE(drm->isEnabled());

    const size_t before = device.commits.size();
    ASSERT_TRUE(Output::configure({{drm.get(), drm->mode(), true}}, false));
    ASSERT_TRUE(drm->isEnabled());
    ASSERT_GT(device.commits.size(), before);

    // The modeset that brings the output back is itself a secure frame: it
    // composites and clears the overlay, so not one frame of the old session's
    // direct scanout can come back with the display.
    EXPECT_EQ(drm->directScanoutFrames(), 1u);
    bool found = false;
    EXPECT_EQ(propertyValue(device.commits.back().request, 11, 1, &found), 0u);
    EXPECT_TRUE(found) << "the overlay plane survived the wake";
    EXPECT_TRUE(drm->lastPresentedFrameWasSecure());
}
