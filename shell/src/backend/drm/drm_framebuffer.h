// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_framebuffer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: no opaque-format alias framebuffer and no modeset
// sequence tracking, and the client DMA-BUF import upstream gets from
// GbmPixmap is done here.

#ifndef LINGMO_BACKEND_DRM_DRM_FRAMEBUFFER_H_
#define LINGMO_BACKEND_DRM_DRM_FRAMEBUFFER_H_

#include "base/geometry.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "render/scanout.h"

struct gbm_bo;

namespace Lingmo::Backend {

class DrmDeviceApi;

// A DRM framebuffer, either wrapping a GBM buffer object the compositor
// rendered into or importing a client DMA-BUF for direct scanout.
//
// For a GBM bo the framebuffer is stored as user data on the bo, so the same bo
// coming back out of the gbm_surface queue reuses its framebuffer and both die
// together, whether the bo's own device scans it out, another device imports
// it or another device shows a copy of it. For a client DMA-BUF it is cached
// on the ScanoutBuffer instead, which gives it the same one-import-per-buffer
// lifetime.
class DrmFramebuffer : public render::ScanoutBuffer::Import
{
public:
    DrmFramebuffer(const DrmFramebuffer &) = delete;
    DrmFramebuffer &operator=(const DrmFramebuffer &) = delete;
    ~DrmFramebuffer() override;

    // Returns the framebuffer for `bo`, creating it on first use. Ownership
    // stays with the bo. Returns nullptr if the framebuffer cannot be created.
    static DrmFramebuffer *forBuffer(DrmDeviceApi *device, gbm_bo *bo);

    // The same for a `bo` another GPU rendered, imported into `device`
    // through DMA-BUF on first use, so a surface's two or three buffers are
    // each imported once rather than on every frame.
    static DrmFramebuffer *importForBuffer(DrmDeviceApi *device, gbm_bo *bo);

    // A dumb framebuffer on `device` kept on `bo`, and filled with `pixels`,
    // the bo's contents, each time: a device that cannot import the bo shows
    // a copy of it. The bo comes back only once its last frame retired, so the
    // copy is never rewritten while on screen.
    static DrmFramebuffer *copyForBuffer(DrmDeviceApi *device, gbm_bo *bo,
                                         const uint8_t *pixels, uint32_t stride);

    // Imports the client's DMA-BUF planes into a framebuffer, preserving the
    // fourcc and the modifier the client allocated with. Returns nullptr when
    // the device cannot scan the buffer out, which is a fallback to
    // composition and never an error.
    static std::unique_ptr<DrmFramebuffer> fromDmaBuf(DrmDeviceApi *device,
                                                      const render::DmaBufAttributes &attributes);

    static std::unique_ptr<DrmFramebuffer> fromPixels(DrmDeviceApi *device, const base::Size &size,
                                                      const uint8_t *pixels, uint32_t stride);

    uint32_t id() const { return m_id; }

    base::Size size() const { return m_size; }

    uint32_t format() const { return m_format; }

    uint64_t modifier() const { return m_modifier; }

private:
    DrmFramebuffer(DrmDeviceApi *device, uint32_t id, const base::Size &size, uint32_t format,
                   uint64_t modifier, std::vector<uint32_t> ownedHandles);

    static void destroyCallback(gbm_bo *bo, void *data);
    // Hands `framebuffer` to `bo`, retiring one of another kind it held.
    static DrmFramebuffer *keep(gbm_bo *bo, std::unique_ptr<DrmFramebuffer> framebuffer);
    bool fill(const uint8_t *pixels, uint32_t stride);

    DrmDeviceApi *m_device = nullptr;
    uint32_t m_id = 0;
    base::Size m_size;
    uint32_t m_format = 0;
    uint64_t m_modifier = 0;
    // GEM handles this framebuffer imported and must close. Empty for a GBM bo,
    // whose handles belong to GBM.
    std::vector<uint32_t> m_ownedHandles;
    // A client DMA-BUF imported through the device's GBM device, which owns
    // its handles and counts them with the renderer's import of the buffer.
    gbm_bo *m_importedBo = nullptr;
    uint32_t m_dumbHandle = 0;
    uint32_t m_dumbStride = 0;
    uint64_t m_dumbSize = 0;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_FRAMEBUFFER_H_
