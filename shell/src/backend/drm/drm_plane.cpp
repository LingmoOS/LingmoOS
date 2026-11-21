// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "backend/drm/drm_plane.h"

#include <drm_fourcc.h>

#include <algorithm>

namespace Lingmo::Backend {

std::vector<std::string> DrmPlane::requiredProperties()
{
    return {
        "type",   "FB_ID",  "CRTC_ID", "SRC_X",  "SRC_Y",  "SRC_W",
        "SRC_H",  "CRTC_X", "CRTC_Y",  "CRTC_W", "CRTC_H",
    };
}

bool DrmPlane::supportsFormat(uint32_t format, uint64_t modifier) const
{
    const auto entry = formats.find(format);
    if (entry == formats.end())
        return false;

    // No IN_FORMATS blob: the plane was enumerated through its legacy format
    // list, which only ever describes implicitly modified buffers.
    if (entry->second.empty())
        return modifier == DRM_FORMAT_MOD_INVALID || modifier == DRM_FORMAT_MOD_LINEAR;

    if (modifier == DRM_FORMAT_MOD_INVALID) {
        // An implicit modifier is whatever the allocator picked. Only the
        // driver knows, so it is accepted and left to the atomic test.
        return true;
    }
    return std::find(entry->second.begin(), entry->second.end(), modifier) != entry->second.end();
}

} // namespace Lingmo::Backend
