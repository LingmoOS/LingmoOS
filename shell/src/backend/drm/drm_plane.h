// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: the plane description and the format/CRTC predicates
// are kept free of ioctls, so plane assignment is testable; enumeration lives
// in drm_backend.cpp.

#ifndef LINGMO_BACKEND_DRM_DRM_PLANE_H_
#define LINGMO_BACKEND_DRM_DRM_PLANE_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "backend/drm/drm_properties.h"

namespace Lingmo::Backend {

// One KMS plane: what it can show and which CRTCs may drive it.
struct DrmPlane {
    uint32_t id = 0;
    // DRM_PLANE_TYPE_PRIMARY / _OVERLAY / _CURSOR.
    uint32_t type = 0;
    // Bit N is set when the CRTC at index N in drmModeRes may use this plane.
    uint32_t possibleCrtcs = 0;
    DrmPropertyMap properties;
    // DRM fourcc -> the modifiers this plane accepts for it. An empty modifier
    // list means the plane was only enumerated through its legacy format list,
    // where the modifier is implicit.
    std::map<uint32_t, std::vector<uint64_t>> formats;

    bool canUseForCrtc(int crtcIndex) const
    {
        return crtcIndex >= 0 && crtcIndex < 32
            && (possibleCrtcs & (1u << static_cast<uint32_t>(crtcIndex)));
    }

    // Whether the plane can scan out `format` laid out with `modifier`.
    bool supportsFormat(uint32_t format, uint64_t modifier) const;

    // Property names an atomic commit of a plane cannot do without. A plane
    // missing any of these is unusable and is dropped at enumeration.
    static std::vector<std::string> requiredProperties();
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_PLANE_H_
