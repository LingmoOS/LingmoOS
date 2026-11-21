// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "render/glass.h"
#include "shell/dock/dock_icon.h"
#include "shell/dock/dock_metrics.h"
#include "shell/dock/minimized_windows.h"
#include "ui/bitmap.h"
#include "ui/geometry.h"
#include "shell/shell.h"

namespace Lingmo::ui {
class DropOffer;
class Menu;
class View;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class Dock;
class DockBounce;
class DockIcon;
class DockMagnification;
class DockPresentation;
class DockTooltip;
class ShellWindow;

// The dock: a strip along the
// primary output's edge, the panel centred in it on the compositor's glass,
// with the launcher, the pinned and the running applications, a divider,
// the minimized windows and the trash. The models and org.lingmo.Dock are
// the Dock it is given; this is the view, its window and its input.
class DockWindow : public Component
{
public:
    DockWindow(Shell *shell, std::unique_ptr<Dock> dock);
    ~DockWindow() override;

    void setTaskbarRightWidth(int width);
    Dock &dock() { return *m_dock; }
    ShellWindow *window() const { return m_window.get(); }
    // The panel in the window's coordinates, and on the screen.
    base::Rect panelRect() const { return m_panelRect; }
    base::Rect primaryGeometry() const;
    // The window's geometry for a dock of `thickness` (DockWindow::windowRect).
    base::Rect windowRect(int thickness) const;
    // Between the panel and the screen edge, inside the window.
    int edgeGap() const;
    // The glass the compositor draws behind the panel.
    render::GlassParams glass() const;
    double cornerRadius() const;

    // The items in order along the dock: applications, minimized windows,
    // then the trash. For tests and the window debug service.
    std::vector<DockIcon *> icons() const;
    DockIcon *applicationIcon(const std::string &id) const;
    DockIcon *minimizedIcon(uint64_t window) const;
    DockIcon *trashIcon() const { return m_trash; }
    // The minimize target last sent for a window, in dock coordinates. For tests.
    base::Rect minimizeTarget(uint64_t window) const
    {
        const auto it = m_sentTargets.find(window);
        return it != m_sentTargets.end() ? it->second : base::Rect();
    }
    // The dividers after the pinned and after the running applications,
    // in panel coordinates; empty while absent.
    base::RectF divider() const { return m_divider; }
    base::RectF trailingDivider() const { return m_trailingDivider; }
    DockTooltip *tooltip() const { return m_tooltip.get(); }
    ui::Menu *openMenu() const { return m_menuOpen ? m_menu.get() : nullptr; }

    // What a drag of a dock icon carries: the application's id, for other
    // drop targets too; a minimized window's id, for the dock alone.
    static constexpr const char *kApplicationMime = "application/x-lingmo-dock-app-id";
    static constexpr const char *kWindowMime = "application/x-lingmo-dock-window-id";
    // What the launcher's drags carry: the desktop id.
    static constexpr const char *kDesktopMime = "application/x-lingmo-desktop-id";

    DockPresentation *applicationPresentation() const { return m_applications.get(); }
    DockPresentation *minimizedPresentation() const { return m_minimized.get(); }
    DockMagnification *magnification() const { return m_magnification.get(); }
    DockBounce *bounce() const { return m_bounce.get(); }
    DockBounce *attentionBounce() const { return m_attentionBounce.get(); }
    // A drag of one of the dock's icons is under way.
    bool isDraggingIcon() const { return m_drag.has_value(); }

    // Called by DockIcon; not part of the public API.
    void iconHovered(DockIcon *icon, bool hovered);
    void iconRunningChanged(DockIcon *icon, bool running);
    // A window of the application asked for attention, or was answered.
    void attentionChanged(uint64_t window);
    void iconDragRequested(DockIcon *icon);
    void iconClicked(DockIcon *icon, int button);
    void iconMenuRequested(DockIcon *icon);
    // Hides the tooltip, as leaving the dock does.
    void hideTooltip();

private:
    class Panel;

    // A drag this dock started from one of its icons.
    struct IconDrag {
        DockIcon::Kind kind;
        std::string key;
        uint64_t window = 0;
        bool started = false;
        // A pinned icon held far enough off the dock: unpinned where let go.
        bool removes = false;
        bool removeOnDrop = false;
        // Nothing took it: the compositor slides its icon home.
        bool returning = false;
    };

    // A drag over the dock: ApplicationDropArea, and what the icons do with
    // one hovering them.
    struct DropTarget {
        ui::DropOffer *offer = nullptr;
        // Bumped for every new offer, so late reads of an old one are dropped.
        uint64_t generation = 0;
        bool application = false;
        bool desktop = false;
        bool window = false;
        bool files = false;
        // What the drag carries, once read; the launcher's arrives later.
        std::optional<std::string> appId;
        std::optional<std::string> desktopId;
        ui::PointF position;
        bool inArea = false;
        bool entered = false;
        bool anchored = false;
        double referenceOrigin = 0;
        double lastAlong = 0;
        bool hasLastAlong = false;
        bool sourcePinned = false;
        int slot = -1;
        bool pinned = false;
        DockIcon *hovered = nullptr;
    };

