// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/drm_framebuffer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "backend/drm/drm_framebuffer.h"

#include <drm_fourcc.h>
#include <gbm.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <utility>

#include "backend/drm/drm_device_api.h"
#include "backend/drm/drm_multi_device_planner.h"
#include "core/logging.h"

namespace Lingmo::Backend {

namespace {

// drmPrimeFDToHandle hands out the same GEM handle for the same underlying
// buffer, so closing one framebuffer's handle would pull it out from under any
// other framebuffer importing the same buffer. The handles are reference
// counted per device to keep that from happening. A device whose file the
// renderer shares never gets here: Mesa holds the same handles there.
using HandleCounts = std::map<std::pair<const DrmDeviceApi *, uint32_t>, int>;

HandleCounts &handleCounts()
{
    static HandleCounts counts;
    return counts;
}

void retainHandle(const DrmDeviceApi *device, uint32_t handle)
{
    ++handleCounts()[{device, handle}];
}

void releaseHandle(DrmDeviceApi *device, uint32_t handle)
{
    auto &counts = handleCounts();
    const auto entry = counts.find({device, handle});
    if (entry == counts.end())
        return;
    if (--entry->second > 0)
        return;
    counts.erase(entry);
    device->closeHandle(handle);
}

gbm_bo *importThroughGbm(gbm_device *gbm, const render::DmaBufAttributes &attributes)
{
    gbm_import_fd_modifier_data data = {};
    data.width = static_cast<uint32_t>(attributes.size.width());
    data.height = static_cast<uint32_t>(attributes.size.height());
    data.format = attributes.format;
    data.num_fds = static_cast<uint32_t>(attributes.planes.size());
    data.modifier = attributes.modifier;
    for (size_t i = 0; i < attributes.planes.size(); ++i) {
        data.fds[i] = attributes.planes[i].fd.get();
        data.strides[i] = static_cast<int>(attributes.planes[i].stride);
        data.offsets[i] = static_cast<int>(attributes.planes[i].offset);
    }
    return gbm_bo_import(gbm, GBM_BO_IMPORT_FD_MODIFIER, &data, GBM_BO_USE_SCANOUT);
}

} // namespace

DrmFramebuffer::DrmFramebuffer(DrmDeviceApi *device, uint32_t id, const base::Size &size,
                               uint32_t format, uint64_t modifier,
                               std::vector<uint32_t> ownedHandles)
    : m_device(device)
    , m_id(id)
    , m_size(size)
    , m_format(format)
    , m_modifier(modifier)
    , m_ownedHandles(std::move(ownedHandles))
{
}

DrmFramebuffer::~DrmFramebuffer()
{
    if (m_id)
        m_device->removeFramebuffer(m_id);
    // The framebuffer has to go first: a handle closed while KMS still knows a
    // framebuffer built from it is a driver-visible use-after-free.
    for (uint32_t handle : m_ownedHandles)
        releaseHandle(m_device, handle);
    if (m_importedBo)
        gbm_bo_destroy(m_importedBo);
    if (m_dumbHandle)
        m_device->destroyDumbBuffer(m_dumbHandle);
}

void DrmFramebuffer::destroyCallback(gbm_bo *, void *data)
{
    delete static_cast<DrmFramebuffer *>(data);
}

DrmFramebuffer *DrmFramebuffer::forBuffer(DrmDeviceApi *device, gbm_bo *bo)
{
    if (!device || !bo)
        return nullptr;

    if (auto *existing = static_cast<DrmFramebuffer *>(gbm_bo_get_user_data(bo)))
        return existing;

    FramebufferParams params;
    params.width = gbm_bo_get_width(bo);
    params.height = gbm_bo_get_height(bo);
    params.format = gbm_bo_get_format(bo);
    params.modifier = gbm_bo_get_modifier(bo);
    params.planeCount = static_cast<size_t>(std::min(gbm_bo_get_plane_count(bo), 4));
    for (size_t i = 0; i < params.planeCount; ++i) {
        params.handles[i] = gbm_bo_get_handle_for_plane(bo, static_cast<int>(i)).u32;
        params.strides[i] = gbm_bo_get_stride_for_plane(bo, static_cast<int>(i));
        params.offsets[i] = gbm_bo_get_offset(bo, static_cast<int>(i));
    }

    uint32_t id = 0;
    if (!device->addFramebuffer(params, &id)) {
        LM_CRITICAL(lingmoDrm).printf("Failed to register a %ux%u GBM buffer as a framebuffer", params.width,
                                  params.height);
        return nullptr;
    }

    auto *framebuffer = new DrmFramebuffer(device, id, base::Size(params.width, params.height),
                                           params.format, params.modifier, {});
    gbm_bo_set_user_data(bo, framebuffer, &DrmFramebuffer::destroyCallback);
    return framebuffer;
}

DrmFramebuffer *DrmFramebuffer::keep(gbm_bo *bo, std::unique_ptr<DrmFramebuffer> framebuffer)
{
    // The bo is only handed out again after its last frame retired, so what it
    // held is off screen.
    delete static_cast<DrmFramebuffer *>(gbm_bo_get_user_data(bo));
    gbm_bo_set_user_data(bo, framebuffer.get(), &DrmFramebuffer::destroyCallback);
    return framebuffer.release();
}

DrmFramebuffer *DrmFramebuffer::importForBuffer(DrmDeviceApi *device, gbm_bo *bo)
{
    if (!device || !bo)
        return nullptr;
    if (auto *existing = static_cast<DrmFramebuffer *>(gbm_bo_get_user_data(bo));
        existing && existing->m_device == device && !existing->m_dumbHandle)
        return existing;

    render::DmaBufAttributes attributes;
    attributes.size = base::Size(gbm_bo_get_width(bo), gbm_bo_get_height(bo));
    attributes.format = gbm_bo_get_format(bo);
    attributes.modifier = gbm_bo_get_modifier(bo);
    for (int plane = 0; plane < gbm_bo_get_plane_count(bo); ++plane)
        attributes.planes.push_back({ScopedFd(gbm_bo_get_fd_for_plane(bo, plane)),
                                     gbm_bo_get_offset(bo, plane),
                                     gbm_bo_get_stride_for_plane(bo, plane)});
    std::unique_ptr<DrmFramebuffer> imported = fromDmaBuf(device, attributes);
    return imported ? keep(bo, std::move(imported)) : nullptr;
}

DrmFramebuffer *DrmFramebuffer::copyForBuffer(DrmDeviceApi *device, gbm_bo *bo,
                                              const uint8_t *pixels, uint32_t stride)
{
    if (!device || !bo)
        return nullptr;
    const base::Size size(gbm_bo_get_width(bo), gbm_bo_get_height(bo));
    auto *existing = static_cast<DrmFramebuffer *>(gbm_bo_get_user_data(bo));
    if (existing && existing->m_device == device && existing->m_dumbHandle
        && existing->m_size == size)
        return existing->fill(pixels, stride) ? existing : nullptr;
    std::unique_ptr<DrmFramebuffer> copy = fromPixels(device, size, pixels, stride);
    return copy ? keep(bo, std::move(copy)) : nullptr;
}

bool DrmFramebuffer::fill(const uint8_t *pixels, uint32_t stride)
{
    if (!pixels || stride < uint64_t(m_size.width()) * 4)
        return false;
    void *mapped = m_device->mapDumbBuffer(m_dumbHandle, m_dumbSize);
    if (!mapped)
        return false;
    for (int row = 0; row < m_size.height(); ++row)
        std::memcpy(static_cast<uint8_t *>(mapped) + size_t(row) * m_dumbStride,
                    pixels + size_t(row) * stride, size_t(m_size.width()) * 4);
    m_device->unmapDumbBuffer(mapped, m_dumbSize);
    return true;
}

std::unique_ptr<DrmFramebuffer> DrmFramebuffer::fromPixels(DrmDeviceApi *device,
                                                           const base::Size &size,
                                                           const uint8_t *pixels, uint32_t stride)
{
    if (!device || !pixels || size.isEmpty()
        || int64_t(size.width()) * size.height() > kMaximumCpuCopyPixels
        || stride < uint64_t(size.width()) * 4)
        return nullptr;
    DumbBufferInfo dumb;
    if (!device->createDumbBuffer(size, &dumb))
        return nullptr;
    if (dumb.stride < uint64_t(size.width()) * 4
        || dumb.size < uint64_t(dumb.stride) * size.height()) {
        device->destroyDumbBuffer(dumb.handle);
        return nullptr;
    }
    FramebufferParams params;
    params.width = size.width();
    params.height = size.height();
    params.format = DRM_FORMAT_XRGB8888;
    params.modifier = DRM_FORMAT_MOD_LINEAR;
    params.planeCount = 1;
    params.handles[0] = dumb.handle;
    params.strides[0] = dumb.stride;
    uint32_t id = 0;
    if (!device->addFramebuffer(params, &id)) {
        device->destroyDumbBuffer(dumb.handle);
        return nullptr;
    }
    auto result = std::unique_ptr<DrmFramebuffer>(
            new DrmFramebuffer(device, id, size, params.format, params.modifier, {}));
    result->m_dumbHandle = dumb.handle;
    result->m_dumbStride = dumb.stride;
    result->m_dumbSize = dumb.size;
    return result->fill(pixels, stride) ? std::move(result) : nullptr;
}

std::unique_ptr<DrmFramebuffer>
DrmFramebuffer::fromDmaBuf(DrmDeviceApi *device, const render::DmaBufAttributes &attributes)
{
    if (!device || attributes.planes.empty() || attributes.planes.size() > 4
        || attributes.size.isEmpty()) {
        return nullptr;
    }

    FramebufferParams params;
    params.width = static_cast<uint32_t>(attributes.size.width());
    params.height = static_cast<uint32_t>(attributes.size.height());
    params.format = attributes.format;
    params.modifier = attributes.modifier;
    params.planeCount = attributes.planes.size();

    // As Ozone's GbmPixmap: the renderer imported the buffer on this file too
    // and got the same GEM handles, which only Mesa may close.
    if (gbm_device *gbm = device->gbmDevice()) {
        gbm_bo *bo = importThroughGbm(gbm, attributes);
        if (!bo)
            return nullptr;
        for (size_t i = 0; i < params.planeCount; ++i) {
            const gbm_bo_handle handle = gbm_bo_get_handle_for_plane(bo, static_cast<int>(i));
            if (handle.s32 <= 0) {
                gbm_bo_destroy(bo);
                return nullptr;
            }
            params.handles[i] = handle.u32;
            params.strides[i] = attributes.planes[i].stride;
            params.offsets[i] = attributes.planes[i].offset;
        }

        uint32_t id = 0;
        if (!device->addFramebuffer(params, &id)) {
            gbm_bo_destroy(bo);
            return nullptr;
        }

        auto framebuffer = std::unique_ptr<DrmFramebuffer>(new DrmFramebuffer(
                device, id, attributes.size, params.format, params.modifier, {}));
        framebuffer->m_importedBo = bo;
        return framebuffer;
    }

    std::vector<uint32_t> handles;
    for (size_t i = 0; i < params.planeCount; ++i) {
        uint32_t handle = 0;
        if (!device->primeFdToHandle(attributes.planes[i].fd.get(), &handle)) {
            for (uint32_t owned : handles)
                releaseHandle(device, owned);
            return nullptr;
        }
        retainHandle(device, handle);
        handles.push_back(handle);
        params.handles[i] = handle;
        params.strides[i] = attributes.planes[i].stride;
        params.offsets[i] = attributes.planes[i].offset;
    }

    uint32_t id = 0;
    if (!device->addFramebuffer(params, &id)) {
        for (uint32_t owned : handles)
            releaseHandle(device, owned);
        return nullptr;
    }

    return std::unique_ptr<DrmFramebuffer>(new DrmFramebuffer(
            device, id, attributes.size, params.format, params.modifier,
            std::move(handles)));
}

} // namespace Lingmo::Backend
