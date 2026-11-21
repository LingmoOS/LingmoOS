// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_shm.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: base::UnsafeSharedMemoryRegion and viz formats give way
// to an mmapped fd and render::PixelFormat.

#include "wayland/protocols/wl_shm.h"

#include <wayland-server-protocol.h>

#include <algorithm>
#include <memory>

#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "compositor/shm_pool.h"
#include "core/logging.h"
#include "render/texture.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

using PoolRef = std::shared_ptr<compositor::ShmPool>;

////////////////////////////////////////////////////////////////////////////////
// wl_buffer_interface:

void buffer_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

const struct wl_buffer_interface buffer_implementation = { buffer_destroy };

void handleBufferReleaseCallback(wl_resource* resource)
{
    wl_client* client = wl_resource_get_client(resource);
    wl_buffer_send_release(resource);
    if (!isClientDestroyed(client))
        wl_client_flush(client);
}

////////////////////////////////////////////////////////////////////////////////
// wl_shm_pool_interface:

// wl_shm names formats by their little-endian word layout; render::PixelFormat
// names them by byte order in memory.
struct SupportedFormat {
    uint32_t shmFormat;
    render::PixelFormat pixelFormat;
};

constexpr SupportedFormat kSupportedFormats[] = {
    { WL_SHM_FORMAT_XRGB8888, render::PixelFormat::Bgrx8888 },
    { WL_SHM_FORMAT_ARGB8888, render::PixelFormat::Bgra8888 },
};

void shm_pool_create_buffer(wl_client* client, wl_resource* resource, uint32_t id,
                            int32_t offset, int32_t width, int32_t height, int32_t stride,
                            uint32_t format)
{
    const auto* supported = std::find_if(std::begin(kSupportedFormats),
                                         std::end(kSupportedFormats),
                                         [format](const SupportedFormat& candidate) {
                                             return candidate.shmFormat == format;
                                         });
    if (supported == std::end(kSupportedFormats)) {
        LM_WARNING(lingmoWayland).printf("wl_shm_pool.create_buffer: invalid format 0x%x", format);
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FORMAT, "invalid format 0x%x",
                               format);
        return;
    }

    compositor::ShmBufferError error = compositor::ShmBufferError::None;
    std::shared_ptr<compositor::ShmBuffer> buffer =
        (*getUserDataAs<PoolRef>(resource))
            ->createBuffer(base::Size(width, height), offset, stride, supported->pixelFormat,
                           &error);
    if (!buffer) {
        LM_WARNING(lingmoWayland).printf(
                "wl_shm_pool.create_buffer: rejected %dx%d stride %d offset %d", width, height,
                stride, offset);
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_STRIDE,
                               "invalid buffer geometry: %dx%d stride %d offset %d", width,
                               height, stride, offset);
        return;
    }

    wl_resource* bufferResource = wl_resource_create(client, &wl_buffer_interface, 1, id);
    if (!bufferResource) {
        wl_resource_post_no_memory(resource);
        return;
    }

    buffer->setReleaseCallback(
        [bufferResource] { handleBufferReleaseCallback(bufferResource); });
    buffer->setFaultCallback([bufferResource] {
        LM_WARNING(lingmoWayland).printf(
                "wl_shm: the file behind a pool shrank while it was being read");
        wl_resource_post_error(bufferResource, WL_SHM_ERROR_INVALID_FD,
                               "error accessing SHM buffer");
    });

    setBufferImplementation(bufferResource, &buffer_implementation, std::move(buffer));
}

void shm_pool_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void shm_pool_resize(wl_client*, wl_resource* resource, int32_t size)
{
    PoolRef* pool = getUserDataAs<PoolRef>(resource);
    if (size < 0 || static_cast<size_t>(size) < (*pool)->size()) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD, "can't shrink a shm pool");
        return;
    }

    if (!(*pool)->resize(size)) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD,
                               "failed to grow the pool to %d bytes", size);
    }
}

const struct wl_shm_pool_interface shm_pool_implementation = { shm_pool_create_buffer,
                                                               shm_pool_destroy,
                                                               shm_pool_resize };

////////////////////////////////////////////////////////////////////////////////
// wl_shm_interface:

void shm_create_pool(wl_client* client, wl_resource* resource, uint32_t id, int fd,
                     int32_t size)
{
    // createShmPool() closes the fd on every path, including failure.
    PoolRef pool = getUserDataAs<compositor::Compositor>(resource)->createShmPool(fd, size);
    if (!pool) {
        wl_resource_post_error(resource, WL_SHM_ERROR_INVALID_FD,
                               "failed to map a pool of %d bytes", size);
        return;
    }

    wl_resource* poolResource = wl_resource_create(client, &wl_shm_pool_interface, 1, id);
    if (!poolResource) {
        wl_resource_post_no_memory(resource);
        return;
    }

    setImplementation(poolResource, &shm_pool_implementation,
                      std::make_unique<PoolRef>(std::move(pool)));
}

const struct wl_shm_interface shm_implementation = { shm_create_pool };

void bind_shm(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    wl_resource* resource =
        wl_resource_create(client, &wl_shm_interface, std::min(version, kWlShmVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &shm_implementation, data, nullptr);

    for (const SupportedFormat& format : kSupportedFormats)
        wl_shm_send_format(resource, format.shmFormat);
}

} // namespace

void setBufferImplementation(wl_resource* resource, const void* implementation, BufferRef buffer)
{
    wl_resource_set_implementation(
        resource, implementation, new BufferRef(std::move(buffer)), [](wl_resource* resource) {
            std::unique_ptr<BufferRef> ref = takeUserDataAs<BufferRef>(resource);
            if (ref && *ref) {
                (*ref)->setReleaseCallback({});
                (*ref)->setFaultCallback({});
            }
        });
}

BufferRef bufferFromResource(wl_resource* resource)
{
    if (!resource)
        return nullptr;
    BufferRef* ref = getUserDataAs<BufferRef>(resource);
    return ref ? *ref : nullptr;
}

wl_global* createShmGlobal(wl_display* display, compositor::Compositor* compositor)
{
    return wl_global_create(display, &wl_shm_interface, kWlShmVersion, compositor, bind_shm);
}

} // namespace Lingmo::Wayland
