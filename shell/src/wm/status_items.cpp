// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/status_items.h"

#include <algorithm>
#include <cmath>

#include "compositor/sub_surface.h"
#include "compositor/surface.h"
#include "wm/shell_surface.h"
#include "wm/window_manager.h"

namespace Lingmo::WM {

StatusItem::StatusItem(StatusItems *items, compositor::Surface *surface, std::string owner, Client client)
    : m_items(items)
    , m_surface(surface)
    , m_owner(std::move(owner))
    , m_client(std::move(client))
{
    m_surface->setHasRole(true);
    m_surface->addSurfaceObserver(this);
    m_items->add(this);
    if (m_items->barSurface()) {
        sendAppearance();
        updateHighlight();
        configure();
    }
}

StatusItem::~StatusItem()
{
    releaseSurface();
    if (m_items)
        m_items->remove(this);
}

void StatusItem::registryGone()
{
    releaseSurface();
    m_items = nullptr;
}

void StatusItem::releaseSurface()
{
    if (!m_surface)
        return;
    dismissPopups();
    m_subSurface.reset();
    m_surface->setHasRole(true);
    m_surface->removeSurfaceObserver(this);
    m_surface = nullptr;
}

void StatusItem::dismissPopups()
{
    ShellSurface *parent = m_items ? popupParent() : nullptr;
    if (!parent || !m_surface)
        return;
    // Each one is told after it lost the item: closing can destroy the rest.
    std::vector<ShellSurface *> popups;
    for (ShellSurface *popup : parent->childPopups()) {
        if (popup->popupAnchorSurface() == m_surface) {
            popup->setPopupAnchorSurface(nullptr);
            popups.push_back(popup);
        }
    }
    for (ShellSurface *popup : popups) {
        const auto &children = parent->childPopups();
        if (std::find(children.begin(), children.end(), popup) != children.end())
            popup->close();
    }
}

void StatusItem::setIdentifier(std::string identifier)
{
    if (identifier == m_identifier)
        return;
    m_identifier = std::move(identifier);
    if (m_items)
        m_items->identifierSet(this);
}

void StatusItem::setHighlightRequested(bool requested)
{
    if (requested == m_highlightRequested)
        return;
    m_highlightRequested = requested;
    updateHighlight();
}

void StatusItem::updateHighlight()
{
    if (!m_items)
        return;
    bool popupOpen = false;
    if (m_surface && m_items->manager()) {
        for (ShellSurface *popup : m_items->manager()->popupGrabs()) {
            if (popup->popupAnchorSurface() == m_surface)
                popupOpen = true;
        }
    }
    const bool highlighted = m_highlightRequested || popupOpen;
    if (highlighted == m_highlighted && m_highlightSent)
        return;
    const bool changed = highlighted != m_highlighted;
    m_highlighted = highlighted;
    m_highlightSent = true;
    if (m_client.highlight)
        m_client.highlight(highlighted);
    if (changed)
        m_items->itemChanged();
}

void StatusItem::sendAppearance()
{
    if (m_items && m_client.appearance)
        m_client.appearance(m_items->darkAppearance(), m_items->foreground());
}

void StatusItem::configure()
{
    if (!m_items || !m_surface || !m_client.configure || m_items->height() <= 0)
        return;
    const uint32_t serial = m_client.configure(m_items->height(), m_items->maxWidth());
    if (serial)
        m_pendingSerials.push_back(serial);
}

bool StatusItem::acknowledgeConfigure(uint32_t serial)
{
    const auto it = std::find(m_pendingSerials.begin(), m_pendingSerials.end(), serial);
    if (it == m_pendingSerials.end())
        return false;
    m_pendingSerials.erase(m_pendingSerials.begin(), it + 1);
    m_configured = true;
    return true;
}

bool StatusItem::isShown() const
{
    return m_items && m_surface && m_configured && m_width > 0
            && (m_items->maxWidth() <= 0 || m_width <= m_items->maxWidth());
}

ShellSurface *StatusItem::popupParent() const
{
    return m_items ? m_items->barShellSurface() : nullptr;
}

bool StatusItem::adoptPopup(ShellSurface *popup, const base::Rect &bounds)
{
    ShellSurface *parent = popupParent();
    if (!m_surface || !isPlaced() || !parent || !popup->setPopup(parent, bounds))
        return false;
    popup->setPopupAnchorSurface(m_surface);
    return true;
}

void StatusItem::place(const base::Point &position)
{
    compositor::Surface *bar = m_items ? m_items->barSurface() : nullptr;
    if (!m_surface || !bar)
        return;
    if (m_subSurface && m_subSurface->parent() != bar)
        m_subSurface.reset();
    if (!m_subSurface) {
        // The role is the item's; the subsurface only puts it in the bar.
        m_surface->setHasRole(false);
        m_subSurface = compositor::SubSurface::create(m_surface, bar);
        if (!m_subSurface) {
            m_surface->setHasRole(true);
            return;
        }
        // Synchronized: a new size shows in the bar's commit that makes room
        // for it, and the items beside it move in the same frame.
        m_subSurface->setCommitBehavior(true);
        m_position = {};
    }
    if (position != m_position) {
        m_position = position;
        m_subSurface->setPosition(position);
    }
    sendPlaced();
}

void StatusItem::unplace()
{
    if (!m_subSurface)
        return;
    dismissPopups();
    m_subSurface.reset();
    if (m_surface)
        m_surface->setHasRole(true);
    sendPlaced();
}

base::Rect StatusItem::screenRect() const
{
    if (!m_items || !isPlaced() || !m_surface)
        return {};
    const base::Point origin = m_items->barOrigin();
    return base::Rect(origin.x() + m_position.x(), origin.y() + m_position.y(), m_width, m_items->height());
}

void StatusItem::sendPlaced()
{
    const base::Rect rect = screenRect();
    if (m_rectSent && rect == m_sentRect)
        return;
    m_rectSent = true;
    m_sentRect = rect;
    if (m_client.placed)
        m_client.placed(rect);
}

void StatusItem::onSurfaceDestroying(compositor::Surface *surface)
{
    if (surface != m_surface)
        return;
    dismissPopups();
    m_subSurface.reset();
    m_surface->removeSurfaceObserver(this);
    m_surface = nullptr;
    if (m_items)
        m_items->itemChanged();
}

void StatusItem::onSurfaceCommit(compositor::Surface *surface)
{
    if (surface != m_surface)
        return;
    // Placed, the commit waits for the bar's; its size is the cached one.
    const base::Size size = isPlaced() ? m_surface->cachedContentSize() : m_surface->contentSize();
    const int width = size.isEmpty() ? 0 : size.width();
    if (width != m_width) {
        m_width = width;
        if (m_items)
            m_items->itemChanged();
        sendPlaced();
    }
    if (isPlaced() && m_items)
        m_items->contentCommitted();
}

StatusItems::StatusItems(WindowManager *manager)
    : m_manager(manager)
{
    if (m_manager) {
        m_popupGrabsChanged = m_manager->popupGrabsChanged.connect([this] {
            for (StatusItem *item : std::vector<StatusItem *>(m_items))
                item->updateHighlight();
        });
    }
}

StatusItems::~StatusItems()
{
    for (StatusItem *item : std::vector<StatusItem *>(m_items))
        item->registryGone();
}

void StatusItems::add(StatusItem *item)
{
    m_items.insert(m_items.begin(), item);
    changed();
}

static std::string placeKey(const StatusItem *item)
{
    return item->owner() + '\n' + item->identifier();
}

void StatusItems::remove(StatusItem *item)
{
    const auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it == m_items.end())
        return;
    if (!item->identifier().empty())
        m_rememberedPlaces[placeKey(item)] = size_t(m_items.end() - it - 1);
    m_items.erase(it);
    changed();
}

