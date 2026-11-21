// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_shm.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#ifndef LINGMO_WAYLAND_PROTOCOLS_WL_SHM_H_
#define LINGMO_WAYLAND_PROTOCOLS_WL_SHM_H_

#include <wayland-server-core.h>

#include <cstdint>
#include <memory>

namespace Lingmo::Compositor {
class Buffer;
class Compositor;
}

namespace Lingmo::Wayland {

constexpr uint32_t kWlShmVersion = 1;

// A wl_buffer resource owns its Buffer; surfaces only hold weak references, so
// that a client destroying wl_buffer while it is committed cannot dangle.
using BufferRef = std::shared_ptr<compositor::Buffer>;

// Returns the buffer behind a wl_buffer resource, or nullptr for a null
// resource.
BufferRef bufferFromResource(wl_resource* resource);

// Installs `implementation` and takes ownership of `buffer` for the wl_buffer
// resource. Also clears the buffer's release callback when the resource dies:
// a Buffer can outlive its wl_buffer - the display may still be scanning it
// out - and must not then send a release event to a resource that is gone.
void setBufferImplementation(wl_resource* resource, const void* implementation, BufferRef buffer);

wl_global* createShmGlobal(wl_display* display, compositor::Compositor* compositor);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_WL_SHM_H_
