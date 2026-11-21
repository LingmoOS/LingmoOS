// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/buffer.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_WAYLAND_PROTOCOLS_LINUX_DRM_SYNCOBJ_H_
#define LINGMO_WAYLAND_PROTOCOLS_LINUX_DRM_SYNCOBJ_H_

#include <wayland-server-core.h>

#include <cstdint>

namespace Lingmo::Wayland {

// wp_linux_drm_syncobj_v1: the current Wayland explicit-synchronization
// protocol. Exo has no equivalent - it synchronizes through gpu::SyncToken
// inside Chromium's GPU process - so only the DRM fence handling underneath is
// ported; see render/drm_syncobj.h and render/sync_fence.h.
constexpr uint32_t kLinuxDrmSyncobjVersion = 1;

// `drmFd` is the device the client's timelines are imported onto. Returns null
// when it is not a usable DRM device, in which case clients fall back to
// implicit synchronization.
wl_global* createLinuxDrmSyncobjGlobal(wl_display* display, int drmFd);

// What a surface commit made with a wp_linux_drm_syncobj_surface_v1 in place
// breaks, as a wp_linux_drm_syncobj_surface_v1 error code, or -1. Once the
// object exists every non-null buffer needs both points and fence support, and
// points need a buffer. `conflicting` is an acquire point at or after the
// release point on the same timeline.
struct SyncobjCommit {
    bool buffer = false;
    bool explicitSyncBuffer = false;
    bool acquire = false;
    bool release = false;
    bool conflicting = false;
};

int syncobjCommitError(const SyncobjCommit& commit);

} // namespace Lingmo::Wayland

#endif // LINGMO_WAYLAND_PROTOCOLS_LINUX_DRM_SYNCOBJ_H_
