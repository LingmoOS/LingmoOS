// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/sub_surface.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#pragma once

#include <memory>

#include "base/geometry.h"

namespace Lingmo::Compositor {

class Surface;

// A subsurface has one parent; the protocol resource owns this role object.
class SubSurface
{
public:
    static std::unique_ptr<SubSurface> create(Surface* surface, Surface* parent);
    ~SubSurface();
    SubSurface(const SubSurface&) = delete;
    SubSurface& operator=(const SubSurface&) = delete;

    Surface* surface() const { return m_surface; }

    Surface* parent() const { return m_parent; }

    void setPosition(const base::Point& position);
    bool placeAbove(Surface* reference);
    bool placeBelow(Surface* reference);
    void setCommitBehavior(bool synchronized);
    bool isSurfaceSynchronized() const;
    void onSurfaceDestroying(Surface* surface);

private:
    SubSurface(Surface* surface, Surface* parent);
    Surface* m_surface;
    Surface* m_parent;
    bool m_isSynchronized = true;
};
} // namespace Lingmo::Compositor
