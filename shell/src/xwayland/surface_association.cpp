// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "surface_association.h"

#include "base/logging.h"

namespace Lingmo::XWayland {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

} // namespace

bool SurfaceAssociation::claimSurfaceSerial(uint64_t serial)
{
    if (serial <= m_lastSurfaceSerial)
        return false;
    m_lastSurfaceSerial = serial;
    return true;
}

bool SurfaceAssociation::addSurface(uint64_t serial, Surface* surface)
{
    if (!serial || !surface)
        return false;
    m_surfaces[serial] = surface;
    match(serial);
    return true;
}

bool SurfaceAssociation::addWindow(uint64_t serial, uint32_t window)
{
    if (!window || serial <= m_lastWindowSerial)
        return false;
    m_lastWindowSerial = serial;
    removeWindow(window);
    m_windows[serial] = window;
    match(serial);
    return true;
}

void SurfaceAssociation::match(uint64_t serial)
{
    auto s = m_surfaces.find(serial);
    auto w = m_windows.find(serial);
    if (s == m_surfaces.end() || w == m_windows.end())
        return;
    auto* surface = s->second;
    auto window = w->second;
    m_surfaces.erase(s);
    m_windows.erase(w);
    if (associated)
        associated(window, surface);
}

void SurfaceAssociation::removeSurface(Surface* surface)
{
    std::erase_if(m_surfaces, [surface](const auto& entry) {
        if (entry.second != surface)
            return false;
        LM_WARNING(logs()) << "Xwayland abandoned surface serial" << entry.first;
        return true;
    });
}

void SurfaceAssociation::removeWindow(uint32_t window)
{
    std::erase_if(m_windows, [window](const auto& entry) {
        if (entry.second != window)
            return false;
        LM_WARNING(logs()) << "Xwayland abandoned window serial" << entry.first;
        return true;
    });
}

} // namespace Lingmo::XWayland
