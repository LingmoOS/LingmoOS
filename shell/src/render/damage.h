// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "render/output_render_context.h"
#include <cmath>
#include <deque>
#include <optional>
#include <vector>
#include "base/region.h"

namespace Lingmo::Render {

inline constexpr int kBackdropBlurSupport = 90;

inline base::Rect physicalDamage(const base::Region &damage, const OutputRenderContext &context,
                            const base::Size &targetSize)
{
    const base::Rect target(base::Point(), targetSize);
    const base::Rect content = context.nativeContentRect.isEmpty() ? target : context.nativeContentRect;
    if (context.transform > 3 || context.scale <= 0)
        return target;
    // drawTextureMapped rounds a native-density texture's origin and size
    // apart, which can reach one pixel past the enclosing rect on the far side.
    const double overhang = context.scale == std::floor(context.scale) ? 0 : 1;
    base::Region result;
    for (const base::Rect &rect : damage.intersected(context.logicalViewport)) {
        base::RectF local(rect.translated(-context.logicalViewport.topLeft()));
        local = base::RectF(local.topLeft() * context.scale,
                       local.size() * context.scale + base::SizeF(overhang, overhang));
        const double w = content.width(), h = content.height();
        switch (context.transform) {
        case 1:
            local = base::RectF(local.y(), h - local.x() - local.width(), local.height(), local.width());
            break;
        case 2:
            local = base::RectF(w - local.x() - local.width(), h - local.y() - local.height(),
                           local.width(), local.height());
            break;
        case 3:
            local = base::RectF(w - local.y() - local.height(), local.x(), local.height(),
                           local.width());
            break;
        }
        // Enclose after transforming; adjacent fractional edges must never lose pixels.
        result += local.translated(content.topLeft()).toEnclosingRect().intersected(content);
    }
    return result.boundingRect().intersected(target);
}

// The logical rectangle a physical repaint takes in, for culling: anything
// physicalDamage() maps into `repaint` comes from inside it. None for flips.
inline std::optional<base::Rect> logicalRepaint(const base::Rect &repaint,
                                           const OutputRenderContext &context,
                                           const base::Size &targetSize)
{
    if (context.transform > 3 || context.scale <= 0)
        return std::nullopt;
    const base::Rect content = context.nativeContentRect.isEmpty() ? base::Rect(base::Point(), targetSize)
                                                              : context.nativeContentRect;
    base::RectF local(repaint.translated(-content.topLeft()));
    const double w = content.width(), h = content.height();
    switch (context.transform) {
    case 1:
        local = base::RectF(h - local.y() - local.height(), local.x(), local.height(), local.width());
        break;
    case 2:
        local = base::RectF(w - local.x() - local.width(), h - local.y() - local.height(),
                       local.width(), local.height());
        break;
    case 3:
        local = base::RectF(local.y(), w - local.x() - local.width(), local.height(), local.width());
        break;
    }
    local = base::RectF(local.topLeft() / context.scale, local.size() / context.scale);
    // A pixel more on every side covers the overhang and the rounding out.
    return local.toEnclosingRect()
            .adjusted(-1, -1, 1, 1)
            .translated(context.logicalViewport.topLeft());
}

// A backdrop blur reads everything in its `samples` region, and outside the
// repaint the target holds a finished frame, not the backdrop. A repaint that
// reaches such a region, if only through a bounding rectangle or the repair
// of an older buffer, takes in all of it.
inline base::Rect coverSamples(base::Rect repaint, const std::vector<base::Rect> &samples)
{
    for (bool grown = true; grown && !repaint.isEmpty();) {
        grown = false;
        for (const base::Rect &sampled : samples) {
            if (repaint.intersects(sampled) && !repaint.contains(sampled)) {
                repaint = repaint.united(sampled);
                grown = true;
            }
        }
    }
    return repaint;
}

// The same for a physical repaint and logical sample regions.
inline base::Rect coverSamples(const base::Rect &repaint, const std::vector<base::Rect> &samples,
                          const OutputRenderContext &context, const base::Size &targetSize)
{
    std::vector<base::Rect> physical;
    for (const base::Rect &sampled : samples)
        physical.push_back(physicalDamage(sampled, context, targetSize));
    return coverSamples(repaint, physical);
}

class BufferDamageHistory
{
public:
    base::Rect repair(const base::Rect &damage, int age, const base::Rect &full) const
    {
        if (age <= 0 || size_t(age - 1) > m_history.size())
            return full;
        base::Rect result = damage;
        for (int i = 0; i < age - 1; ++i)
            result = result.united(m_history[i]);
        return result.intersected(full);
    }

    void presented(const base::Rect &damage)
    {
        m_history.push_front(damage);
        if (m_history.size() > 16)
            m_history.pop_back();
    }

    void reset() { m_history.clear(); }

private:
    std::deque<base::Rect> m_history;
};

} // namespace Lingmo::Render
