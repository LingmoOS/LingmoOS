// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/drm/drm_selection.h"

#include <algorithm>
#include <utility>

#include <xf86drm.h>

namespace Lingmo::Backend {

bool isDesktopOutput(const ConnectorInfo& connector)
{
    return connector.connection == DRM_MODE_CONNECTED && connector.modeCount > 0
        && !connector.nonDesktop;
}

std::optional<int> selectConnector(const std::vector<ConnectorInfo>& connectors)
{
    for (size_t i = 0; i < connectors.size(); ++i) {
        if (isDesktopOutput(connectors[i]))
            return int(i);
    }
    return std::nullopt;
}

std::vector<std::string> orderDisplayDevices(const std::vector<DrmDeviceCandidate>& candidates)
{
    static const std::vector<std::string> preferred = {"i915", "amdgpu", "virtio_gpu"};
    const auto rank = [](const DrmDeviceCandidate& candidate) {
        const auto index =
            std::find(preferred.begin(), preferred.end(), candidate.driver) - preferred.begin();
        return std::pair(candidate.bootVga ? 0 : 1, index);
    };
    std::vector<DrmDeviceCandidate> ordered = candidates;
    std::stable_sort(ordered.begin(), ordered.end(),
                     [&](const DrmDeviceCandidate& a, const DrmDeviceCandidate& b) {
                         return rank(a) < rank(b);
                     });
    std::vector<std::string> paths;
    for (const DrmDeviceCandidate& candidate : ordered)
        paths.push_back(candidate.path);
    return paths;
}

std::optional<drmModeModeInfo> selectMode(const drmModeModeInfo* modes, int count)
{
    if (!modes || count <= 0)
        return std::nullopt;

    const auto interlaced = [](const drmModeModeInfo& mode) {
        return (mode.flags & DRM_MODE_FLAG_INTERLACE) != 0;
    };
    const drmModeModeInfo* best = nullptr;
    for (int i = 0; i < count; ++i) {
        const drmModeModeInfo& mode = modes[i];
        if (mode.type & DRM_MODE_TYPE_PREFERRED)
            return mode;

        if (!best) {
            best = &mode;
            continue;
        }
        // Interlaced scanout flickers on text and is only a last resort.
        if (interlaced(mode) != interlaced(*best)) {
            if (!interlaced(mode))
                best = &mode;
            continue;
        }

        const long bestArea = static_cast<long>(best->hdisplay) * best->vdisplay;
        const long area = static_cast<long>(mode.hdisplay) * mode.vdisplay;
        if (area > bestArea || (area == bestArea && refreshRate(mode) > refreshRate(*best)))
            best = &mode;
    }

    return *best;
}

std::vector<drmModeModeInfo> distinctModes(const drmModeModeInfo* modes, int count)
{
    std::vector<drmModeModeInfo> result;
    for (int i = 0; modes && i < count; ++i) {
        const drmModeModeInfo& mode = modes[i];
        const auto kept = std::find_if(result.begin(), result.end(),
                                       [&mode](const drmModeModeInfo& other) {
                                           return other.hdisplay == mode.hdisplay
                                               && other.vdisplay == mode.vdisplay
                                               && other.clock == mode.clock
                                               && other.htotal == mode.htotal
                                               && other.vtotal == mode.vtotal
                                               && other.vscan == mode.vscan
                                               && other.flags == mode.flags;
                                       });
        if (kept == result.end())
            result.push_back(mode);
        else if ((mode.type & DRM_MODE_TYPE_PREFERRED) && !(kept->type & DRM_MODE_TYPE_PREFERRED))
            *kept = mode;
    }

    return result;
}

uint32_t selectCrtc(uint32_t possibleCrtcs,
                    const std::vector<uint32_t>& crtcIds,
                    uint32_t usedCrtcMask,
                    uint32_t currentCrtcId)
{
    uint32_t first = 0;
    for (size_t i = 0; i < crtcIds.size(); ++i) {
        const uint32_t bit = 1u << i;
        if (!(possibleCrtcs & bit) || (usedCrtcMask & bit))
            continue;
        if (crtcIds[i] == currentCrtcId && currentCrtcId != 0)
            return currentCrtcId;
        if (!first)
            first = crtcIds[i];
    }
    return first;
}

double refreshRate(const drmModeModeInfo& mode)
{
    if (mode.htotal == 0 || mode.vtotal == 0)
        return 0.0;

    double total = static_cast<double>(mode.htotal) * mode.vtotal;
    if (mode.flags & DRM_MODE_FLAG_INTERLACE)
        total /= 2.0;
    if (mode.flags & DRM_MODE_FLAG_DBLSCAN)
        total *= 2.0;
    if (mode.vscan > 1)
        total *= mode.vscan;

    return mode.clock * 1000.0 / total;
}

std::string connectorName(uint32_t type, uint32_t typeId)
{
    const char* name = drmModeGetConnectorTypeName(type);
    return std::string(name ? name : "Unknown") + '-' + std::to_string(typeId);
}

} // namespace Lingmo::Backend
