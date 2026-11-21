// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/buffer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the gfx::GpuMemoryBuffer / viz::TransferableResource
// production is replaced by an EGLImage import, and the DMA-BUF description is
// kept whole so the same buffer can be scanned out directly.

#ifndef LINGMO_COMPOSITOR_DMABUF_BUFFER_H_
#define LINGMO_COMPOSITOR_DMABUF_BUFFER_H_

#include <memory>
#include <utility>

#include "compositor/buffer.h"
#include "render/dmabuf.h"

namespace Lingmo::Compositor {

// A wl_buffer backed by a DMA-BUF the client allocated.
//
// The compositor samples the client's storage directly instead of copying it,
// so retainsClientStorage() stays true: wl_buffer.release waits until the
// buffer is off every surface, not merely until a frame was drawn.
class DmaBufBuffer : public Buffer, public render::ScanoutBuffer
{
public:
    // Takes ownership of the plane fds in `attributes` in every case. Returns
    // nullptr when `device` cannot import the buffer, which the protocol layer
    // reports as zwp_linux_buffer_params_v1.failed.
    static std::unique_ptr<DmaBufBuffer> create(render::DmaBufDevice* device,
                                                render::DmaBufAttributes attributes);

    ~DmaBufBuffer() override;

    base::Size size() const override { return m_attributes.size; }

    render::PixelFormat format() const override { return m_format; }

    // Nothing is mapped: the contents are read by the GPU straight out of the
    // client's DMA-BUF.
    const uint8_t* pixels() const override { return nullptr; }

    int stride() const override;

    std::shared_ptr<render::Texture> texture() const override { return m_texture; }

    // The imported description, still owning its plane fds. The scanout path
    // builds a DRM framebuffer from these very same fds rather than copying
    // the buffer into compositor-owned storage.
    const render::DmaBufAttributes& attributes() const { return m_attributes; }

    const render::DmaBufAttributes& scanoutAttributes() const override { return m_attributes; }

    render::ScanoutBuffer* scanoutBuffer() override { return this; }

    bool supportsExplicitSync() const override { return true; }

    std::vector<render::SyncFence> implicitFences() const override;

    void onScanoutAcquired() override { Buffer::onScanoutAcquired(); }

    void onScanoutReleased(render::SyncFence fence) override
    {
        Buffer::onScanoutReleased(std::move(fence));
    }

private:
    DmaBufBuffer(render::DmaBufAttributes attributes, std::shared_ptr<render::Texture> texture,
                 render::PixelFormat format);

    render::DmaBufAttributes m_attributes;
    std::shared_ptr<render::Texture> m_texture;
    render::PixelFormat m_format = render::PixelFormat::Rgba8888;
};

} // namespace Lingmo::Compositor

#endif // LINGMO_COMPOSITOR_DMABUF_BUFFER_H_
