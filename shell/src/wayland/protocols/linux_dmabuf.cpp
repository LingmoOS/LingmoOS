// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/zwp_linux_dmabuf.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: gfx::NativePixmapHandle becomes render::DmaBufAttributes
// and Display::CreateLinuxDMABufBuffer becomes Compositor::createDmaBufBuffer.
// The plane_idx, already_used and out_of_bounds checks are protocol-mandated
// errors upstream leaves to its GPU import; they are explicit here.

#include "wayland/protocols/linux_dmabuf.h"

#include <drm_fourcc.h>
#include <linux-dmabuf-v1-server-protocol.h>
#include <wayland-server-protocol.h>

#include <unistd.h>

#include <algorithm>
#include <map>
#include <utility>

#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "core/logging.h"
#include "core/scoped_fd.h"
#include "render/dmabuf.h"
#include "wayland/protocols/dmabuf_feedback_manager.h"
#include "wayland/protocols/wl_shm.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

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
// linux_buffer_params_interface:

struct LinuxBufferParams {
    struct Plane {
        ScopedFd fd;
        uint32_t stride = 0;
        uint32_t offset = 0;
        uint64_t modifier = 0;
    };

    explicit LinuxBufferParams(DmaBufFeedbackManager* feedbackManager)
        : feedbackManager(feedbackManager)
    {
    }

    DmaBufFeedbackManager* const feedbackManager;
    std::map<uint32_t, Plane> planes;
    // A params object may back at most one buffer.
    bool used = false;
};

void linux_buffer_params_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

// Returns false and posts already_used when `params` has already created a
// buffer. Every request but destroy is invalid from then on.
bool checkNotUsed(wl_resource* resource, LinuxBufferParams* params)
{
    if (!params->used)
        return true;

    wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                           "the params object has already been used to create a wl_buffer");
    return false;
}

void linux_buffer_params_add(wl_client*, wl_resource* resource, int32_t fd, uint32_t planeIdx,
                             uint32_t offset, uint32_t stride, uint32_t modifierHi,
                             uint32_t modifierLo)
{
    auto* params = getUserDataAs<LinuxBufferParams>(resource);

    const uint64_t modifier = (static_cast<uint64_t>(modifierHi) << 32) | modifierLo;
    LinuxBufferParams::Plane plane { ScopedFd(fd), stride, offset, modifier };

    if (!checkNotUsed(resource, params))
        return;

    if (planeIdx >= render::kMaxDmaBufPlanes) {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                               "plane index %u is out of bounds", planeIdx);
        return;
    }

    const bool inserted = params->planes.insert(std::make_pair(planeIdx, std::move(plane))).second;
    if (!inserted) { // The plane was already there.
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET,
                               "plane already set");
    }
}

// The size of the DMA-BUF behind `fd`, or 0 when it cannot be determined, in
// which case the range check is skipped rather than rejecting a valid buffer.
uint64_t dmaBufSize(int fd)
{
    const off_t end = lseek(fd, 0, SEEK_END);
    return end <= 0 ? 0 : static_cast<uint64_t>(end);
}

// Every plane must describe a range that actually exists inside its fd. Not in
// upstream Exo, which lets the GPU import fail instead; the protocol names an
// error for it, and diagnosing that error beats diagnosing a blank window.
bool validatePlaneRanges(wl_resource* resource, const LinuxBufferParams* params, int32_t height)
{
    for (const auto& [index, plane] : params->planes) {
        if (plane.stride == 0) {
            wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                                   "plane %u has a zero stride", index);
            return false;
        }

        const uint64_t size = dmaBufSize(plane.fd.get());
        if (size == 0)
            continue;

        // Planes may be subsampled, so this is the loosest bound that still
        // catches a plane pointing outside its own fd.
        const uint64_t end = static_cast<uint64_t>(plane.offset) + plane.stride;
        if (plane.offset >= size || end > size) {
            wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                                   "plane %u: offset %u and stride %u exceed the %llu byte "
                                   "buffer",
                                   index, plane.offset, plane.stride,
                                   static_cast<unsigned long long>(size));
            return false;
        }

        if (index == 0) {
            const uint64_t needed = static_cast<uint64_t>(plane.offset)
                + static_cast<uint64_t>(plane.stride) * static_cast<uint64_t>(height);
            if (needed > size) {
                wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                                       "plane 0 needs %llu bytes but the buffer holds %llu",
                                       static_cast<unsigned long long>(needed),
                                       static_cast<unsigned long long>(size));
                return false;
            }
        }
    }
    return true;
}

