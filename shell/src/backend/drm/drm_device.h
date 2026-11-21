// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/common/drm_wrapper.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: wraps the compositor-owned DRM descriptor behind its
// small injectable ioctl interface.

#ifndef LINGMO_BACKEND_DRM_DRM_DEVICE_H_
#define LINGMO_BACKEND_DRM_DRM_DEVICE_H_

#include "backend/drm/drm_device_api.h"

namespace Lingmo::Backend {

// DrmDeviceApi over a real DRM file descriptor. The fd stays owned by the
// session and the GBM device by DrmBackend, which opens both.
class DrmDevice final : public DrmDeviceApi
{
public:
    DrmDevice(int fd, gbm_device *gbm);
    ~DrmDevice() override;

    int fd() const override { return m_fd; }

    gbm_device *gbmDevice() const override { return m_gbm; }

    bool addFramebuffer(const FramebufferParams &params, uint32_t *framebufferId) override;
    bool removeFramebuffer(uint32_t framebufferId) override;
    bool exportCrtcFramebuffer(uint32_t crtcId, render::DmaBufAttributes *attributes) override;

    bool primeFdToHandle(int fd, uint32_t *handle) override;
    bool closeHandle(uint32_t handle) override;

    bool createPropertyBlob(const void *data, size_t size, uint32_t *id) override;
    void destroyPropertyBlob(uint32_t id) override;

    bool atomicCommit(const AtomicRequest &request, uint32_t flags, void *userData) override;

    bool setCursor(uint32_t crtcId, uint32_t handle, const base::Size &size,
                   const base::Point &hotspot) override;
    bool moveCursor(uint32_t crtcId, const base::Point &position) override;

    base::Size maxCursorSize() const override { return m_maxCursorSize; }

    bool createDumbBuffer(const base::Size &size, DumbBufferInfo *info) override;
    bool destroyDumbBuffer(uint32_t handle) override;
    void *mapDumbBuffer(uint32_t handle, uint64_t size) override;
    void unmapDumbBuffer(void *data, uint64_t size) override;

    void invalidate() { m_fd = -1; }

private:
    int m_fd = -1;
    gbm_device *m_gbm = nullptr;
    base::Size m_maxCursorSize;
    // drmModeSetCursor2 carries the hotspot; drivers without it need the
    // caller to fold the hotspot into the cursor position instead.
    bool m_hasCursor2 = true;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_DEVICE_H_
