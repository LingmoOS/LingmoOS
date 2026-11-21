// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/
//               hardware_display_plane_manager.cc
//               (HardwareDisplayPlaneManager::IsCompatible / AssignOverlayPlanes)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: a pure function over plane descriptions rather than a
// method mutating in_use/owning_crtc on a shared plane list, since one output
// on one CRTC has no plane migration to track.

#ifndef LINGMO_BACKEND_DRM_PLANE_ASSIGNMENT_H_
#define LINGMO_BACKEND_DRM_PLANE_ASSIGNMENT_H_

#include <cstdint>
#include <vector>

#include "backend/drm/drm_plane.h"

namespace Lingmo::Backend {

// What one piece of content needs from a plane.
struct PlaneRequest {
    uint32_t format = 0;
    uint64_t modifier = 0;
    // z 0 must land on the primary plane; everything above it on an overlay.
    bool isPrimary = false;
};

// The plane chosen for `requests[index]`.
struct PlaneAssignment {
    size_t requestIndex = 0;
    uint32_t planeId = 0;
};

// Assigns planes on `crtcIndex` to `requests`, in the order given.
//
// Returns an empty vector when any request cannot be placed: a partial
// assignment is never useful, because the caller would have to composite the
// leftovers and re-plan anyway. Cursor planes are never handed out here; they
// are driven through the legacy cursor ioctls.
std::vector<PlaneAssignment> assignPlanes(const std::vector<DrmPlane>& planes, int crtcIndex,
                                          const std::vector<PlaneRequest>& requests);

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_PLANE_ASSIGNMENT_H_
