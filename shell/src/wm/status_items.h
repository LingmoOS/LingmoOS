// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_WM_STATUS_ITEMS_H_
#define LINGMO_WM_STATUS_ITEMS_H_

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "compositor/surface_observer.h"

namespace Lingmo::Compositor {
class SubSurface;
class Surface;
} // namespace Lingmo::Compositor

namespace Lingmo::WM {

class ShellSurface;
class StatusItems;
class WindowManager;

// A surface an application draws in the status bar, as exo's notification
// surfaces are client surfaces Ash's message centre embeds in its views. The
// bar gives it a place in its row; while placed, the surface is a
// synchronized subsurface of the bar's own surface, so the seat targets it,
// the scene draws it and its client hears its input as for any surface, and
// each of its commits shows with the bar's next one.
class StatusItem : public compositor::SurfaceObserver
{
public:
    // What the item says to its client; the protocol layer sets these.
    struct Client {
        // The height to draw at and the widest the item may be; returns the
        // serial to acknowledge.
        std::function<uint32_t(int height, int maxWidth)> configure;
        // `foreground` is 0xAARRGGBB with straight alpha.
        std::function<void(bool dark, uint32_t foreground)> appearance;
        std::function<void(bool highlighted)> highlight;
        // Global logical; empty while the item has no place.
        std::function<void(const base::Rect &rect)> placed;
    };

    // `owner` names the application, for the places remembered by identifier.
    StatusItem(StatusItems *items, compositor::Surface *surface, std::string owner, Client client);
    StatusItem(const StatusItem &) = delete;
    StatusItem &operator=(const StatusItem &) = delete;
    // Takes the item out of the bar and dismisses its popups.
    ~StatusItem() override;

    // Null once the surface is gone.
    compositor::Surface *surface() const { return m_surface; }
    const std::string &owner() const { return m_owner; }

    const std::string &identifier() const { return m_identifier; }
    void setIdentifier(std::string identifier);

    // The client's set_highlight.
    void setHighlightRequested(bool requested);
    // What the bar draws: asked for, or a grabbing popup of the item is up.
    bool isHighlighted() const { return m_highlighted; }

    // False for a serial never sent or already acknowledged.
    bool acknowledgeConfigure(uint32_t serial);
    // The client acknowledged a configure.
    bool isConfigured() const { return m_configured; }

    // The committed content's width, logical; 0 without a buffer.
    int width() const { return m_width; }
    // Configured, with contents no wider than the bar allows.
    bool isShown() const;

    // Hangs `popup`, a popup without a parent yet, from the item at `bounds`,
    // the positioner's result relative to the bar: its parent becomes the
    // bar's shell surface. False while the item has no place in the bar.
    bool adoptPopup(ShellSurface *popup, const base::Rect &bounds);
    // The bar's shell surface, the parent of the item's popups.
    ShellSurface *popupParent() const;

    // --- The bar ---

    // Puts the item at `position` in the bar's surface coordinates.
    void place(const base::Point &position);
    void unplace();
    bool isPlaced() const { return m_subSurface != nullptr; }
    base::Point position() const { return m_position; }
    // Global logical; empty while unplaced.
    base::Rect screenRect() const;

    // --- compositor::SurfaceObserver ---

    void onSurfaceDestroying(compositor::Surface *surface) override;
    void onSurfaceCommit(compositor::Surface *surface) override;

    // Called by StatusItems; not part of the public API.
    void registryGone();
    void configure();
    void sendAppearance();
    void updateHighlight();
    void sendPlaced();

private:
    void releaseSurface();
    void dismissPopups();

    StatusItems *m_items;
    compositor::Surface *m_surface;
    std::string m_owner;
    Client m_client;
    std::string m_identifier;
    std::unique_ptr<compositor::SubSurface> m_subSurface;
    base::Point m_position;
    std::deque<uint32_t> m_pendingSerials;
    bool m_configured = false;
    int m_width = 0;
    bool m_highlightRequested = false;
    bool m_highlighted = false;
    bool m_highlightSent = false;
    base::Rect m_sentRect;
    bool m_rectSent = false;
};

// The status items and the bar that shows them. The bar sets its surface,
// size and look here, lays the items out in items() order and places each.
class StatusItems
{
public:
    explicit StatusItems(WindowManager *manager);
    StatusItems(const StatusItems &) = delete;
    StatusItems &operator=(const StatusItems &) = delete;
    // The items outlive it inert, out of any bar.
    ~StatusItems();

    WindowManager *manager() const { return m_manager; }

    // In the bar's order, leftmost first: the newest first.
    const std::vector<StatusItem *> &items() const { return m_items; }

    // --- The bar ---

    // The bar's surface and the shell surface its popups hang from; null when
    // there is no bar. Items keep their places until the bar places them
    // again.
    void setBar(compositor::Surface *surface, ShellSurface *shellSurface);
    compositor::Surface *barSurface() const { return m_barSurface; }
    ShellSurface *barShellSurface() const { return m_barShellSurface; }
    // The bar surface's position on the screen, global logical.
    void setBarOrigin(const base::Point &origin);
    base::Point barOrigin() const { return m_barOrigin; }
    // The height items are drawn at and the widest one may be.
    void setLimits(int height, int maxWidth);
    int height() const { return m_height; }
    int maxWidth() const { return m_maxWidth; }
    void setAppearance(bool dark, uint32_t foreground);
    bool darkAppearance() const { return m_dark; }
    uint32_t foreground() const { return m_foreground; }

    // An item came or went, or one changed its width, shown state or
    // highlight: the bar lays its row out again.
    base::Signal<> changed;
    // A placed item committed: the bar commits too, which shows it.
    base::Signal<> committed;

    // Called by StatusItem; not part of the public API.
    void add(StatusItem *item);
    void remove(StatusItem *item);
    void identifierSet(StatusItem *item);
    void itemChanged() { changed(); }
    void contentCommitted() { committed(); }

private:
    WindowManager *m_manager;
    std::vector<StatusItem *> m_items;
    // How many items stood right of an item when it went, by owner and
    // identifier, for the rest of the session.
    std::map<std::string, size_t> m_rememberedPlaces;
    compositor::Surface *m_barSurface = nullptr;
    ShellSurface *m_barShellSurface = nullptr;
    base::Point m_barOrigin;
    int m_height = 0;
    int m_maxWidth = 0;
    bool m_dark = false;
    uint32_t m_foreground = 0xd9000000;
    base::Subscription m_popupGrabsChanged;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_STATUS_ITEMS_H_
