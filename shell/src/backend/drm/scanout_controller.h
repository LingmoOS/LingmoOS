// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/
//               hardware_display_plane_manager_atomic.cc (Commit
//                   AddOutFencePtrProperties AddAllPendingCrtcProperties)
//               hardware_display_plane_manager.cc (UpdatePendingCrtcState)
//               hardware_display_plane_atomic.cc (AssignPlaneProps, SetPlaneProps)
//               drm_overlay_validator.cc (TestPageFlip)
//               drm_gpu_util.cc (CreateCTMBlob CreateLutBlob)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: one CRTC, no plane migration between controllers, a
// colour pipeline holding nothing but per-channel gains, which ride the next
// page flip instead of a blocking property commit, and the test buffers
// upstream allocates are replaced by the composited framebuffer already in
// hand, which has exactly the format and modifier the real commit will use.

#ifndef LINGMO_BACKEND_DRM_SCANOUT_CONTROLLER_H_
#define LINGMO_BACKEND_DRM_SCANOUT_CONTROLLER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "backend/drm/drm_device_api.h"
#include "backend/drm/drm_plane.h"
#include "backend/drm/drm_properties.h"
#include "base/geometry.h"
#include "render/color_transform.h"
#include "render/scanout.h"
#include "render/sync_fence.h"
#include "scene/overlay_candidate.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Backend {

// One plane's programmed state for a frame.
struct PlaneContent {
    uint32_t planeId = 0;
    uint32_t framebufferId = 0;
    uint32_t format = 0;
    uint64_t modifier = 0;
    base::Rect displayRect;
    base::Rect sourceRect;
    // Owned: a frame can be committed again after the plan that produced it
    // is gone, and KMS dups the fd on every commit.
    std::shared_ptr<render::SyncFence> acquireFence;
    // Held for as long as KMS may be reading the buffer. Null for the
    // compositor's own framebuffer, which the GBM surface owns.
    std::shared_ptr<render::ScanoutBuffer> buffer;
    scene::SurfaceNode *node = nullptr;

    bool isDirectScanout() const { return buffer != nullptr; }

    int acquireFenceFd() const { return acquireFence ? acquireFence->get() : -1; }
};

// Turns a scene plan into a hardware plane configuration and commits it.
class ScanoutController
{
public:
    struct Config {
        uint32_t crtcId = 0;
        // Index into drmModeRes, which is what a plane's possible_crtcs bits
        // refer to.
        int crtcIndex = -1;
        uint32_t connectorId = 0;
        DrmPropertyMap crtcProperties;
        DrmPropertyMap connectorProperties;
        std::vector<DrmPlane> planes;
        base::Size modeSize;
    };

    ScanoutController(DrmDeviceApi *device, Config config);
    ScanoutController(const ScanoutController &) = delete;
    ScanoutController &operator=(const ScanoutController &) = delete;
    ~ScanoutController();

    // Overlay planes usable on this CRTC. Zero means overlays are never
    // proposed, which is a normal configuration and not a failure.
    size_t maxOverlays() const { return m_overlayPlanes.size(); }

    bool hasPrimaryPlane() const { return m_primaryPlane != nullptr; }

    // Resolves `plan` to hardware planes and validates the result with an
    // atomic TEST_ONLY, dropping the topmost overlay and retesting until it
    // passes - upstream's DrmOverlayValidator::TestPageFlip.
    //
    // `composited` describes the framebuffer composition will produce; it is
    // used for the primary plane unless the plan has direct scanout. Returns
    // the planes to program, primary first, or just `composited` when nothing
    // could be promoted.
    std::vector<PlaneContent> validate(const scene::OverlayPlan &plan,
                                       const PlaneContent &composited, uint32_t modeBlobId);

    // Programs `planes` and commits. A modeset commits blocking and without an
    // event, as several drivers reject ALLOW_MODESET together with a
    // non-blocking flip; otherwise it is a non-blocking page flip with
    // `userData` as the completion token.
    //
    // `releaseFence`, when non-null, receives the CRTC's OUT_FENCE: it signals
    // when this commit reaches the screen, which is exactly when the buffers it
    // replaces stop being read.
    // `secure` builds the request so that every plane on this CRTC except the
    // ones in `planes` is explicitly turned off, rather than only the ones this
    // controller programmed last. A locked screen cannot rely on its own
    // bookkeeping being complete: a plane left over from a modeset, a previous
    // compositor or a frame whose commit failed has to be cleared too.
    bool commit(const std::vector<PlaneContent> &planes, bool modeset, void *userData,
                uint32_t modeBlobId, render::SyncFence *releaseFence, bool secure = false);

    // Turns the CRTC and every plane off, blocking.
    bool disable();

    // A hardware cursor of ours drives the cursor plane through the legacy
    // cursor calls, so a modeset leaves that plane alone.
    void setHardwareCursor(bool owned) { m_hardwareCursor = owned; }

    // Whether the CRTC can tint everything it shows, cursor plane included:
    // through its CTM, or a gamma LUT where it has no CTM.
    bool supportsColorTransform() const;

    // The tint every commit from the next one on carries. Returns whether that
    // changed what the next commit sends; a CRTC that cannot take it, or whose
    // driver once refused it, stays untinted, and DrmOutput blends the tint.
    bool setColorTransform(const render::ColorTransform &transform);

    // The DRM framebuffer id for `buffer`, imported on first use and cached on
    // the buffer itself. Zero when this device cannot scan it out.
    uint32_t framebufferFor(render::ScanoutBuffer &buffer, uint32_t *format, uint64_t *modifier);

    // Diagnostics.
    uint64_t testFailures() const { return m_testFailures; }

    uint64_t importFailures() const { return m_importFailures; }

private:
    friend class DrmOutput;

    // The blobs that program a colour transform: the CTM's, or the gamma LUT's
    // on a CRTC without a CTM. Zero unsets a property, as the identity does.
    struct ColorPipeline {
        render::ColorTransform transform;
        uint32_t ctmBlob = 0;
        uint32_t gammaBlob = 0;
    };

    AtomicRequest buildRequest(const std::vector<PlaneContent> &planes, bool modeset,
                               uint32_t modeBlobId, bool off, bool secure = false) const;
    void destroyBlobs(const ColorPipeline &color);
    // Every plane this CRTC can drive, primary and overlays alike.
    std::vector<const DrmPlane *> allPlanes() const;
    void addPlaneProperties(AtomicRequest &request, const DrmPlane &plane,
                            const PlaneContent &content, bool off) const;
    const DrmPlane *planeById(uint32_t id) const;

    DrmDeviceApi *m_device = nullptr;
    Config m_config;
    const DrmPlane *m_primaryPlane = nullptr;
    std::vector<const DrmPlane *> m_overlayPlanes;
    std::vector<const DrmPlane *> m_cursorPlanes;
    bool m_hardwareCursor = false;
    // Planes programmed by the last commit, so ones that fall out of use are
    // explicitly disabled rather than left showing a stale buffer.
    std::vector<uint32_t> m_activePlanes;
    ColorPipeline m_color;
    // The transform has not reached the CRTC yet, so the next commit sends it.
    bool m_colorPending = false;
    bool m_colorRejected = false;
    uint64_t m_testFailures = 0;
    uint64_t m_importFailures = 0;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_SCANOUT_CONTROLLER_H_
