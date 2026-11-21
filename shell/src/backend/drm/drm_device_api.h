// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/common/drm_wrapper.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: only the ioctls the compositor actually issues, and the
// atomic request is a plain property list rather than a drmModeAtomicReq, so
// the planner and the commit path can be exercised without a DRM device.

#ifndef LINGMO_BACKEND_DRM_DRM_DEVICE_API_H_
#define LINGMO_BACKEND_DRM_DRM_DEVICE_API_H_

#include "base/geometry.h"

#include <cstdint>
#include <vector>

#include "render/dmabuf.h"

struct gbm_device;

namespace Lingmo::Backend {

// One property assignment in an atomic commit.
struct AtomicProperty {
    uint32_t objectId = 0;
    uint32_t propertyId = 0;
    uint64_t value = 0;
};

// An atomic modeset or page flip, built by the caller and translated into a
// drmModeAtomicReq by the real device.
struct AtomicRequest {
    std::vector<AtomicProperty> properties;

    void add(uint32_t objectId, uint32_t propertyId, uint64_t value)
    {
        // A property the driver does not expose has id 0 and is skipped, which
        // is how optional properties like IN_FENCE_FD stay optional.
        if (propertyId)
            properties.push_back(AtomicProperty {objectId, propertyId, value});
    }
};

// A framebuffer to register with the kernel.
struct FramebufferParams {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t format = 0;
    uint64_t modifier = 0;
    size_t planeCount = 0;
    uint32_t handles[4] = {};
    uint32_t strides[4] = {};
    uint32_t offsets[4] = {};
};

// A CPU-mappable buffer, used for the hardware cursor.
struct DumbBufferInfo {
    uint32_t handle = 0;
    uint32_t stride = 0;
    uint64_t size = 0;
};

// Everything the display path asks of the DRM device.
class DrmDeviceApi
{
public:
    virtual ~DrmDeviceApi() = default;

    virtual int fd() const = 0;

    // The renderer's GBM device when it shares this file, else null. The file's
    // GEM handles are Mesa's then, so client buffers are imported through GBM.
    virtual gbm_device* gbmDevice() const = 0;

    // --- Framebuffers. ---

    virtual bool addFramebuffer(const FramebufferParams& params, uint32_t* framebufferId) = 0;
    // A plane still showing the framebuffer keeps showing it until a commit
    // replaces it (CLOSEFB), so a session's last frame outlives the session.
    virtual bool removeFramebuffer(uint32_t framebufferId) = 0;

    // The framebuffer `crtcId` scans out now, whoever added it, as DMA-BUF
    // planes. False when the CRTC shows nothing, or when this file is not the
    // DRM master, which is the only one the kernel hands the buffers to.
    virtual bool exportCrtcFramebuffer(uint32_t crtcId, render::DmaBufAttributes* attributes) = 0;

    // Imports a DMA-BUF into a GEM handle on this device. The handle must be
    // closed once no framebuffer references it any more.
    virtual bool primeFdToHandle(int fd, uint32_t* handle) = 0;
    virtual bool closeHandle(uint32_t handle) = 0;

    virtual bool createPropertyBlob(const void* data, size_t size, uint32_t* id) = 0;
    virtual void destroyPropertyBlob(uint32_t id) = 0;

    // --- Atomic commits. ---

    // `flags` are DRM_MODE_ATOMIC_*; `userData` is the page-flip event token
    // and must be null for a TEST_ONLY commit.
    virtual bool atomicCommit(const AtomicRequest& request, uint32_t flags, void* userData) = 0;

    // --- Cursor plane. ---
    //
    // The legacy cursor ioctls, as Ozone uses them: KMS drives the cursor plane
    // itself, so moving the cursor costs no atomic commit and no repaint.

    virtual bool setCursor(uint32_t crtcId, uint32_t handle, const base::Size& size,
                           const base::Point& hotspot) = 0;
    virtual bool moveCursor(uint32_t crtcId, const base::Point& position) = 0;

    // The largest cursor the hardware accepts, or an empty size when it has no
    // cursor plane at all.
    virtual base::Size maxCursorSize() const = 0;

    virtual bool createDumbBuffer(const base::Size& size, DumbBufferInfo* info) = 0;
    virtual bool destroyDumbBuffer(uint32_t handle) = 0;
    // Returns nullptr on failure. `size` is DumbBufferInfo::size.
    virtual void* mapDumbBuffer(uint32_t handle, uint64_t size) = 0;
    virtual void unmapDumbBuffer(void* data, uint64_t size) = 0;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_DEVICE_API_H_
