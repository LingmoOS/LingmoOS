// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "input/shortcuts_inhibition.h"

#include <utility>

#include "compositor/surface.h"
#include "input/seat.h"
#include "input/session_gate.h"

namespace Lingmo::Input {

namespace {

const compositor::Surface *rootSurface(const compositor::Surface *surface)
{
    while (surface && surface->parentSurface())
        surface = surface->parentSurface();
    return surface;
}

} // namespace

ShortcutsInhibition::ShortcutsInhibition(Seat *seat)
    : m_seat(seat)
{
}

ShortcutsInhibition::~ShortcutsInhibition()
{
    m_active = nullptr;
    for (auto &[surface, delegate] : std::exchange(m_inhibitors, {}))
        surface->removeSurfaceObserver(this);
}

bool ShortcutsInhibition::add(ShortcutsInhibitorDelegate *delegate)
{
    compositor::Surface *surface = delegate->inhibitedSurface();
    if (!surface || !m_inhibitors.try_emplace(surface, delegate).second)
        return false;
    surface->addSurfaceObserver(this);
    update();
    return true;
}

void ShortcutsInhibition::remove(ShortcutsInhibitorDelegate *delegate)
{
    if (m_active == delegate)
        m_active = nullptr;
    compositor::Surface *surface = delegate->inhibitedSurface();
    const auto it = m_inhibitors.find(surface);
    if (it == m_inhibitors.end() || it->second != delegate)
        return;
    m_inhibitors.erase(it);
    surface->removeSurfaceObserver(this);
}

void ShortcutsInhibition::update()
{
    ShortcutsInhibitorDelegate *active = nullptr;
    const auto *gate = m_seat->sessionGate();
    if (m_permitted && !m_seat->wasShutdown() && !(gate && gate->restrictsInput())) {
        const auto it = m_inhibitors.find(m_seat->focusedSurface());
        if (it != m_inhibitors.end())
            active = it->second;
    }
    if (active == m_active)
        return;
    if (auto *previous = std::exchange(m_active, active))
        previous->onInhibitorDeactivated();
    if (m_active)
        m_active->onInhibitorActivated();
}

bool ShortcutsInhibition::breakByUserAction()
{
    if (!m_active)
        return false;
    m_permitted = false;
    update();
    return true;
}

void ShortcutsInhibition::onPointerPressed(compositor::Surface *surface)
{
    if (m_permitted || !surface)
        return;
    const auto it = m_inhibitors.find(m_seat->focusedSurface());
    if (it == m_inhibitors.end() || rootSurface(it->first) != rootSurface(surface))
        return;
    m_permitted = true;
    update();
}

void ShortcutsInhibition::onSurfaceDestroying(compositor::Surface *surface)
{
    const auto it = m_inhibitors.find(surface);
    if (it == m_inhibitors.end())
        return;
    ShortcutsInhibitorDelegate *delegate = it->second;
    m_inhibitors.erase(it);
    surface->removeSurfaceObserver(this);
    if (m_active == delegate) {
        m_active = nullptr;
        delegate->onInhibitorDeactivated();
    }
}

} // namespace Lingmo::Input
