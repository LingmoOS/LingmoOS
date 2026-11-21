// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkSurface.h"

namespace Lingmo::ui {

class View;

// A layered view's texture: the view and its children painted untransformed.
struct Layer
{
    sk_sp<SkSurface> surface;
    // Where the texture sits untransformed, in window device pixels.
    SkIRect bounds = SkIRect::MakeEmpty();
    // Window device pixels, untransformed, whose texture content is stale.
    SkRegion damage;
    bool repaintAll = true;
};

// What changed in the window since the last presented frame.
struct FrameDamage
{
    SkRegion pending;
    bool full = true;
    // What each of the last presented frames changed, newest first.
    std::deque<SkRegion> history;

    // A view that draws what a subtree paints: where it reads it, untransformed
    // in window device pixels, and how often that may have changed.
    struct Watch
    {
        const View* watcher;
        const View* subtree;
        SkIRect rect;
        uint64_t changes = 0;
    };
    std::vector<Watch> watches;
};

} // namespace Lingmo::ui
