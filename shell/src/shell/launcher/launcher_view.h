// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "shell/launcher/launcher_metrics.h"
#include "shell/launcher/launcher_tween.h"
#include "ui/scroller.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Label;
class Menu;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class ApplicationBadges;
class Launcher;
class LauncherCell;
class LauncherSearchField;
class LauncherWallpaper;
class FolderOverlay;

// The launcher's content: the blurred wallpaper, the search field, the
// application grid in pages that slide sideways, the page dots and "Not
// found", with its keys, wheel and touchpad paging, drags and menus.
class LauncherView final : public ui::View, private ui::Scroller::Target
{
public:
    // The MIME type an application dragged out of the launcher carries: its
    // desktop id, as UTF-8.
    static constexpr const char *kDesktopIdMimeType = "application/x-lingmo-desktop-id";

    LauncherView(Launcher &launcher, LauncherWallpaper &wallpaper, ApplicationBadges &badges);
    ~LauncherView() override;

    void setMargins(const LauncherMetrics::Margins &margins);
    void setCompact(bool compact);
    // Where menus stay, in window coordinates: the output's work area.
    void setAvailableGeometry(std::optional<ui::RectF> geometry) { m_availableGeometry = geometry; }
    const LauncherMetrics &metrics() const { return m_metrics; }

    // The launcher came up or went: animations play only while it is up,
    // and going resets the search, the selection and the swipe at once.
    void setShown(bool shown);
    // Rasterises the icons on the pool at the size and scale they show at,
    // when they or the window's size or scale changed.
    void preloadIcons();

    // --- State, for the window and tests ---

    int pageTotal() const { return int(m_pages.size()); }
    int currentPage() const { return m_currentPage; }
    double contentX() const { return m_contentX; }
    // An index into the model's rows; -1 for none.
    int selectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);
    bool isSearching() const;
    LauncherSearchField *searchField() const { return m_search; }
    LauncherCell *cell(const std::string &id) const;
    // The cell of a row, null when absent.
    LauncherCell *cellAt(int row) const;
    // Where a row's cell rests, in window coordinates.
    ui::RectF cellRect(int row) const;
    const std::string &draggedId() const { return m_draggedId; }
    ui::Menu *menu() const;
    bool isMenuOpen() const;
    // The dots' centres, in window coordinates.
    std::vector<ui::PointF> indicatorDots() const;
    bool isNotFoundVisible() const;

    // --- What the keys, the wheel and the dots do ---

    void moveSelection(LauncherDirection direction);
    void launchSelected();
    void goToPage(int page);
    void scrollNextPage();
    void scrollPreviousPage();
    // Jumps to the first page, without the slide.
    void resetToFirstPage();
    // Lands at once on the page a swipe was heading for.
    void stopSwipe();

    // Called by LauncherCell; not part of the public API.
    void cellClicked(LauncherCell *cell, ui::MouseButton button, ui::PointF windowPosition);
    void renameFolderInline(const std::string &folderId, std::string name);
    bool startCellDrag(LauncherCell *cell, ui::View *icon);

    // A press on nothing but the page: hide.
    base::Signal<> dismissRequested;
    // The shell surface uses this to apply backdrop blur to the folder card.
    std::function<void(ui::RectF, bool)> folderBlurChanged;
    std::function<void(std::unique_ptr<ui::View>)> folderOverlayCreated;
    std::function<void(bool)> folderOverlayShown;

protected:
    bool acceptsFocus() const override { return true; }
    void layout() override;
    void paint(ui::Painter &painter) override;
    bool keyPress(const ui::KeyEvent &event) override;
    bool scroll(const ui::ScrollEvent &event) override;
    bool canDrop(const ui::DropOffer &offer) const override;
    void dragEnter(const ui::DragEvent &event) override;
    uint32_t dragUpdate(const ui::DragEvent &event) override;
    void dragLeave() override;
    uint32_t drop(const ui::DragEvent &event) override;

private:
    friend class FolderOverlay;
    friend class LauncherCell;
    class Pages;
    class Page;
    class Indicator;
    friend class Pages;
    friend class Indicator;

    // ui::Scroller::Target
    ui::Scroller::Axis horizontalAxis() const override;
    ui::Scroller::Axis verticalAxis() const override;
    ui::PointF position() const override;
    void applyPosition(ui::PointF position) override;
    void movingChanged(bool moving) override;

    LauncherMetrics computeMetrics() const;
    void syncCells();
    void syncPages();
    void placePages();
    void updateSelection();
    void setContentX(double x);
    double pageX(int page) const;
    int nearestPage() const;
    void settleSwipe(int page);
    void slideTo(int page);
    void clampCurrentPage();
    void startSettle(std::chrono::milliseconds duration);
    bool settling() const;
    void applySearch(const std::string &text);
    void flushSearch();
    void searchTextChanged();
    bool searchKey(uint32_t key);
    void handleWheel(const ui::ScrollEvent &event);
    void openMenu(LauncherCell *cell, ui::PointF windowPosition);
    void endDrag();
    void requestMove(int to);
    void applyMove();
    void updateDragTarget(ui::PointF position);
    void updateBackdrop();
    void retranslate();
    void retire(std::unique_ptr<ui::View> view);
    void closeFolderOverlay();

    // Pages: the background's press, flick and click.
    void backgroundPressed(ui::PointF position);
    void backgroundMoved(ui::PointF position);
    void backgroundReleased(ui::PointF position);

    Launcher &m_launcher;
    LauncherWallpaper &m_wallpaper;
    ApplicationBadges &m_badges;
    LauncherMetrics::Margins m_margins;
    LauncherMetrics m_metrics;
    bool m_shown = false;
    bool m_compact = false;

    Pages *m_pagesView = nullptr;
    std::vector<Page *> m_pages;
    std::map<std::string, LauncherCell *> m_cells;
    LauncherSearchField *m_search = nullptr;
    Indicator *m_indicator = nullptr;
    ui::Label *m_notFound = nullptr;
    std::unique_ptr<FolderOverlay> m_folderOverlayOwner;
    FolderOverlay *m_folderOverlay = nullptr;
    LauncherTween m_folderTween;
    double m_folderProgress = 0;
    std::string m_openFolderId;

    int m_currentPage = -1;
    double m_contentX = 0;
    bool m_swiping = false;
    std::chrono::steady_clock::time_point m_settleUntil;
    LauncherTween m_slide;
    std::unique_ptr<ui::Scroller> m_scroller;
    int m_selectedIndex = -1;

    double m_wheelAccumulated = 0;
    base::Timer m_wheelReset;
    base::Timer m_searchTimer;

    // A press on the background, and the flick it may become.
    bool m_backgroundPressed = false;
    bool m_flicking = false;
    ui::PointF m_pressPosition;
    double m_pressContentX = 0;
    std::vector<std::pair<std::chrono::steady_clock::time_point, double>> m_flickSamples;

    std::optional<ui::RectF> m_availableGeometry;
    std::unique_ptr<ui::Menu> m_menu;
    std::string m_menuId;

    // A drag out of a cell, and the reorder it may ask for.
    std::string m_draggedId;
    std::string m_hoveredId;
    bool m_reorderPending = false;
    base::Timer m_reorderTimer;
    base::Timer m_reorderAnimation;
    int m_edge = 0;
    base::Timer m_edgeTimer;
    std::string m_pendingMoveId;
    int m_pendingMoveTo = -1;

    base::Timer m_preloadTimer;
    std::vector<std::string> m_preloadedNames;
    std::string m_preloadedKey;

    std::vector<std::unique_ptr<ui::View>> m_retired;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell

