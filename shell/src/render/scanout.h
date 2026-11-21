// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/gbm_pixmap.h
//           (GbmPixmap::framebuffer(), the framebuffer cached on the buffer)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: instead of a GbmPixmap holding one DrmFramebuffer, the
// import slot is keyed by device and opaque, so nothing above the backend has
// to know that DRM exists.

#ifndef LINGMO_RENDER_SCANOUT_H_
#define LINGMO_RENDER_SCANOUT_H_

#include <memory>
#include <utility>
#include <vector>

#include "render/dmabuf.h"
#include "render/sync_fence.h"

namespace Lingmo::Render {

// Buffer storage a display controller may be able to scan out directly.
//
// Implemented by DmaBufBuffer. The same buffer is also what the renderer
// samples, so importing it for scanout must not disturb the EGLImage import or
// the plane fds either import holds.
class ScanoutBuffer
{
public:
    // A device-side import of this buffer, normally a DRM framebuffer. Cached
    // on the buffer so a client buffer is imported once rather than once per
    // frame, and torn down with the buffer that describes it.
    class Import
    {
    public:
        virtual ~Import() = default;
    };

    virtual ~ScanoutBuffer() = default;

    // The DMA-BUF description, plane fds included.
    virtual const DmaBufAttributes& scanoutAttributes() const = 0;

    // The import `device` previously cached here, or nullptr. `device` is any
    // stable pointer identifying the importer.
    Import* scanoutImport(const void* device) const
    {
        for (const auto& entry : m_imports) {
            if (entry.first == device)
                return entry.second.get();
        }
        return nullptr;
    }

    // Caches `import` for `device`, replacing any previous one. A null import
    // records that the device could not import this buffer, which stops the
    // scanout path retrying it every frame.
    Import* setScanoutImport(const void* device, std::unique_ptr<Import> import)
    {
        Import* raw = import.get();
        for (auto& entry : m_imports) {
            if (entry.first == device) {
                entry.second = std::move(import);
                return raw;
            }
        }
        m_imports.emplace_back(device, std::move(import));
        return raw;
    }

    // Whether `device` has already tried and failed to import this buffer.
    bool scanoutImportFailed(const void* device) const
    {
        for (const auto& entry : m_imports) {
            if (entry.first == device)
                return entry.second == nullptr;
        }
        return false;
    }

    // The display controller took a reference for KMS scanout, and gave it up
    // again once the buffer had left every plane. Releasing the client's
    // storage waits for the matching release, which is what stops a buffer
    // still on screen from being handed back to the client.
    virtual void onScanoutAcquired() { }
    virtual void onScanoutReleased(SyncFence fence) { (void)fence; }

private:
    std::vector<std::pair<const void*, std::unique_ptr<Import>>> m_imports;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_SCANOUT_H_
