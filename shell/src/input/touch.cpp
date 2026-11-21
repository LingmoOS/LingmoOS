// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/touch.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: see touch.h.

#include "input/touch.h"

#include "compositor/surface.h"
#include "input/hit_test.h"
#include "input/seat.h"
#include "input/touch_delegate.h"

namespace Lingmo::Input {

Touch::Touch(TouchDelegate* delegate, Seat* seat)
    : m_delegate(delegate)
    , m_seat(seat)
{
    m_seat->addTouch(this);
}

Touch::~Touch()
{
    m_delegate->onTouchDestroying(this);
    for (auto& [surface, count] : m_surfaceCounts)
        surface->removeSurfaceObserver(this);
    m_seat->removeTouch(this);
}

void Touch::onTouchDown(compositor::Surface* target, Timestamp timestamp, int32_t id,
                        const base::PointF& rootLocation)
{
    if (!target || !m_delegate->canAcceptTouchEventsForSurface(target) || m_points.contains(id))
        return;
    m_points.emplace(id, target);
    if (m_surfaceCounts[target]++ == 0)
        target->addSurfaceObserver(this);
    m_delegate->onTouchDown(target, timestamp, id, locationInSurface(target, rootLocation));
    m_framePending = true;
}

void Touch::onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF& rootLocation)
{
    const auto it = m_points.find(id);
    if (it == m_points.end())
        return;
    // Relative to the surface the point went down on, wherever it has moved.
    m_delegate->onTouchMotion(timestamp, id, locationInSurface(it->second, rootLocation));
    m_framePending = true;
}

void Touch::onTouchUp(Timestamp timestamp, int32_t id)
{
    const auto it = m_points.find(id);
    if (it == m_points.end())
        return;
    compositor::Surface* surface = it->second;
    m_points.erase(it);
    if (--m_surfaceCounts[surface] <= 0) {
        m_surfaceCounts.erase(surface);
        surface->removeSurfaceObserver(this);
    }
    m_delegate->onTouchUp(timestamp, id);
    m_framePending = true;
}

void Touch::onTouchCancel()
{
    if (m_points.empty())
        return;
    // One cancelled point cancels the whole touch session.
    cancelAllTouches();
    m_delegate->onTouchCancel();
    m_framePending = false;
}

void Touch::onTouchFrame()
{
    if (!std::exchange(m_framePending, false))
        return;
    m_delegate->onTouchFrame();
}

void Touch::onSurfaceDestroying(compositor::Surface*)
{
    // As upstream, a surface going away cancels every point, not only its own.
    cancelAllTouches();
    m_delegate->onTouchCancel();
    m_framePending = false;
}

void Touch::cancelAllTouches()
{
    for (auto& [surface, count] : m_surfaceCounts)
        surface->removeSurfaceObserver(this);
    m_points.clear();
    m_surfaceCounts.clear();
}

} // namespace Lingmo::Input
