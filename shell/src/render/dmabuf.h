// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/gfx/native_pixmap_handle.h
//           chromium/src/gpu/command_buffer/common/capabilities.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gfx::NativePixmapHandle becomes DmaBufAttributes, and
// the slice of gpu::Capabilities the dmabuf protocol needs becomes DmaBufDevice.

#ifndef LINGMO_RENDER_DMABUF_H_
#define LINGMO_RENDER_DMABUF_H_

#include <sys/types.h>

#include <cstdint>
#include <map>
#include <memory>
#include <vector>


#include "core/scoped_fd.h"
#include "render/texture.h"
#include "base/geometry.h"

namespace Lingmo::Render {

// linux-dmabuf allows at most four planes per buffer.
constexpr size_t kMaxDmaBufPlanes = 4;

struct DmaBufPlane {
    ScopedFd fd;
    uint32_t offset = 0;
    uint32_t stride = 0;
};

// One importable DMA-BUF: a DRM fourcc, one modifier shared by every plane, and
// the plane fds themselves. Kept whole rather than reduced to an EGLImage, so
// the same description can go to DRM/KMS for scanout.
struct DmaBufAttributes {
    base::Size size;
    uint32_t format = 0; // DRM fourcc
    uint64_t modifier = 0;
    bool yInvert = false;
    std::vector<DmaBufPlane> planes;
};

// DRM fourcc -> the modifiers the device can import it with. DRM_FORMAT_MOD_INVALID
// stands for "implicit modifier", which every importer accepts.
using DmaBufFormats = std::map<uint32_t, std::vector<uint64_t>>;

// The subset of the renderer's capabilities the dmabuf protocol needs, plus the
// import itself. Implemented by EglRenderer; null in tests and headless runs,
// in which case no linux-dmabuf global is advertised.
class DmaBufDevice
{
public:
    virtual ~DmaBufDevice() = default;

    // Formats and modifiers this device can import, in DRM fourcc terms.
    virtual const DmaBufFormats& dmaBufFormats() const = 0;

    // st_rdev of the DRM node clients should allocate on, or 0 when it could
    // not be determined, which downgrades the advertised protocol version.
    virtual dev_t dmaBufMainDevice() const = 0;

    // Imports `attributes` into a sampleable texture. The attributes, and so
    // the plane fds, must outlive the returned texture. Returns nullptr when
    // the import fails, which the protocol reports as params.failed.
    virtual std::shared_ptr<Texture> importDmaBuf(const DmaBufAttributes& attributes) = 0;
};

} // namespace Lingmo::Render

#endif // LINGMO_RENDER_DMABUF_H_
