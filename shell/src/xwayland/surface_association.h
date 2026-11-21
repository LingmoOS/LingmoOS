// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <functional>
#include <map>

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::XWayland {

// One instance belongs to one Xwayland generation. Xwayland hands out serials
// in increasing order, so a matched or abandoned serial cannot come back.
class SurfaceAssociation
{
public:
    using Surface = compositor::Surface;
    static constexpr size_t limit = 65536;

    static uint64_t serial(uint32_t low, uint32_t high)
    {
        return uint64_t(low) | (uint64_t(high) << 32);
    }

    // Takes a serial from set_serial, which carries them in Xwayland's order;
    // the commits that add them may come in any order.
    bool claimSurfaceSerial(uint64_t serial);
    bool addSurface(uint64_t serial, Surface* surface);
    bool addWindow(uint64_t serial, uint32_t window);
    void removeSurface(Surface* surface);
    void removeWindow(uint32_t window);
    std::function<void(uint32_t, Surface*)> associated;

    size_t pendingCount() const { return m_surfaces.size() + m_windows.size(); }

private:
    void match(uint64_t serial);
    std::map<uint64_t, Surface*> m_surfaces;
    std::map<uint64_t, uint32_t> m_windows;
    uint64_t m_lastSurfaceSerial = 0;
    uint64_t m_lastWindowSerial = 0;
};

} // namespace Lingmo::XWayland
