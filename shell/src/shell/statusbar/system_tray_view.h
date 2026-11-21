// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"
#include "ui/animation.h"
#include "shell/statusbar/bar_item.h"
#include "ui/bitmap.h"
#include "ui/image.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Menu;
class ThemeIcon;
class Window;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class DBusMenu;
class DBusMenuPopup;
class SystemTrayModel;
class TrayItemView;
class OverflowButton;

// The tray: the StatusNotifierItems as 40 px bar items, the first
// registered at the right end. Left click activates, middle click asks for
// the secondary action, right click opens the item's menu (its exported one
// as a FishUI menu, else the item is asked to show its own), as a left click
// does on an item that is only a menu.
class SystemTrayView : public ui::View
{
public:
    struct Host {
        std::function<void()> closePanels;
        std::function<void()> closeMenuBar;
        // Closes the bar's other popups.
        std::function<void()> closePopups;
        std::function<ui::RectF()> availableGeometry;
        // The bar window's top-left on the screen, for the clicks' positions.
        std::function<ui::PointF()> origin;
    };

    // An item's slot: 16 + 12 + 12.
    static constexpr double kItemWidth = 40;

    SystemTrayView(BarContext *bar, SystemTrayModel *model, Host host);
    ~SystemTrayView() override;

    int count() const { return int(m_items.size()); }
    TrayItemView *item(int row) const;
    double preferredWidth() const;

    void closeMenu();
    ui::Menu *openMenu() const;
    // The bar's look changed.
    void barChanged();

    // The width the bar should give it changed.
    std::function<void()> onCountChanged;

    // The item being dragged to another place, or empty.
    const std::string &draggedItem() const { return m_dragged; }

    // Called by TrayItemView; not part of the public API.
    void itemClicked(TrayItemView *item, const ui::PointerEvent &event);
    void startItemDrag(TrayItemView *item);
    void dragEntered(TrayItemView *target);

protected:
    void layout() override;

private:
    void rebuild();
    void toggleMenu(TrayItemView *item, const ui::PointF &position);
    void openContextMenu(TrayItemView *item, const ui::PointF &position);
    void showMenu(TrayItemView *item, DBusMenu *menu);
    void menuClosed();
    void toggleOverflow();
    void closeOverflow();
    int visibleCount() const;
    double slotX(int row) const;

    BarContext *m_bar;
    SystemTrayModel *m_model;
    Host m_host;
    std::vector<TrayItemView *> m_items;
    OverflowButton *m_overflowButton = nullptr;
    std::unique_ptr<ui::Window> m_overflowWindow;
    ui::View *m_overflowRoot = nullptr;
    std::vector<TrayItemView *> m_overflowItems;
    std::unique_ptr<DBusMenuPopup> m_popup;
    // The item whose menu the popup shows, open or not.
    TrayItemView *m_popupItem = nullptr;
    TrayItemView *m_menuItem = nullptr;
    // A menu whose first layout is on its way.
    base::Timer m_menuWait;
    base::Subscription m_menuLoaded;
    std::vector<base::Subscription> m_subscriptions;
    std::string m_dragged;
    // moveDisplaced: the items slide to their new places, 200 ms InOutQuad.
    std::unique_ptr<ui::Animation> m_moveAnimation;
    std::map<TrayItemView *, std::pair<double, double>> m_moves;
    double m_moveProgress = 1;
    bool m_animateMoves = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

// One tray item: the item's icon, single-colour ones in the bar's colour.
class TrayItemView : public BarItem
{
public:
    TrayItemView(BarContext *bar, SystemTrayView *tray, std::string id);
    ~TrayItemView() override;

    const std::string &itemId() const { return m_id; }

    // The model's icon: a theme name, else the item's pictures, else an
    // application's icon (name or file), else application-x-desktop.
    void setIcon(const std::string &name, const std::vector<ui::Bitmap> &pixmaps,
                 const std::string &fallback);
    // The bar's look changed.
    void updateLook();
    // The icon as a drag shows it, at a scale.
    ui::Bitmap dragImage(double scale) const;
    // Hidden while it is being dragged.
    void setIconHidden(bool hidden);

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;
    bool canDrop(const ui::DropOffer &offer) const override;
    void dragEnter(const ui::DragEvent &event) override;
    uint32_t dragUpdate(const ui::DragEvent &event) override;
    uint32_t drop(const ui::DragEvent &event) override;

private:
    void setMonochrome(bool monochrome);
    void updatePixmapImage();

    SystemTrayView *m_tray;
    std::string m_id;
    std::string m_name;
    std::string m_file;
    std::vector<ui::Bitmap> m_pixmaps;
    bool m_monochrome = false;
    ui::ThemeIcon *m_themeIcon = nullptr;
    ui::Image m_image;
    // The picture's silhouette in the bar's colour, for a single-colour one.
    ui::Image m_tinted;
    ui::Color m_tintedColor;
    base::Job m_probe;
    std::string m_probed;
    bool m_iconHidden = false;
};

} // namespace Lingmo::Shell

