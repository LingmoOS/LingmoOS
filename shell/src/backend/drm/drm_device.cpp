// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/common/drm_wrapper.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "backend/drm/drm_device.h"

#include <drm_fourcc.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <algorithm>
#include <cerrno>
#include <cstring>

#include "core/logging.h"

namespace Lingmo::Backend {

namespace {

// Ozone caps the cursor at 256x256 regardless of what the driver reports: a
// larger cursor buys nothing and costs a much bigger dumb buffer.
constexpr uint64_t kMaxCursorSize = 256;

} // namespace

DrmDevice::DrmDevice(int fd, gbm_device *gbm)
    : m_fd(fd)
    , m_gbm(gbm)
{
    uint64_t width = 0;
    uint64_t height = 0;
    if (drmGetCap(m_fd, DRM_CAP_CURSOR_WIDTH, &width) != 0)
        width = 64;
    if (drmGetCap(m_fd, DRM_CAP_CURSOR_HEIGHT, &height) != 0)
        height = 64;
    m_maxCursorSize = base::Size(static_cast<int>(std::min(width, kMaxCursorSize)),
                            static_cast<int>(std::min(height, kMaxCursorSize)));
}

DrmDevice::~DrmDevice() = default;

bool DrmDevice::addFramebuffer(const FramebufferParams& params, uint32_t* framebufferId)
{
    uint64_t modifiers[4] = {};
    uint32_t flags = 0;
    // AddFB2 only looks at the modifiers when DRM_MODE_FB_MODIFIERS is set, and
    // an implicit modifier must be left to the driver rather than declared.
    if (params.modifier != DRM_FORMAT_MOD_INVALID) {
        for (size_t i = 0; i < params.planeCount && i < 4; ++i)
            modifiers[i] = params.modifier;
        flags = DRM_MODE_FB_MODIFIERS;
    }

    const int result = drmModeAddFB2WithModifiers(
        m_fd, params.width, params.height, params.format,
        const_cast<uint32_t*>(params.handles), const_cast<uint32_t*>(params.strides),
        const_cast<uint32_t*>(params.offsets), modifiers, framebufferId, flags);
    if (result == 0)
        return true;

    // Drivers without modifier support reject the flag outright; the same
    // buffer may still register without it.
    if (flags == 0
        || drmModeAddFB2(m_fd, params.width, params.height, params.format,
                         const_cast<uint32_t*>(params.handles),
                         const_cast<uint32_t*>(params.strides),
                         const_cast<uint32_t*>(params.offsets), framebufferId, 0)
            != 0) {
        return false;
    }
    return true;
}

bool DrmDevice::removeFramebuffer(uint32_t framebufferId)
{
    // RMFB would turn off a plane still showing the framebuffer; CLOSEFB (Linux
    // 6.8) leaves it there for the next commit or the next DRM master to replace.
    return framebufferId == 0 || drmModeCloseFB(m_fd, framebufferId) == 0
            || drmModeRmFB(m_fd, framebufferId) == 0;
}

bool DrmDevice::exportCrtcFramebuffer(uint32_t crtcId, render::DmaBufAttributes* attributes)
{
    drmModeCrtc* crtc = drmModeGetCrtc(m_fd, crtcId);
    const uint32_t framebufferId = crtc ? crtc->buffer_id : 0;
    drmModeFreeCrtc(crtc);
    drmModeFB2* framebuffer = framebufferId ? drmModeGetFB2(m_fd, framebufferId) : nullptr;
    if (!framebuffer)
        return false;

    attributes->size = base::Size(int(framebuffer->width), int(framebuffer->height));
    attributes->format = framebuffer->pixel_format;
    attributes->modifier = framebuffer->flags & DRM_MODE_FB_MODIFIERS ? framebuffer->modifier
                                                                       : DRM_FORMAT_MOD_INVALID;
    attributes->planes.clear();
    bool exported = framebuffer->handles[0] != 0;
    for (size_t i = 0; exported && i < render::kMaxDmaBufPlanes && framebuffer->handles[i]; ++i) {
        int fd = -1;
        exported = drmPrimeHandleToFD(m_fd, framebuffer->handles[i], DRM_CLOEXEC, &fd) == 0;
        if (exported)
            attributes->planes.push_back(
                    {ScopedFd(fd), framebuffer->offsets[i], framebuffer->pitches[i]});
    }
    // GETFB2 created these handles for us; planes may share one, closed once.
    for (size_t i = 0; i < render::kMaxDmaBufPlanes; ++i) {
        const uint32_t handle = framebuffer->handles[i];
        if (handle && std::find(framebuffer->handles, framebuffer->handles + i, handle)
                    == framebuffer->handles + i)
            drmCloseBufferHandle(m_fd, handle);
    }
    drmModeFreeFB2(framebuffer);
    return exported;
}

bool DrmDevice::primeFdToHandle(int fd, uint32_t* handle)
{
    return drmPrimeFDToHandle(m_fd, fd, handle) == 0;
}

bool DrmDevice::closeHandle(uint32_t handle)
{
    return drmCloseBufferHandle(m_fd, handle) == 0;
}

bool DrmDevice::createPropertyBlob(const void* data, size_t size, uint32_t* id)
{
    return drmModeCreatePropertyBlob(m_fd, data, size, id) == 0;
}

void DrmDevice::destroyPropertyBlob(uint32_t id)
{
    drmModeDestroyPropertyBlob(m_fd, id);
}

bool DrmDevice::atomicCommit(const AtomicRequest& request, uint32_t flags, void* userData)
{
    drmModeAtomicReq* atomic = drmModeAtomicAlloc();
    if (!atomic)
        return false;

    bool built = true;
    for (const AtomicProperty& property : request.properties) {
        if (drmModeAtomicAddProperty(atomic, property.objectId, property.propertyId,
                                     property.value)
            < 0) {
            built = false;
            break;
        }
    }

    const int result = built ? drmModeAtomicCommit(m_fd, atomic, flags, userData) : -EINVAL;
    drmModeAtomicFree(atomic);
    return result == 0;
}

bool DrmDevice::setCursor(uint32_t crtcId, uint32_t handle, const base::Size& size,
                          const base::Point& hotspot)
{
    if (m_hasCursor2) {
        if (drmModeSetCursor2(m_fd, crtcId, handle, size.width(), size.height(), hotspot.x(),
                              hotspot.y())
            == 0) {
            return true;
        }
        if (errno != EINVAL && errno != ENOSYS)
            return false;
        m_hasCursor2 = false;
    }
    return drmModeSetCursor(m_fd, crtcId, handle, size.width(), size.height()) == 0;
}

bool DrmDevice::moveCursor(uint32_t crtcId, const base::Point& position)
{
    return drmModeMoveCursor(m_fd, crtcId, position.x(), position.y()) == 0;
}

bool DrmDevice::createDumbBuffer(const base::Size& size, DumbBufferInfo* info)
{
    drm_mode_create_dumb request = {};
    request.width = static_cast<uint32_t>(size.width());
    request.height = static_cast<uint32_t>(size.height());
    request.bpp = 32;
    if (drmIoctl(m_fd, DRM_IOCTL_MODE_CREATE_DUMB, &request) != 0)
        return false;

    info->handle = request.handle;
    info->stride = request.pitch;
    info->size = request.size;
    return true;
}

bool DrmDevice::destroyDumbBuffer(uint32_t handle)
{
    drm_mode_destroy_dumb request = {};
    request.handle = handle;
    return drmIoctl(m_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &request) == 0;
}

void* DrmDevice::mapDumbBuffer(uint32_t handle, uint64_t size)
{
    drm_mode_map_dumb request = {};
    request.handle = handle;
    if (drmIoctl(m_fd, DRM_IOCTL_MODE_MAP_DUMB, &request) != 0)
        return nullptr;

    void* data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd,
                      static_cast<off_t>(request.offset));
    return data == MAP_FAILED ? nullptr : data;
}

void DrmDevice::unmapDumbBuffer(void* data, uint64_t size)
{
    if (data)
        munmap(data, size);
}

} // namespace Lingmo::Backend
