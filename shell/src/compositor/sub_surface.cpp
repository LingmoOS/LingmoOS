// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/sub_surface.cc, display.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "compositor/sub_surface.h"

#include "compositor/surface.h"

namespace Lingmo::Compositor {

std::unique_ptr<SubSurface> SubSurface::create(Surface* surface, Surface* parent)
{
    // Display::CreateSubSurface: parent must not be contained in child's tree,
    // and the surface must not already have a role object.
    // hasRole() rather than only an existing subsurface: a surface already
    // serving as a pointer cursor may not take the subsurface role either.
    if (!surface || !parent || surface->hasRole())
        return nullptr;
    for (auto* ancestor = parent; ancestor; ancestor = ancestor->parentSurface())
        if (ancestor == surface)
            return nullptr;
    return std::unique_ptr<SubSurface>(new SubSurface(surface, parent));
}

SubSurface::SubSurface(Surface* surface, Surface* parent)
    : m_surface(surface)
    , m_parent(parent)
{
    m_surface->m_subSurface = this;
    m_parent->addSubSurface(m_surface);
}

SubSurface::~SubSurface()
{
    if (m_surface) {
        if (m_parent)
            m_parent->removeSubSurface(m_surface);
        m_surface->m_subSurface = nullptr;
        m_surface->updateMapping();
    }
    // Destroying a sub-surface takes effect immediately (remove schedules paint).
}

void SubSurface::setPosition(const base::Point& position)
{
    if (!m_parent || !m_surface)
        return;
    m_parent->setSubSurfacePosition(m_surface, position);
}

bool SubSurface::placeAbove(Surface* reference)
{
    if (!m_parent || !m_surface)
        return true; // inert resource
    return m_parent->placeSubSurface(m_surface, reference, true);
}

bool SubSurface::placeBelow(Surface* reference)
{
    if (!m_parent || !m_surface)
        return true;
    return m_parent->placeSubSurface(m_surface, reference, false);
}

void SubSurface::setCommitBehavior(bool synchronized)
{
    m_isSynchronized = synchronized;
    // wl_subsurface.set_desync requires immediate application of an existing
    // cache if no ancestor still synchronizes us. Exo's setter alone doesn't
    // flush it; Lingmo has no SurfaceTreeHost to trigger that application.
    if (m_surface && !isSurfaceSynchronized())
        m_surface->requestDesynchronizedCommit();
}

bool SubSurface::isSurfaceSynchronized() const
{
    // A sub-surface is effectively synchronized if either its parent is
    // synchronized or itself is in synchronized mode. Preserve Exo's recursive
    // ancestor walk, including desynchronized links within a synchronized tree.
    if (m_isSynchronized)
        return true;
    return m_parent && m_parent->isSynchronized();
}

void SubSurface::onSurfaceDestroying(Surface* surface)
{
    if (surface == m_parent) {
        m_parent->removeSubSurface(m_surface);
        m_parent = nullptr;
        m_surface->updateMapping();
        return;
    }
    if (m_parent)
        m_parent->removeSubSurface(m_surface);
    m_surface = nullptr;
    // No observer registration remains on the parent after the child dies.
    m_parent = nullptr;
}
} // namespace Lingmo::Compositor
