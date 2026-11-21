// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_DRM_DRM_SELECTION_H_
#define LINGMO_BACKEND_DRM_DRM_SELECTION_H_

#include <xf86drmMode.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Pure connector/CRTC/mode selection. Kept free of ioctls so it can be tested
// without a DRM device.
//
// Reference: chromium/src/ui/ozone/platform/drm/common/drm_util.cc
// (GetCrtcs, ExtractDisplayModes)
namespace Lingmo::Backend {

struct ConnectorInfo {
    uint32_t id = 0;
    uint32_t type = 0;
    uint32_t typeId = 0;
    // drmModeConnection: DRM_MODE_CONNECTED etc.
    uint32_t connection = 0;
    int modeCount = 0;
    // The connector's "non-desktop" property: a head-mounted display, which a
    // VR runtime leases rather than the desktop extending onto it.
    bool nonDesktop = false;
};

// Whether the desktop may drive the connector: connected, with modes, and not
// a head-mounted display, which KWin and wlroots leave to leasing as well.
bool isDesktopOutput(const ConnectorInfo& connector);

// Index of the connector to drive: the first desktop output.
std::optional<int> selectConnector(const std::vector<ConnectorInfo>& connectors);

// Preferred mode, else the largest progressive one by pixel area (highest
// refresh breaks ties), else the largest interlaced one, else nothing.
std::optional<drmModeModeInfo> selectMode(const drmModeModeInfo* modes, int count);

// The modes with duplicates dropped: modes of the same size, clock, totals and
// flags, which differ only in sync positions, are one mode to the display
// configuration. A preferred duplicate replaces the one kept.
std::vector<drmModeModeInfo> distinctModes(const drmModeModeInfo* modes, int count);

// Picks a free CRTC from `crtcIds` (in drmModeRes order, so bit N of
// `possibleCrtcs` maps to index N). The CRTC the connector is already attached
// to wins when it is compatible and free, which avoids a needless modeset.
// Returns 0 when nothing is available.
uint32_t selectCrtc(uint32_t possibleCrtcs,
                    const std::vector<uint32_t>& crtcIds,
                    uint32_t usedCrtcMask,
                    uint32_t currentCrtcId = 0);

// A primary DRM node the desktop could run on.
struct DrmDeviceCandidate {
    std::string path;
    // The DRM driver, such as "i915", as drmGetVersion names it.
    std::string driver;
    // The firmware showed its boot console there: /sys/.../device/boot_vga.
    bool bootVga = false;
};

// The order in which to try `candidates` for the rendering and primary
// display device. The firmware's boot display goes first, as KWin and wlroots
// choose it, then ozone's preferred drivers (GetPreferredDrmDrivers), then the
// enumeration order.
std::vector<std::string> orderDisplayDevices(const std::vector<DrmDeviceCandidate>& candidates);

double refreshRate(const drmModeModeInfo& mode);

std::string connectorName(uint32_t type, uint32_t typeId);

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_SELECTION_H_