bool validateLinuxBufferParams(wl_resource* resource, int32_t width, int32_t height, uint32_t flags)
{
    if (width <= 0 || height <= 0) {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
                               "invalid width or height");
        return false;
    }

    if (flags & ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_INTERLACED) {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
                               "flags not supported");
        return false;
    }

    auto* params = getUserDataAs<LinuxBufferParams>(resource);

    const size_t numPlanes = params->planes.size();
    if (numPlanes == 0) {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
                               "no planes given");
        return false;
    }

    // Validate that we have planes 0..numPlanes-1.
    for (uint32_t i = 0; i < numPlanes; ++i) {
        if (!params->planes.contains(i)) {
            wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
                                   "missing a plane");
            return false;
        }
    }

    // All planes must have the same modifier.
    const uint64_t modifier = params->planes[0].modifier;
    for (uint32_t i = 1; i < numPlanes; ++i) {
        if (params->planes[i].modifier != modifier) {
            wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT,
                                   "all planes must have same modifier");
            return false;
        }
    }

    return validatePlaneRanges(resource, params, height);
}

wl_resource* create_buffer(wl_client* client, wl_resource* resource, uint32_t bufferId,
                           int32_t width, int32_t height, uint32_t format, uint32_t flags)
{
    auto* params = getUserDataAs<LinuxBufferParams>(resource);

    if (!checkNotUsed(resource, params))
        return nullptr;

    if (!params->feedbackManager->isFormatSupported(format)) {
        wl_resource_post_error(resource, ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_FORMAT,
                               "format not supported");
        return nullptr;
    }

    if (!validateLinuxBufferParams(resource, width, height, flags))
        return nullptr;

    params->used = true;

    render::DmaBufAttributes attributes;
    attributes.size = base::Size(width, height);
    attributes.format = format;
    attributes.modifier = params->planes[0].modifier;
    attributes.yInvert = (flags & ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT) != 0;

    for (uint32_t i = 0; i < params->planes.size(); ++i) {
        auto& plane = params->planes[i];
        attributes.planes.push_back(
            render::DmaBufPlane { std::move(plane.fd), plane.offset, plane.stride });
    }
    params->planes.clear();

    std::shared_ptr<compositor::Buffer> buffer =
        params->feedbackManager->compositor()->createDmaBufBuffer(std::move(attributes));
    if (!buffer) {
        zwp_linux_buffer_params_v1_send_failed(resource);
        return nullptr;
    }

    wl_resource* bufferResource = wl_resource_create(client, &wl_buffer_interface, 1, bufferId);
    if (!bufferResource) {
        wl_resource_post_no_memory(resource);
        return nullptr;
    }

    buffer->setReleaseCallback(
        [bufferResource] { handleBufferReleaseCallback(bufferResource); });

    setBufferImplementation(bufferResource, &buffer_implementation, std::move(buffer));

    return bufferResource;
}

void linux_buffer_params_create(wl_client* client, wl_resource* resource, int32_t width,
                                int32_t height, uint32_t format, uint32_t flags)
{
    wl_resource* bufferResource = create_buffer(client, resource, 0, width, height, format, flags);

    if (bufferResource)
        zwp_linux_buffer_params_v1_send_created(resource, bufferResource);
}

void linux_buffer_params_create_immed(wl_client* client, wl_resource* resource,
                                      uint32_t bufferId, int32_t width, int32_t height,
                                      uint32_t format, uint32_t flags)
{
    create_buffer(client, resource, bufferId, width, height, format, flags);
}