void StatusItems::identifierSet(StatusItem *item)
{
    const auto remembered = m_rememberedPlaces.find(placeKey(item));
    if (item->identifier().empty() || remembered == m_rememberedPlaces.end())
        return;
    const auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it == m_items.end())
        return;
    m_items.erase(it);
    const size_t fromRight = std::min(remembered->second, m_items.size());
    m_items.insert(m_items.end() - std::ptrdiff_t(fromRight), item);
    changed();
}

void StatusItems::setBar(compositor::Surface *surface, ShellSurface *shellSurface)
{
    if (surface == m_barSurface && shellSurface == m_barShellSurface)
        return;
    for (StatusItem *item : std::vector<StatusItem *>(m_items))
        item->unplace();
    const bool configure = !m_barSurface && surface;
    m_barSurface = surface;
    m_barShellSurface = shellSurface;
    if (configure) {
        for (StatusItem *item : std::vector<StatusItem *>(m_items)) {
            item->sendAppearance();
            item->updateHighlight();
            item->configure();
        }
    }
    changed();
}

void StatusItems::setBarOrigin(const base::Point &origin)
{
    if (origin == m_barOrigin)
        return;
    m_barOrigin = origin;
    for (StatusItem *item : std::vector<StatusItem *>(m_items))
        item->sendPlaced();
}

void StatusItems::setLimits(int height, int maxWidth)
{
    if (height == m_height && maxWidth == m_maxWidth)
        return;
    m_height = height;
    m_maxWidth = maxWidth;
    if (!m_barSurface)
        return;
    for (StatusItem *item : std::vector<StatusItem *>(m_items))
        item->configure();
    changed();
}

void StatusItems::setAppearance(bool dark, uint32_t foreground)
{
    if (dark == m_dark && foreground == m_foreground)
        return;
    m_dark = dark;
    m_foreground = foreground;
    if (!m_barSurface)
        return;
    for (StatusItem *item : std::vector<StatusItem *>(m_items))
        item->sendAppearance();
}

} // namespace Lingmo::WM
