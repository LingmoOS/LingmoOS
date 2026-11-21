// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <memory>
#include <string>

#include "base/timer.h"
#include "ui/bitmap.h"
#include "ui/image.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Animation;
class ThemeIcon;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class CountBadge;
class DockWindow;

// One slot of the dock (an application, a minimized window or the trash):
// the icon, or a minimized window's thumbnail with its
// application's badge, the application's count, the running dot, and the
// slot's hit area. The view's geometry is that hit area: the dock's whole
// thickness and half the gap to each neighbour.
class DockIcon : public ui::View
{
public:
    enum class Kind {
        Application,
        Minimized,
        Trash,
    };

    DockIcon(DockWindow *dock, Kind kind);
    ~DockIcon() override;

    Kind kind() const { return m_kind; }
    // The application's id, or the minimized window's id as text.
    const std::string &key() const { return m_key; }
    void setKey(std::string key) { m_key = std::move(key); }
    uint64_t windowId() const { return m_windowId; }
    void setWindowId(uint64_t id) { m_windowId = id; }

    // A theme icon's name, or an SVG file for the launcher.
    void setIcon(std::string name, bool file = false);
    const std::string &iconName() const { return m_iconName; }
    void setToolTip(std::string text) { m_toolTip = std::move(text); }
    const std::string &toolTipText() const { return m_toolTip; }
    void setRunning(bool running);
    bool isRunning() const { return m_running; }
    void setActive(bool active);
    bool isActive() const { return m_active; }
    // The running dot: not while the icon appears, only once it is whole.
    bool isDotShown() const { return m_running && m_opacity == 1; }
    // A minimized window's image; null shows the icon.
    void setThumbnail(const ui::Bitmap *thumbnail, double aspect);
    bool hasThumbnail() const { return !m_thumbnail.isNull(); }
    // The badge fades in 140 ms on OutCubic once the window has landed;
    // the first value is taken at once.
    void setBadgeOpacity(double opacity);
    double badgeOpacity() const { return m_badgeOpacity; }
    // The application's count on the icon's corner; 0 for none.
    void setCount(int64_t count);
    CountBadge *countBadge() const { return m_countBadge; }

    // Where the icon is in this view, and how the slot draws it: scaled
    // about the icon's centre, and its opacity.
    void setIconRect(const ui::RectF &rect);
    ui::RectF iconRect() const { return m_iconRect; }
    void setPresentation(double scale, double opacity);
    // The same, moved by `offset` in the view's coordinates: the magnification
    // lifts an icon off the dock's base line, and the launch bounce hops it.
    void setPresentation(double scale, double opacity, const ui::PointF &offset);
    void setPresentationOpacity(double opacity);
    double presentationScale() const { return m_scale; }
    double presentationOpacity() const { return m_opacity; }
    const ui::PointF &presentationOffset() const { return m_presentOffset; }
    // The icon as drawn now: scaled about its centre and moved by the offset.
    ui::RectF presentedIconRect() const;
    // The running dot, in this view.
    void setDotRect(const ui::RectF &rect);

    // Pressed, or holding its open menu: the icon is dimmed, except while
    // it is dragged.
    bool isDimmed() const { return (m_pressed || m_menuOpen) && !m_dragHidden; }
    bool isPointerOver() const { return m_hovered; }
    void setMenuOpen(bool open);
    // Pressing it and moving starts a drag (DockWindow::iconDragRequested).
    void setDraggable(bool draggable) { m_draggable = draggable; }
    // Away as a drag: nothing of it is drawn in its slot.
    void setDragHidden(bool hidden);
    bool isDragHidden() const { return m_dragHidden; }
    // A drag hovers it: the trash dims as a press does.
    void setDropHover(bool hover);
    bool isDropHover() const { return m_dropHover; }
    // Lets go of the press a drag took over: the drag hears the release.
    void releasePress();

    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerMove(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void paint(ui::Painter &painter) override;

private:
    class SvgFace;
    class ThumbnailMask;

    void paintThumbnail(ui::Painter &painter);
    ui::RectF thumbnailRect() const;
    void updateFace();
    void updateMask();
    void applyBadgeOpacity();
    void placeCountBadge();
    // `rect` in this view, drawn as the icon is: scaled about the icon's
    // centre and moved by the presentation offset.
    ui::RectF presentedRect(const ui::RectF &rect) const;

    DockWindow *m_dock;
    const Kind m_kind;
    std::string m_key;
    uint64_t m_windowId = 0;
    std::string m_iconName;
    bool m_iconIsFile = false;
    std::string m_toolTip;
    bool m_running = false;
    bool m_active = false;
    double m_activeProgress = 0;
    std::unique_ptr<ui::Animation> m_activeAnimation;
    // The icon: a theme icon, or the launcher's artwork.
    ui::ThemeIcon *m_themeFace = nullptr;
    SvgFace *m_svgFace = nullptr;
    // The pressed look: the icon's silhouette in black at 0.4 over it.
    ui::ThemeIcon *m_themeMask = nullptr;
    SvgFace *m_svgMask = nullptr;
    // A minimized window's application.
    ui::ThemeIcon *m_badge = nullptr;
    // The pressed look of a thumbnail: its shape and the badge in black at 0.5.
    ThumbnailMask *m_thumbnailMask = nullptr;
    ui::Image m_thumbnail;
    double m_thumbnailAspect = 1;
    double m_badgeOpacity = 1;
    double m_badgeFrom = 1;
    double m_badgeTarget = 1;
    bool m_badgeSet = false;
    std::unique_ptr<ui::Animation> m_badgeFade;
    // Above the icon and its pressed look.
    CountBadge *m_countBadge = nullptr;
    ui::RectF m_iconRect;
    ui::RectF m_dotRect;
    double m_scale = 1;
    double m_opacity = 1;
    ui::PointF m_presentOffset;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_menuOpen = false;
    bool m_draggable = false;
    bool m_dragArmed = false;
    ui::PointF m_pressPoint;
    bool m_dragHidden = false;
    bool m_dropHover = false;
    int m_pressedButton = -1;
    // Holding the left button opens the menu like a right click.
    base::Timer m_holdTimer;
    bool m_held = false;
};

} // namespace Lingmo::Shell

