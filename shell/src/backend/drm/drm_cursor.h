// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_controller.cc
//           (SetCursor, MoveCursor, NextCursorBuffer, DrawCursor)
//           chromium/src/ui/ozone/platform/drm/gpu/drm_dumb_buffer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: SkBitmap becomes raw premultiplied pixels and the
// dynamic Intel cursor-size table is omitted; one buffer size is allocated from
// the device's reported maximum.

#ifndef LINGMO_BACKEND_DRM_DRM_CURSOR_H_
#define LINGMO_BACKEND_DRM_DRM_CURSOR_H_

#include <string>

#include "base/geometry.h"

#include <array>
#include <cstdint>
#include <memory>

#include "backend/drm/drm_device_api.h"
#include "render/color_transform.h"

namespace Lingmo::Backend {

// The KMS cursor plane for one CRTC.
//
// Moving the cursor here costs one ioctl and no repaint, which is the whole
// point: with the cursor composited, every pointer motion is a full frame.
class DrmCursor
{
public:
    DrmCursor(const DrmCursor&) = delete;
    DrmCursor& operator=(const DrmCursor&) = delete;
    ~DrmCursor();

    // Returns nullptr when the device has no cursor plane or its buffers
    // cannot be allocated, in which case the caller composites the cursor.
    static std::unique_ptr<DrmCursor> create(DrmDeviceApi* device, uint32_t crtcId);

    // Uploads `pixels`, premultiplied ARGB32 (blue, green, red, alpha in memory).
    // Returns false when the image is larger than the plane can show; the
    // caller must then fall back to compositing it.
    bool setImage(const uint8_t* pixels, const base::Size& size, int stride,
                  const base::Point& hotspot);

    // Scales the colour channels of the image the plane shows, for a CRTC that
    // cannot tint its planes itself; a visible cursor is uploaded again.
    void setTint(const render::ColorTransform& tint);

    // Takes the cursor off the plane. Idempotent.
    void hide();

    // `position` is where the hotspot should land, in output pixels.
    void move(const base::Point& position);

    bool isVisible() const { return m_visible; }

private:
    struct Buffer {
        DumbBufferInfo info;
        void* data = nullptr;
    };

    DrmCursor(DrmDeviceApi* device, uint32_t crtcId, const base::Size& size,
              std::array<Buffer, 2> buffers);
    bool upload();

    DrmDeviceApi* m_device = nullptr;
    uint32_t m_crtcId = 0;
    base::Size m_size;
    // Two buffers, so an image change never writes into the one the display is
    // currently scanning out.
    std::array<Buffer, 2> m_buffers;
    size_t m_front = 0;
    // The last image, kept to be tinted again.
    std::string m_image;
    base::Size m_imageSize;
    int m_imageStride = 0;
    render::ColorTransform m_tint;
    base::Point m_hotspot;
    base::Point m_position;
    // Until the first move the plane stays where the previous DRM master left it.
    bool m_positioned = false;
    bool m_visible = false;
    // Set once the driver has rejected drmModeSetCursor; retrying every frame
    // would be pointless.
    bool m_failed = false;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_CURSOR_H_