    void rebuild();
    void retranslate();
    void updateMetrics(bool animate);
    void resize(bool animate);
    void layoutItems();
    void updateLayoutStart();
    void updatePanelRect(const base::Rect &rect, bool notify = true);
    void updateStrut();
    void updateGlass();
    void updateMinimizeTargets();
    void configureMagnification();
    int magnificationHeadroom() const;
    base::Rect baseWindowRect(int thickness) const;
    void panelHoverMoved(const ui::PointF &position);
    void panelHoverLeft();
    void setPointerAlong(std::optional<double> along);
    void handOffLanded();
    double pixelRatio() const;
    base::Rect minimizedSlot(int index, int count) const;
    base::Rect applicationSlot(double position) const;
    double place(double position, double rest) const;
    void showTooltip(DockIcon *icon);
    ui::Bitmap iconImage(DockIcon *icon, double *scale) const;
    base::PointF iconCentre(DockIcon *icon) const;
    double distanceOffDock(const base::PointF &position) const;
    void onDragStarted();
    void onDragMoved(const base::PointF &location);
    void onDragEnded();
    void finishIconDrag();
    void dropEnter(ui::DropOffer *offer, const ui::PointF &position);
    uint32_t dropMove(const ui::PointF &position);
    void dropLeave();
    uint32_t dropAt(const ui::PointF &position);
    void resetDropTarget();
    void setDropHovered(DockIcon *icon);
    bool inApplicationArea(const ui::PointF &position) const;
    void enterApplicationArea();
    void anchorApplicationArea();
    void updateDropSlot();
    DockIcon *iconAt(const ui::PointF &position) const;
    void reorderMinimized();
    void retire(DockIcon *icon);
    void popupMenu(DockIcon *icon, std::unique_ptr<ui::Menu> menu);

    int m_taskbarRightWidth = 360;
    Shell *m_shell;
    std::unique_ptr<Dock> m_dock;
    std::unique_ptr<ShellWindow> m_window;
    Panel *m_panel = nullptr;
    std::unique_ptr<DockPresentation> m_applications;
    std::unique_ptr<DockPresentation> m_minimized;
    std::unique_ptr<DockMagnification> m_magnification;
    std::unique_ptr<DockBounce> m_bounce;
    std::unique_ptr<DockBounce> m_attentionBounce;
    // A callback made from inside layoutItems() must not start it again.
    bool m_layingOut = false;
    // The pointer's place along the dock, in the window's coordinates; unset
    // while it is not on the dock.
    std::optional<double> m_pointerAlong;
    std::map<std::string, DockIcon *> m_applicationIcons;
    std::map<uint64_t, DockIcon *> m_minimizedIcons;
    // The last model state of each slot, kept while a removed one leaves.
    std::map<uint64_t, MinimizedWindows::Entry> m_minimizedStates;
    // Leaving icons, drawn above the rest where they last were.
    std::set<DockIcon *> m_leavingIcons;
    DockIcon *m_trash = nullptr;
    DockMetrics m_metrics;
    DrawnDockMetrics m_fromMetrics;
    DrawnDockMetrics m_drawn;
    base::Rect m_windowRect;
    base::Rect m_panelRect;
    // The dark mode the glass was tinted for.
    bool m_glassDark = false;
    base::RectF m_divider;
    base::RectF m_trailingDivider;
    double m_trailingOpacity = 0;
    // Icons of applications and windows gone, until the next loop turn.
    std::vector<std::unique_ptr<ui::View>> m_retired;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);

    // DockLayout: the panel's place along the strip.
    double m_surfaceLength = 0;
    double m_start = 0;
    double m_length = 0;
    double m_minimizedStart = 0;
    double m_restingLength = -1;
    double m_restingSurface = -1;
    double m_fromLength = 0;
    double m_bias = 0;
    double m_fromBias = 0;
    double m_toBias = 0;
    double m_fromEnds = -1;

    std::optional<IconDrag> m_drag;
    // A returned icon is in its slot from the next frame on; the compositor's
    // copy goes once that frame is out.
    bool m_releaseReturn = false;
    DropTarget m_drop;
    // A foreign drag held over a running application raises it.
    base::Timer m_raiseTimer;
    // A minimized window dragged over another swaps with it.
    base::Timer m_reorderTimer;

    std::unique_ptr<DockTooltip> m_tooltip;
    DockIcon *m_tooltipOwner = nullptr;
    base::Timer m_tooltipHide;
    std::unique_ptr<ui::Menu> m_menu;
    DockIcon *m_menuIcon = nullptr;
    bool m_menuOpen = false;

    // Minimized windows whose thumbnail shows now, for the window manager to
    // stop drawing them once that frame is composed.
    std::vector<uint64_t> m_landed;
    std::map<uint64_t, base::Rect> m_sentTargets;
    base::Timer m_strutSettle;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