const struct zwp_linux_buffer_params_v1_interface linux_buffer_params_implementation = {
    linux_buffer_params_destroy, linux_buffer_params_add, linux_buffer_params_create,
    linux_buffer_params_create_immed
};

////////////////////////////////////////////////////////////////////////////////
// linux_dmabuf_interface:

void linux_dmabuf_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

void linux_dmabuf_create_params(wl_client* client, wl_resource* resource, uint32_t id)
{
    auto* feedbackManager = getUserDataAs<DmaBufFeedbackManager>(resource);

    wl_resource* paramsResource = wl_resource_create(
        client, &zwp_linux_buffer_params_v1_interface, wl_resource_get_version(resource), id);
    if (!paramsResource) {
        wl_resource_post_no_memory(resource);
        return;
    }

    setImplementation(paramsResource, &linux_buffer_params_implementation,
                      std::make_unique<LinuxBufferParams>(feedbackManager));
}

void linux_dmabuf_get_default_feedback(wl_client* client, wl_resource* dmaBufResource,
                                       uint32_t feedbackId)
{
    getUserDataAs<DmaBufFeedbackManager>(dmaBufResource)
        ->getDefaultFeedback(client, dmaBufResource, feedbackId);
}

void linux_dmabuf_get_surface_feedback(wl_client* client, wl_resource* dmaBufResource,
                                       uint32_t feedbackId, wl_resource* surfaceResource)
{
    getUserDataAs<DmaBufFeedbackManager>(dmaBufResource)
        ->getSurfaceFeedback(client, dmaBufResource, feedbackId, surfaceResource);
}

const struct zwp_linux_dmabuf_v1_interface linux_dmabuf_implementation = {
    linux_dmabuf_destroy,
    linux_dmabuf_create_params,
    linux_dmabuf_get_default_feedback,
    linux_dmabuf_get_surface_feedback,
};

void bind_linux_dmabuf(wl_client* client, void* data, uint32_t version, uint32_t id)
{
    auto* feedbackManager = static_cast<DmaBufFeedbackManager*>(data);

    wl_resource* resource = wl_resource_create(
        client, &zwp_linux_dmabuf_v1_interface,
        std::min(version, feedbackManager->versionSupportedByPlatform()), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &linux_dmabuf_implementation, feedbackManager,
                                   nullptr);

    if (wl_resource_get_version(resource)
        < ZWP_LINUX_DMABUF_V1_GET_DEFAULT_FEEDBACK_SINCE_VERSION) {
        feedbackManager->sendFormatsAndModifiers(resource);
    }
}

} // namespace

LinuxDmabufGlobal::LinuxDmabufGlobal(wl_global* global,
                                     std::unique_ptr<DmaBufFeedbackManager> manager)
    : m_global(global)
    , m_feedbackManager(std::move(manager))
{
}

LinuxDmabufGlobal::~LinuxDmabufGlobal()
{
    if (m_global)
        wl_global_destroy(m_global);
}

std::unique_ptr<LinuxDmabufGlobal> LinuxDmabufGlobal::create(wl_display* display,
                                                             compositor::Compositor* compositor,
                                                             render::DmaBufDevice* device)
{
    if (!display || !compositor || !device)
        return nullptr;

    auto manager = std::make_unique<DmaBufFeedbackManager>(compositor, device);
    if (manager->formatsAndModifiers().empty()) {
        LM_WARNING(lingmoWayland) << "No importable DMA-BUF format; not advertising linux-dmabuf";
        return nullptr;
    }

    // The global's version is the platform's, so clients never bind a version
    // whose requests we could not answer.
    wl_global* global = wl_global_create(
        display, &zwp_linux_dmabuf_v1_interface,
        std::min(kLinuxDmabufVersion, manager->versionSupportedByPlatform()), manager.get(),
        bind_linux_dmabuf);
    if (!global)
        return nullptr;

    return std::unique_ptr<LinuxDmabufGlobal>(
        new LinuxDmabufGlobal(global, std::move(manager)));
}

} // namespace Lingmo::Wayland
