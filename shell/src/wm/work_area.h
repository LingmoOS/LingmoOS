// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_WORK_AREA_H_
#define LINGMO_WM_WORK_AREA_H_

#include <map>
#include <string>

#include "base/geometry.h"
#include "wm/types.h"

namespace Lingmo::WM {

// Aggregates the work-area reservations of the shell's panels (the status bar,
// the dock). Keys are opaque tokens owned by the caller - the panel's window -
// so a reservation can be dropped again when the panel that made it goes.
class ExclusiveZones
{
public:
    struct Zone {
        ScreenEdge edge = ScreenEdge::Top;
        int thickness = 0;
        std::string outputName;
        // Withdrawn while a window on its output is maximized, as a dock that
        // hides for maximized windows is.
        bool yieldsToMaximized = false;
    };

    // Returns true when the aggregate changed and the work area has to be
    // recomputed.
    bool set(const void* token, ScreenEdge edge, int thickness,
             const std::string& outputName = {}, bool yieldsToMaximized = false);
    bool clear(const void* token);

    bool isEmpty() const { return m_zones.empty(); }
    bool hasYieldingZone() const;

    // `outputBounds` minus every reservation. Reservations on the same edge do
    // not stack: the largest one wins, so a dock that briefly has two
    // reservations in flight cannot shrink the work area twice. `maximized`
    // leaves out the reservations that yield to maximized windows.
    base::Rect apply(const base::Rect& outputBounds) const;
    base::Rect apply(const base::Rect& outputBounds, const std::string& outputName, bool primary,
                     bool maximized = false) const;

private:
    std::map<const void*, Zone> m_zones;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_WORK_AREA_H_
