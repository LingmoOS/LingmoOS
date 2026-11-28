// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_view.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <utility>

#include "base/i18n.h"
#include "core/main_loop.h"
#include "shell/common/application_badges.h"
#include "shell/launcher/launcher.h"
#include "shell/launcher/launcher_cell.h"
#include "shell/launcher/launcher_search_field.h"
#include "shell/launcher/launcher_wallpaper.h"
#include "shell/launcher/launcher_folder_view.h"
#include "ui/application.h"
#include "ui/bitmap.h"
#include "ui/drag.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/text_field.h"
#include "ui/window.h"

namespace Lingmo::Shell {

using Clock = std::chrono::steady_clock;
using Lingmo::i18n::tr;

static const ui::Color kWhite {255, 255, 255, 255};
// AllAppsView's highlightMoveDuration, and how long a slide or swap keeps
// the wheel and the reorder dwell waiting by default.
static constexpr std::chrono::milliseconds kSlideDuration {300};
static constexpr std::chrono::milliseconds kSettleDuration {270};
// Flickable's drag threshold, and the speed past which a released flick
// turns the page rather than settling on the nearest one.
static constexpr double kFlickThreshold = 10;
static constexpr double kFlickVelocity = 400;

static ui::Image imageFromBitmap(const ui::Bitmap &bitmap)
{
    if (bitmap.isNull())
        return {};
    ui::Image image(bitmap.width(), bitmap.height());
    uint32_t *out = image.bits();
    for (int y = 0; y < bitmap.height(); ++y) {
        const auto *line = reinterpret_cast<const uint32_t *>(bitmap.constScanLine(y));
        for (int x = 0; x < bitmap.width(); ++x) {
            const uint32_t pixel = line[x];
            const uint32_t alpha = pixel >> 24;
            const auto unpremultiply = [alpha](uint32_t channel) {
                return alpha == 0 ? 0u : std::min(255u, (channel * 255 + alpha / 2) / alpha);
            };
            *out++ = alpha << 24 | unpremultiply((pixel >> 16) & 0xff) << 16
                    | unpremultiply((pixel >> 8) & 0xff) << 8 | unpremultiply(pixel & 0xff);
        }
    }
    return image;
}

static int jsRound(double value)
{
    return int(std::floor(value + 0.5));
}

// Centred: the parent's centre less half the size, that half
// rounded, so a half pixel goes up and to the left.
static double centred(double start, double extent, double size)
{
    return start + jsRound(extent / 2 - jsRound(size / 2));
}

// One page of the grid. It slides by its transform, so a page in motion is
// drawn from its texture.
class LauncherView::Page final : public ui::View
{
public:
    Page() { setName("launcher-page"); }
};

// Behind the pages: a click hides the launcher, a drag pages as the
// ListView's flick did.
class LauncherView::Pages final : public ui::View
{
public:
    explicit Pages(LauncherView *view)
        : m_view(view)
    {
        setName("launcher-pages");
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const ui::PointerEvent &event) override
    {
        if (event.button == ui::MouseButton::Left)
            m_view->backgroundPressed(mapToWindow(event.position));
    }
    void pointerMove(const ui::PointerEvent &event) override
    {
        m_view->backgroundMoved(mapToWindow(event.position));
    }
    void pointerRelease(const ui::PointerEvent &event) override
    {
        if (event.button == ui::MouseButton::Left)
            m_view->backgroundReleased(mapToWindow(event.position));
    }

private:
    LauncherView *m_view;
};


// The page indicator: a row of round dots, the current page's lit.
class LauncherView::Indicator final : public ui::View
{
public:
    // The Basic style's padding around the dots.
    static constexpr double kPadding = 6;

    explicit Indicator(LauncherView *view)
        : m_view(view)
    {
        setName("launcher-indicator");
        setOpacity(0);
    }

    bool acceptsPointer() const override { return m_count > 1; }

    void setDots(int count, int current, bool animate)
    {
        const double opacity = count > 1 ? 1 : 0;
        if (count != m_count) {
            m_count = count;
            m_alphas.assign(size_t(std::max(0, count)), 0.5);
            if (current >= 0 && current < count)
                m_alphas[size_t(current)] = 1;
            m_current = current;
            requestPaint();
        }
        if (opacity != m_opacityTween.target() || (!m_opacityTween.isRunning() && this->opacity() != opacity))
            m_opacityTween.start(animate ? window() : nullptr, this->opacity(), opacity,
                                 std::chrono::milliseconds(200), easing::outCubic,
                                 [this](double value) { setOpacity(value); });
        if (current == m_current)
            return;
        m_current = current;
        const std::vector<double> from = m_alphas;
        m_colorTween.start(animate ? window() : nullptr, 0, 1, std::chrono::milliseconds(160), easing::linear,
                           [this, from](double t) {
                               for (size_t i = 0; i < m_alphas.size() && i < from.size(); ++i) {
                                   const double target = int(i) == m_current ? 1 : 0.5;
                                   m_alphas[i] = from[i] + (target - from[i]) * t;
                               }
                               requestPaint();
                           });
    }

    double dotSize() const { return m_view->m_metrics.indicatorDotSize; }
    double spacing() const { return jsRound(dotSize() * 1.4); }

    // The indicator's own size, padding included, as the row centres it.
    ui::SizeF implicitSize() const
    {
        const int n = std::max(0, m_count);
        return {n * dotSize() + std::max(0, n - 1) * spacing() + 2 * kPadding, dotSize() + 2 * kPadding};
    }

    ui::RectF dotRect(int index) const
    {
        return {kPadding + index * (dotSize() + spacing()), kPadding, dotSize(), dotSize()};
    }

protected:
    void paint(ui::Painter &painter) override
    {
        for (int i = 0; i < m_count; ++i) {
            const ui::RectF dot = dotRect(i);
            painter.drawSmoothRectangle(dot, dot.width / 2, kWhite.withAlpha(m_alphas[size_t(i)]));
        }
    }

    void pointerPress(const ui::PointerEvent &event) override { pick(event.position); }
    void pointerMove(const ui::PointerEvent &event) override { pick(event.position); }

private:
    // The dot under the pointer, its spacing shared with its neighbours.
    void pick(ui::PointF position)
    {
        for (int i = 0; i < m_count; ++i) {
            const ui::RectF dot = dotRect(i);
            const double half = spacing() / 2;
            if (position.x >= dot.x - half && position.x < dot.right() + half) {
                m_view->goToPage(i);
                return;
            }
        }
    }

    LauncherView *m_view;
    int m_count = 0;
    int m_current = -1;
    std::vector<double> m_alphas;
    LauncherTween m_opacityTween;
    LauncherTween m_colorTween;
};

LauncherView::LauncherView(Launcher &launcher, LauncherWallpaper &wallpaper, ApplicationBadges &badges)
    : m_launcher(launcher)
    , m_wallpaper(wallpaper)
    , m_badges(badges)
    , m_wheelReset(core::mainLoop())
    , m_searchTimer(core::mainLoop())
    , m_reorderTimer(core::mainLoop())
    , m_reorderAnimation(core::mainLoop())
    , m_edgeTimer(core::mainLoop())
    , m_preloadTimer(core::mainLoop())
{
    setName("launcher");
    m_pagesView = addChild<Pages>(this);
    m_search = addChild<LauncherSearchField>();
    m_notFound = addChild<ui::Label>(tr("Not found"));
    m_notFound->setColor(kWhite);
    m_notFound->setVisible(false);
    m_indicator = addChild<Indicator>(this);
    // Keep the folder in the launcher's own surface.  This lets the
    // LauncherFolderView is a full-screen overlay that handles open/close
    // animation, paged scrolling and cell reorder inside the folder.
    m_folderView = addChild<LauncherFolderView>(this, m_launcher.applications(), badges, wallpaper);

    m_wheelReset.setSingleShot(true);
    m_wheelReset.setInterval(std::chrono::milliseconds(250));
    m_wheelReset.onTimeout = [this] { m_wheelAccumulated = 0; };
    m_searchTimer.setSingleShot(true);
    m_searchTimer.setInterval(std::chrono::milliseconds(150));
    m_searchTimer.onTimeout = [this] { applySearch(m_search->text()); };
    m_reorderTimer.setSingleShot(true);
    // Passing through the grid on the way to the dock must not rearrange it.
    m_reorderTimer.setInterval(std::chrono::milliseconds(450));
    m_reorderTimer.onTimeout = [this] {
        if (m_hoveredId.empty() || !m_reorderPending)
            return;
        // Wait out the previous swap's animation.
        if (settling()) {
            m_reorderTimer.start();
            return;
        }
        LauncherCell *source = cell(m_draggedId);
        LauncherCell *target = cell(m_hoveredId);
        if (!source || !target) {
            m_reorderPending = false;
            m_folderDropState = FolderDropState::Dragging;
            return;
        }
        if (m_folderPreviewEligible && !source->isFolder() && !target->isFolder()) {
            m_folderDropState = FolderDropState::FolderPreview;
            target->setFolderPreview(true, source->iconName());
            return;
        }
        if (!source->isFolder() && target->isFolder())
            return;

        m_reorderPending = false;
        m_folderDropState = FolderDropState::Dragging;
        requestMove(target->page() * m_metrics.pageCount + target->index());
    };
    // Only a drag reorder slides the icons; this keeps them sliding a moment
    // after a swap.
    m_reorderAnimation.setSingleShot(true);
    m_reorderAnimation.setInterval(std::chrono::milliseconds(400));
    m_edgeTimer.setInterval(std::chrono::milliseconds(600));
    m_edgeTimer.onTimeout = [this] {
        if (m_edge < 0)
            scrollPreviousPage();
        else if (m_edge > 0)
            scrollNextPage();
    };

    // A burst of model changes asks once.
    m_preloadTimer.setSingleShot(true);
    m_preloadTimer.setInterval(std::chrono::milliseconds(0));
    m_preloadTimer.onTimeout = [this] { preloadIcons(); };

    m_search->onTextChanged = [this] { searchTextChanged(); };
    m_search->onKey = [this](uint32_t key) { return searchKey(key); };

    LauncherModel &model = m_launcher.applications();
    const auto changed = [this] { syncCells(); };
    m_subscriptions.push_back(model.rowsInserted.connect([changed](int, int) { changed(); }));
    m_subscriptions.push_back(model.rowsRemoved.connect([changed](int, int) { changed(); }));
    m_subscriptions.push_back(model.rowMoved.connect([changed](int, int) { changed(); }));
    m_subscriptions.push_back(model.dataChanged.connect([changed](int, int) { changed(); }));
    m_subscriptions.push_back(model.modelReset.connect(changed));
    m_subscriptions.push_back(m_wallpaper.changed.connect([this] {
        updateBackdrop();
        requestPaint();
    }));
    m_subscriptions.push_back(m_badges.changed.connect([this] {
        for (const auto &[id, cell] : m_cells)
            cell->setCount(m_badges.count(id));
        if (m_folderView)
            m_folderView->sync();
    }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
}

LauncherView::~LauncherView()
{
    *m_alive = false;
    m_subscriptions.clear();
    m_menu.reset();
    m_scroller.reset();
}

LauncherMetrics LauncherView::computeMetrics() const
{
    return LauncherMetrics::compute(base::Size(int(std::lround(geometry().width)),
                                               int(std::lround(geometry().height))),
                                    m_margins);
}

void LauncherView::setMargins(const LauncherMetrics::Margins &margins)
{
    if (margins == m_margins)
        return;
    m_margins = margins;
    requestLayout();
}

void LauncherView::setCompact(bool compact)
{
    if (compact == m_compact)
        return;
    m_compact = compact;
    requestLayout();
    requestPaint();
}

void LauncherView::setShown(bool shown)
{
    if (shown == m_shown)
        return;
    m_shown = shown;
    m_search->setAnimated(shown);
    if (shown) {
        preloadIcons();
        setFocus();
        return;
    }
    // Reset while hidden, synchronously: a hidden window renders no frames,
    // so anything deferred would play on the next show.
    if (m_menu)
        m_menu->close();
    // Close the folder immediately when the launcher hides.
    if (m_folderView && m_folderView->isOpen()) {
        m_folderView->close();
        if (!m_openFolderId.empty()) {
            if (LauncherCell *folder = cell(m_openFolderId))
                folder->setVisible(true);
            m_openFolderId.clear();
        }
    }
    stopSwipe();
    m_slide.finish();
    m_searchTimer.stop();
    m_search->setText({});
    applySearch({});
    setSelectedIndex(-1);
    m_backgroundPressed = false;
    m_flicking = false;
    endDrag();
    for (const auto &[id, cell] : m_cells)
        cell->finishMove();
    setFocus();
}

bool LauncherView::isSearching() const
{
    return !m_search->text().empty();
}

LauncherCell *LauncherView::cell(const std::string &id) const
{
    const auto it = m_cells.find(id);
    return it != m_cells.end() ? it->second : nullptr;
}

LauncherCell *LauncherView::cellAt(int row) const
{
    const auto &rows = m_launcher.applications().rows();
    if (row < 0 || row >= int(rows.size()))
        return nullptr;
    return cell(rows[size_t(row)].id);
}

ui::RectF LauncherView::cellRect(int row) const
{
    if (row < 0)
        return {};
    const base::Rect rect = m_metrics.cellRect(row % m_metrics.pageCount);
    const double offset = pageX(row / m_metrics.pageCount) - m_contentX;
    return {rect.x() + offset, double(rect.y()), double(rect.width()), double(rect.height())};
}

ui::Menu *LauncherView::menu() const
{
    return m_menu.get();
}

bool LauncherView::isMenuOpen() const
{
    return m_menu && m_menu->isOpen();
}

std::vector<ui::PointF> LauncherView::indicatorDots() const
{
    std::vector<ui::PointF> dots;
    const ui::RectF bounds = m_indicator->geometry();
    for (int i = 0; i < pageTotal(); ++i) {
        const ui::RectF dot = m_indicator->dotRect(i);
        dots.push_back({bounds.x + dot.x + dot.width / 2, bounds.y + dot.y + dot.height / 2});
    }
    return dots;
}

bool LauncherView::isNotFoundVisible() const
{
    return m_notFound->isVisible();
}

void LauncherView::layout()
{
    const LauncherMetrics metrics = computeMetrics();
    const bool changed = !(metrics == m_metrics);
    m_metrics = metrics;
    m_pagesView->setGeometry(localRect());
    if (m_folderView) {
        m_folderView->setGeometry(localRect());
        m_folderView->setMetrics(m_metrics);
    }

    const base::Rect row = m_metrics.searchRow;
    m_search->setMetrics(m_metrics);
    m_search->setGeometry({centred(row.x(), row.width(), m_metrics.searchFieldWidth),
                           centred(row.y(), row.height(), m_metrics.searchFieldHeight),
                           double(m_metrics.searchFieldWidth), double(m_metrics.searchFieldHeight)});

    m_notFound->setFontSize(m_metrics.notFoundPixelSize);
    const ui::SizeF notFound = m_notFound->sizeHint();
    const base::Rect grid = m_metrics.gridArea;
    m_notFound->setGeometry({centred(grid.x(), grid.width(), notFound.width),
                             centred(grid.y(), grid.height(), notFound.height),
                             notFound.width, notFound.height});

    if (changed)
        syncCells();
    else
        placePages();

    const ui::SizeF indicator = m_indicator->implicitSize();
    const base::Rect dots = m_metrics.indicatorRow;
    m_indicator->setGeometry({centred(dots.x(), dots.width(), indicator.width),
                              centred(dots.y(), dots.height(), indicator.height),
                              indicator.width, indicator.height});
}

void LauncherView::paint(ui::Painter &painter)
{
    const ui::RectF bounds = localRect();
    if (m_compact) {
        // In efficient mode the launcher covers the full screen, same as
        // normal mode.  Draw the wallpaper (or dim it) exactly as normal mode
        // does — no rounded rect overlay that obscures the glass or causes
        // flicker when paging.
    }
    const bool folderOpen = m_folderView && m_folderView->isOpen();
    const ui::RectF panel = folderOpen ? m_folderView->panelRect() : ui::RectF();
    const auto drawBackground = [this, &painter, &bounds](const ui::RectF &clip) {
        painter.save();
        painter.clip(clip);
        if (!m_wallpaper.isPicture())
            painter.fillRect(bounds, m_wallpaper.color());
        else {
            // Fill with the average wallpaper colour first so grab() never
            // returns a fully-black bitmap when the image hasn't decoded yet.
            if (m_wallpaper.image().isNull())
                painter.fillRect(bounds, m_wallpaper.backdropColor());
            else
                painter.drawImage(m_wallpaper.image(), bounds);
            // Dimmer on top — also covers the transitional colour fill.
            painter.fillRect(bounds, ui::Color {0, 0, 0, 102});
        }
        painter.restore();
    };
    if (!folderOpen) {
        drawBackground(bounds);
        return;
    }
    // Leave the folder panel transparent so the compositor's local blur can
    // sample the desktop below this launcher surface.
    const double right = panel.right();
    const double bottom = panel.bottom();
    drawBackground({0, 0, bounds.width, std::max(0.0, panel.y)});
    drawBackground({0, bottom, bounds.width, std::max(0.0, bounds.height - bottom)});
    drawBackground({0, panel.y, std::max(0.0, panel.x), panel.height});
    drawBackground({right, panel.y, std::max(0.0, bounds.width - right), panel.height});
}

void LauncherView::updateBackdrop()
{
    const ui::Color backdrop = m_wallpaper.backdropColor();
    for (const auto &[id, cell] : m_cells)
        cell->setBackdrop(backdrop);
}

// The cell menu is made when it opens: an open one closes. The search
// stays.
void LauncherView::retranslate()
{
    if (m_menu)
        m_menu->close();
    m_search->retranslate();
    m_notFound->setText(tr("Not found"));
    requestLayout();
}

void LauncherView::syncCells()
{
    m_preloadTimer.start();
    if (geometry().width <= 0 || geometry().height <= 0)
        return;
    const LauncherModel &model = m_launcher.applications();
    const auto &rows = model.rows();
    const bool animate = m_shown && (!m_draggedId.empty() || m_reorderAnimation.isActive());

    // Pages are added and removed one by one; their cells stay.
    const int total = rows.empty() ? 0 : m_metrics.pagesFor(int(rows.size()));
    while (int(m_pages.size()) < total) {
        Page *page = m_pagesView->addChild<Page>();
        page->setGeometry(localRect());
        m_pages.push_back(page);
    }

    std::set<std::string> present;
    const ui::Color backdrop = m_wallpaper.backdropColor();
    for (size_t row = 0; row < rows.size(); ++row) {
        const LauncherItem &item = rows[row];
        present.insert(item.id);
        const int pageIndex = int(row) / m_metrics.pageCount;
        const int index = int(row) % m_metrics.pageCount;
        Page *page = m_pages[size_t(pageIndex)];
        LauncherCell *cell = this->cell(item.id);
        bool samePage = false;
        if (!cell) {
            cell = page->addChild<LauncherCell>(this, item.id);
            m_cells[item.id] = cell;
        } else if (cell->parent() != page) {
            // Across pages a cell is a new one there, as a delegate is.
            cell->finishMove();
            std::unique_ptr<ui::View> moved = cell->parent()->takeChild(cell);
            page->addChild(std::move(moved));
        } else {
            samePage = cell->isVisible();
        }
        cell->setVisible(item.id != m_openFolderId || !m_folderView || !m_folderView->isOpen());
        cell->setItem(item);
        cell->setCount(m_badges.count(item.id));
        cell->setMetrics(m_metrics);
        cell->setBackdrop(backdrop);
        cell->setSlot(pageIndex, index);
        cell->setDragging(item.id == m_draggedId);
        const base::Rect rect = m_metrics.cellRect(index);
        cell->moveTo({double(rect.x()), double(rect.y())}, animate && samePage);
    }

    // Cells of applications gone go on the next turn: one may be handling
    // the press that removed it. Those a search leaves out only hide.
    std::set<std::string> all;
    for (const LauncherItem &item : model.allItems())
        all.insert(item.id);
    for (auto it = m_cells.begin(); it != m_cells.end();) {
        if (present.count(it->first)) {
            ++it;
            continue;
        }
        LauncherCell *cell = it->second;
        if (all.count(it->first)) {
            cell->setVisible(false);
            ++it;
            continue;
        }
        retire(cell->parent()->takeChild(cell));
        it = m_cells.erase(it);
    }

    while (int(m_pages.size()) > total) {
        Page *page = m_pages.back();
        m_pages.pop_back();
        // Hidden cells move to the first page, which stays while any exist.
        for (const auto &[id, cell] : m_cells) {
            if (cell->parent() == page && !m_pages.empty())
                m_pages.front()->addChild(page->takeChild(cell));
        }
        if (m_pages.empty()) {
            for (const auto &[id, cell] : m_cells) {
                if (cell->parent() == page)
                    m_pagesView->addChild(page->takeChild(cell));
            }
        }
        retire(m_pagesView->takeChild(page));
    }
    for (const auto &[id, cell] : m_cells) {
        // Cells parked on the pages view while there are no pages go back to
        // a page as soon as one exists.
        if (cell->parent() == m_pagesView && !m_pages.empty())
            m_pages.front()->addChild(m_pagesView->takeChild(cell));
    }

    syncPages();
    if (m_selectedIndex >= int(rows.size()))
        setSelectedIndex(int(rows.size()) - 1);
    else
        updateSelection();
    m_notFound->setVisible(isSearching() && rows.empty());
    // Keep the open folder in sync when the model changes (item added,
    // removed, renamed or badge count updated).
    if (m_folderView)
        m_folderView->sync();
    requestPaint();
}

void LauncherView::syncPages()
{
    // An explicit -1 marks the index as cleared, so the view never picks
    // page 0 on its own.
    if (pageTotal() > 0 && m_currentPage < 0) {
        m_currentPage = 0;
        setContentX(pageX(0));
    } else {
        clampCurrentPage();
    }
    if (pageTotal() == 0)
        m_currentPage = -1;
    m_indicator->setDots(pageTotal(), m_currentPage, m_shown);
    const ui::SizeF size = m_indicator->implicitSize();
    const base::Rect dots = m_metrics.indicatorRow;
    m_indicator->setGeometry({centred(dots.x(), dots.width(), size.width),
                              centred(dots.y(), dots.height(), size.height), size.width,
                              size.height});
    placePages();
}

void LauncherView::clampCurrentPage()
{
    if (pageTotal() == 0 || m_currentPage < pageTotal())
        return;
    m_slide.stop();
    m_currentPage = pageTotal() - 1;
    setContentX(pageX(m_currentPage));
}

double LauncherView::pageX(int page) const
{
    return page * geometry().width;
}

int LauncherView::nearestPage() const
{
    const double width = geometry().width;
    if (width <= 0 || pageTotal() == 0)
        return 0;
    return std::clamp(int(std::lround(m_contentX / width)), 0, pageTotal() - 1);
}

void LauncherView::setContentX(double x)
{
    if (x == m_contentX)
        return;
    m_contentX = x;
    placePages();
}

void LauncherView::placePages()
{
    const double width = geometry().width;
    for (size_t i = 0; i < m_pages.size(); ++i) {
        Page *page = m_pages[i];
        page->setGeometry(localRect());
        const double offset = std::round((pageX(int(i)) - m_contentX) * 64) / 64;
        page->setVisible(std::abs(offset) < width);
        page->setTransform({{offset, 0}, 1});
    }
}

void LauncherView::startSettle(std::chrono::milliseconds duration)
{
    m_settleUntil = Clock::now() + duration;
}

bool LauncherView::settling() const
{
    return m_swiping || Clock::now() < m_settleUntil;
}

void LauncherView::slideTo(int page)
{
    m_slide.start(m_shown ? window() : nullptr, m_contentX, pageX(page), kSlideDuration, easing::inOutQuad,
                  [this](double x) { setContentX(x); });
}

void LauncherView::goToPage(int page)
{
    stopSwipe();
    if (page < 0 || page >= pageTotal() || page == m_currentPage)
        return;
    m_currentPage = page;
    m_indicator->setDots(pageTotal(), m_currentPage, m_shown);
    startSettle(kSlideDuration);
    slideTo(page);
}

void LauncherView::scrollNextPage()
{
    goToPage(std::max(0, m_currentPage) + 1);
}

void LauncherView::scrollPreviousPage()
{
    goToPage(std::max(0, m_currentPage) - 1);
}

void LauncherView::resetToFirstPage()
{
    stopSwipe();
    if (pageTotal() == 0)
        return;
    m_slide.stop();
    m_currentPage = 0;
    setContentX(pageX(0));
    m_indicator->setDots(pageTotal(), m_currentPage, m_shown);
}

void LauncherView::stopSwipe()
{
    if (m_scroller)
        m_scroller->cancel(false);
    if (m_swiping)
        settleSwipe(nearestPage());
}

void LauncherView::settleSwipe(int page)
{
    m_slide.stop();
    setContentX(pageX(page));
    m_currentPage = pageTotal() > 0 ? page : -1;
    m_swiping = false;
    m_indicator->setDots(pageTotal(), m_currentPage, m_shown);
}

ui::Scroller::Axis LauncherView::horizontalAxis() const
{
    const double width = geometry().width;
    return {0, std::max(0, pageTotal() - 1) * width, width, pageTotal() > 0};
}

ui::Scroller::Axis LauncherView::verticalAxis() const
{
    return {};
}

ui::PointF LauncherView::position() const
{
    return {m_contentX, 0};
}

void LauncherView::applyPosition(ui::PointF position)
{
    setContentX(position.x);
}

void LauncherView::movingChanged(bool moving)
{
    if (moving) {
        m_slide.stop();
        m_swiping = true;
    } else if (m_swiping) {
        settleSwipe(nearestPage());
    }
}

void LauncherView::setSelectedIndex(int index)
{
    if (index == m_selectedIndex)
        return;
    m_selectedIndex = index;
    updateSelection();
}

void LauncherView::updateSelection()
{
    const auto &rows = m_launcher.applications().rows();
    const std::string selected = m_selectedIndex >= 0 && m_selectedIndex < int(rows.size())
            ? rows[size_t(m_selectedIndex)].id
            : std::string();
    for (const auto &[id, cell] : m_cells)
        cell->setSelected(id == selected);
}

void LauncherView::moveSelection(LauncherDirection direction)
{
    const int total = m_launcher.applications().count();
    const int next = moveLauncherSelection(m_selectedIndex, m_currentPage, total, m_metrics.columns,
                                           m_metrics.pageCount, direction);
    if (next < 0)
        return;
    setSelectedIndex(next);
    goToPage(next / m_metrics.pageCount);
}

void LauncherView::launchSelected()
{
    flushSearch();
    LauncherModel &model = m_launcher.applications();
    if (m_selectedIndex >= 0)
        model.launchRow(m_selectedIndex);
    else if (isSearching() && model.count() > 0)
        model.launchRow(0);
}

void LauncherView::applySearch(const std::string &text)
{
    m_launcher.applications().search(text);
    resetToFirstPage();
    setSelectedIndex(!text.empty() && m_launcher.applications().count() > 0 ? 0 : -1);
    m_notFound->setVisible(!text.empty() && m_launcher.applications().count() == 0);
}

void LauncherView::flushSearch()
{
    // Keys typed within the debounce act on the filtered list, not the stale one.
    if (!m_searchTimer.isActive())
        return;
    m_searchTimer.stop();
    applySearch(m_search->text());
}

void LauncherView::searchTextChanged()
{
    // Search mode follows the text at once; the results after the debounce.
    m_notFound->setVisible(isSearching() && m_launcher.applications().count() == 0);
    if (m_search->text().empty()) {
        m_searchTimer.stop();
        applySearch({});
    } else {
        m_searchTimer.start();
    }
}

bool LauncherView::searchKey(uint32_t key)
{
    switch (key) {
    case ui::key::Escape:
        m_launcher.hide();
        return true;
    case ui::key::Up:
        flushSearch();
        moveSelection(LauncherDirection::Up);
        return true;
    case ui::key::Down:
        flushSearch();
        moveSelection(LauncherDirection::Down);
        return true;
    case ui::key::Return:
    case ui::key::KeypadEnter:
        launchSelected();
        return true;
    default:
        return false;
    }
}

bool LauncherView::keyPress(const ui::KeyEvent &event)
{
    if (event.key == ui::key::Escape && m_folderView && m_folderView->isOpen()) {
        closeFolderOverlay();
        return true;
    }
    switch (event.key) {
    case ui::key::Escape:
        m_launcher.hide();
        return true;
    case ui::key::Left:
        moveSelection(LauncherDirection::Left);
        return true;
    case ui::key::Right:
        moveSelection(LauncherDirection::Right);
        return true;
    case ui::key::Up:
        moveSelection(LauncherDirection::Up);
        return true;
    case ui::key::Down:
        moveSelection(LauncherDirection::Down);
        return true;
    case ui::key::Return:
    case ui::key::KeypadEnter:
        launchSelected();
        return true;
    default:
        break;
    }
    // The first letter or digit starts a search. A shortcut such as Ctrl+A
    // must not.
    const uint32_t modifiers = event.modifiers & ~uint32_t(ui::ShiftModifier);
    const uint32_t key = event.key;
    const bool letter = (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z');
    const bool digit = (key >= '0' && key <= '9') || (key >= 0xffb0 && key <= 0xffb9);
    if (modifiers == 0 && !event.text.empty() && (letter || digit)) {
        m_search->focusField();
        m_search->setText(event.text);
        return true;
    }
    return false;
}

bool LauncherView::scroll(const ui::ScrollEvent &event)
{
    if (event.precise || event.stop) {
        // Fingers page only sideways; up and down do nothing.
        if (!m_scroller && window()) {
            m_scroller = std::make_unique<ui::Scroller>(window(), static_cast<ui::Scroller::Target *>(this));
            m_scroller->setPaged(true);
        }
        if (m_scroller && pageTotal() > 0)
            m_scroller->handleScroll(event);
        return true;
    }
    handleWheel(event);
    return true;
}

void LauncherView::handleWheel(const ui::ScrollEvent &event)
{
    // Wheels count down as angleDelta counts up.
    const double delta = event.value120Y != 0 ? -event.value120Y : -event.value120X;
    if (delta == 0)
        return;
    m_wheelReset.start();
    // Nothing pages until the slide has finished: the rest of a quick spin
    // would otherwise flip the next page at once.
    if (settling()) {
        m_wheelAccumulated = 0;
        return;
    }
    if ((m_wheelAccumulated > 0) != (delta > 0))
        m_wheelAccumulated = 0;
    m_wheelAccumulated += delta;
    if (std::abs(m_wheelAccumulated) < 120)
        return;
    m_wheelAccumulated = 0;
    if (delta > 0)
        scrollPreviousPage();
    else
        scrollNextPage();
}

void LauncherView::backgroundPressed(ui::PointF position)
{
    m_backgroundPressed = true;
    m_flicking = false;
    m_pressPosition = position;
    m_pressContentX = m_contentX;
    m_flickSamples.clear();
}

void LauncherView::backgroundMoved(ui::PointF position)
{
    if (!m_backgroundPressed || pageTotal() == 0)
        return;
    const double dx = position.x - m_pressPosition.x;
    if (!m_flicking) {
        if (std::abs(dx) < kFlickThreshold)
            return;
        m_flicking = true;
        stopSwipe();
        m_slide.stop();
        m_swiping = true;
        m_pressContentX = m_contentX + dx;
    }
    // DragOverBounds: past either end the content follows at half the rate.
    double x = m_pressContentX - dx;
    const double maximum = pageX(pageTotal() - 1);
    if (x < 0)
        x /= 2;
    else if (x > maximum)
        x = maximum + (x - maximum) / 2;
    setContentX(x);
    const auto now = Clock::now();
    m_flickSamples.push_back({now, position.x});
    while (m_flickSamples.size() > 2 && now - m_flickSamples.front().first > std::chrono::milliseconds(100))
        m_flickSamples.erase(m_flickSamples.begin());
}

void LauncherView::backgroundReleased(ui::PointF position)
{
    if (!m_backgroundPressed)
        return;
    m_backgroundPressed = false;
    if (!m_flicking) {
        // A background click commits an active folder rename first. The click
        // must not fall through to the launcher's dismissible background.
        if (finishInlineRename() || m_inlineRenameDismissGuard) {
            m_inlineRenameDismissGuard = false;
            return;
        }
        // Anything that is not an icon, the search field or the dots dismisses.
        dismissRequested();
        return;
    }
    m_flicking = false;
    double velocity = 0;
    if (m_flickSamples.size() >= 2) {
        const auto &[t0, x0] = m_flickSamples.front();
        const double seconds = std::chrono::duration<double>(Clock::now() - t0).count();
        if (seconds > 0)
            velocity = (position.x - x0) / seconds;
    }
    // SnapOneItem: at most one page from where the press started.
    int page = nearestPage();
    const int from = std::clamp(int(std::lround(m_pressContentX / std::max(1.0, geometry().width))), 0,
                                std::max(0, pageTotal() - 1));
    if (std::abs(velocity) > kFlickVelocity)
        page = std::clamp(velocity < 0 ? from + 1 : from - 1, 0, pageTotal() - 1);
    page = std::clamp(page, from - 1, from + 1);
    m_swiping = false;
    m_currentPage = page;
    m_indicator->setDots(pageTotal(), m_currentPage, m_shown);
    startSettle(kSlideDuration);
    slideTo(page);
}

void LauncherView::cellClicked(LauncherCell *cell, ui::MouseButton button, ui::PointF windowPosition)
{
    if (button == ui::MouseButton::Left) {
        if (cell->isFolder()) {
            if (m_folderView) {
                m_openFolderId = cell->id();
                cell->setVisible(false);
                const ui::RectF icon = cell->iconRect();
                const ui::PointF origin = cell->mapToWindow(icon.topLeft());
                m_folderView->open(cell->id(), true, {origin.x, origin.y, icon.width, icon.height});
            }
        } else {
            m_launcher.applications().launch(cell->id());
        }
    } else if (button == ui::MouseButton::Right) {
        openMenu(cell, windowPosition);
    }
}

void LauncherView::renameFolderInline(const std::string &folderId, std::string name)
{
    m_launcher.applications().renameFolder(folderId, std::move(name));
}

bool LauncherView::finishInlineRename()
{
    for (const auto &[id, cell] : m_cells) {
        if (!cell->isRenaming())
            continue;
        cell->finishRename();
        return true;
    }
    return false;
}

void LauncherView::inlineRenameFinished()
{
    m_inlineRenameDismissGuard = true;
}

void LauncherView::clearInlineRenameDismissGuardSoon()
{
    std::weak_ptr<bool> alive = m_alive;
    core::mainLoop()->post([this, alive] {
        if (!alive.expired())
            m_inlineRenameDismissGuard = false;
    });
}

void LauncherView::closeFolderOverlay()
{
    if (!m_folderView || !m_folderView->isOpen())
        return;
    m_folderView->dismiss();
}

void LauncherView::openFolderOverlay(const std::string &folderId, ui::RectF from)
{
    if (m_folderView && m_folderView->isOpen())
        closeFolderOverlay();
    if (LauncherCell *folder = cell(folderId))
        folder->setVisible(false);
    m_openFolderId = folderId;
    m_folderView->open(folderId, true, from);
}

void LauncherView::openMenu(LauncherCell *cell, ui::PointF windowPosition)
{
    if (m_menu)
        m_menu->close();
    if (LauncherCell *previous = this->cell(m_menuId))
        previous->setMenuOpen(false);
    const std::string id = cell->id();
    const auto row = std::find_if(m_launcher.applications().rows().begin(),
                                  m_launcher.applications().rows().end(),
                                  [&id](const LauncherItem &item) { return item.id == id; });
    m_menuId = id;
    auto menu = std::make_unique<ui::Menu>();
    Launcher &launcher = m_launcher;
    if (row != m_launcher.applications().rows().end() && row->folder) {
        menu->addItem(tr("Rename Folder"), [this, id] {
            if (LauncherCell *folder = this->cell(id))
                folder->beginRename();
        });
        menu->addItem(tr("Dissolve Folder"), [&launcher, id] { launcher.applications().dissolveFolder(id); });
    } else {
        menu->addItem(tr("Open"), [&launcher, id] { launcher.applications().launch(id); });
        const size_t sendToDock = menu->addItem(tr("Send to dock"), [&launcher, id] { launcher.sendToDock(id); });
        menu->addItem(tr("Send to desktop"), [&launcher, id] { launcher.applications().sendToDesktop(id); });
        const size_t removeFromDock =
                menu->addItem(tr("Remove from dock"), [&launcher, id] { launcher.removeFromDock(id); });
        const bool pinned = m_launcher.isPinnedInDock(id);
        menu->setItemVisible(sendToDock, !pinned);
        menu->setItemVisible(removeFromDock, pinned);
    }
    std::weak_ptr<bool> alive = m_alive;
    menu->onClosed = [this, alive, id] {
        if (alive.expired())
            return;
        if (LauncherCell *cell = this->cell(id))
            cell->setMenuOpen(false);
        if (m_menuId == id)
            m_menuId.clear();
    };
    if (m_availableGeometry)
        menu->setAvailableGeometry(*m_availableGeometry);
    m_menu = std::move(menu);
    m_menu->popup(window(), windowPosition);
    cell->setMenuOpen(m_menu->isOpen());
}

bool LauncherView::startCellDrag(LauncherCell *cell, ui::View *icon)
{
    if (isSearching() || !m_draggedId.empty())
        return false;
    ui::DragSource source;
    source.mimeTypes = {kDesktopIdMimeType};
    source.data = [id = cell->id()](const std::string &) { return id; };
    source.actions = ui::MoveAction;
    const double size = m_metrics.iconSize;
    source.hotSpot = cell->isFolder()
            ? ui::PointF {double(m_metrics.cellWidth) / 2, double(m_metrics.cellPaddingV) + size / 2}
            : ui::PointF {size / 2, size / 2};
    if (ui::Window *window = this->window()) {
        // Capture the whole cell so a folder keeps its material and member
        // icons while it follows the pointer.
        source.iconScale = window->scale();
        source.icon = cell->isFolder()
                ? cell->grab(source.iconScale)
                : ui::themeIconBitmap(cell->iconName().empty() ? "application-x-desktop" : cell->iconName(),
                                      m_metrics.iconSize, source.iconScale);
    }
    std::weak_ptr<bool> alive = m_alive;
    source.onFinished = [this, alive](ui::DropAction action) {
        if (alive.expired())
            return;
        // A drop applies its pending move a loop turn later, after this.
        m_launcher.applications().endReorder(action != ui::NoAction);
        endDrag();
    };
    if (!icon->startDrag(std::move(source)))
        return false;
    m_launcher.applications().beginReorder();
    m_draggedId = cell->id();
    m_folderDropState = FolderDropState::Dragging;
    cell->setDragging(true);
    // Losing the activation to the drag must not hide the launcher.
    m_launcher.setDragging(true);
    return true;
}

void LauncherView::endDrag()
{
    m_reorderTimer.stop();
    m_edgeTimer.stop();
    m_edge = 0;
    if (LauncherCell *old = cell(m_hoveredId)) {
        old->setFolderPreview(false);
        old->setDragHover(false);
    }
    m_hoveredId.clear();
    m_reorderPending = false;
    m_folderPreviewEligible = false;
    m_folderDropState = FolderDropState::Normal;
    if (m_draggedId.empty())
        return;
    const std::string id = std::exchange(m_draggedId, {});
    if (LauncherCell *cell = this->cell(id))
        cell->setDragging(false);
    m_launcher.setDragging(false);
}

void LauncherView::requestMove(int to)
{
    // Captured now: the drag is over by the time a move asked for on drop
    // gets to run.
    m_pendingMoveId = m_draggedId;
    m_pendingMoveTo = to;
    std::weak_ptr<bool> alive = m_alive;
    core::mainLoop()->post([this, alive] {
        if (!alive.expired())
            applyMove();
    });
}

void LauncherView::applyMove()
{
    const std::string id = std::exchange(m_pendingMoveId, {});
    const int to = std::exchange(m_pendingMoveTo, -1);
    if (id.empty() || to < 0)
        return;
    LauncherModel &model = m_launcher.applications();
    const int from = model.findById(id);
    if (from < 0 || from == to)
        return;
    startSettle(kSettleDuration);
    m_reorderAnimation.start();
    model.move(from, to);
}

bool LauncherView::canDrop(const ui::DropOffer &offer) const
{
    // The whole surface takes the drag, so the cursor never says "no drop"
    // between the icons.
    return offer.hasMimeType(kDesktopIdMimeType);
}

void LauncherView::dragEnter(const ui::DragEvent &event)
{
    updateDragTarget(event.position);
}

uint32_t LauncherView::dragUpdate(const ui::DragEvent &event)
{
    updateDragTarget(event.position);
    return ui::MoveAction;
}

void LauncherView::dragLeave()
{
    m_reorderTimer.stop();
    m_reorderPending = false;
    if (LauncherCell *old = cell(m_hoveredId)) {
        old->setFolderPreview(false);
        old->setDragHover(false);
    }
    m_hoveredId.clear();
    m_folderPreviewEligible = false;
    if (!m_draggedId.empty())
        m_folderDropState = m_folderDropState == FolderDropState::FolderPreview
                ? FolderDropState::Cancelled
                : FolderDropState::Dragging;
    m_edgeTimer.stop();
    m_edge = 0;
}

uint32_t LauncherView::drop(const ui::DragEvent &event)
{
    updateDragTarget(event.position);
    LauncherCell *source = cell(m_draggedId);
    LauncherCell *target = cell(m_hoveredId);
    const bool previewCommit = m_folderDropState == FolderDropState::FolderPreview && source && target
            && !source->isFolder() && !target->isFolder();
    const bool addToFolder = source && target && !source->isFolder() && target->isFolder();
    if (m_reorderPending && !m_draggedId.empty() && !m_hoveredId.empty()
        && m_hoveredId != m_draggedId && (previewCommit || addToFolder)) {
        m_reorderTimer.stop();
        if (previewCommit || addToFolder) {
            target->setFolderPreview(false);
            target->setDragHover(false);
        }
        const std::string sourceId = m_draggedId;
        const std::string targetId = m_hoveredId;
        if (m_launcher.applications().makeFolder(m_draggedId, m_hoveredId)) {
            const auto &items = m_launcher.applications().allItems();
            const auto created = std::find_if(items.begin(), items.end(), [&](const LauncherItem &item) {
                if (!item.folder)
                    return false;
                return std::find(item.members.begin(), item.members.end(), sourceId) != item.members.end()
                        && std::find(item.members.begin(), item.members.end(), targetId) != item.members.end();
            });
            if (created != items.end()) {
                if (LauncherCell *folder = cell(created->id))
                    folder->animateFolderCreated();
            }
            m_folderDropState = FolderDropState::FolderCreated;
            m_reorderPending = false;
            m_hoveredId.clear();
            m_folderPreviewEligible = false;
            return ui::MoveAction;
        }
    }
    m_reorderTimer.stop();
    m_edgeTimer.stop();
    m_edge = 0;
    // A swap rebinds the hovered slot, so moving again here would undo the
    // dwell's move.
    if (m_reorderPending && !m_draggedId.empty()) {
        if (const LauncherCell *hovered = cell(m_hoveredId))
            requestMove(hovered->page() * m_metrics.pageCount + hovered->index());
    }
    if (LauncherCell *old = cell(m_hoveredId)) {
        old->setFolderPreview(false);
        old->setDragHover(false);
    }
    if (m_folderDropState == FolderDropState::FolderPreview || m_folderDropState == FolderDropState::Hovering)
        m_folderDropState = FolderDropState::Cancelled;
    m_reorderPending = false;
    m_hoveredId.clear();
    m_folderPreviewEligible = false;
    return ui::MoveAction;
}

void LauncherView::updateDragTarget(ui::PointF position)
{
    // The edge areas, above the cells, page every 600 ms while the drag
    // rests in them.
    int edge = 0;
    bool inEdge = false;
    if (m_shown && !isSearching() && pageTotal() > 1) {
        const double extra = m_metrics.gridArea.width() * (1 - LauncherMetrics::kContentWidthRatio) / 2;
        const double left = m_metrics.content.x() + extra;
        const double right = geometry().width - (m_metrics.content.x() + m_metrics.content.width()) + extra;
        if (position.x < left) {
            inEdge = true;
            edge = m_currentPage > 0 ? -1 : 0;
        } else if (position.x >= geometry().width - right) {
            inEdge = true;
            edge = m_currentPage < pageTotal() - 1 ? 1 : 0;
        }
    }
    if (edge != m_edge) {
        m_edge = edge;
        if (edge)
            m_edgeTimer.start();
        else
            m_edgeTimer.stop();
    }

    // The cell under the drag, by its resting slot on the page shown.
    std::string hovered;
    if (!inEdge && !isSearching() && m_currentPage >= 0) {
        const base::Rect grid = m_metrics.pageGrid();
        const double x = position.x - (pageX(m_currentPage) - m_contentX);
        if (x >= grid.x() && position.y >= grid.y() && x < grid.x() + grid.width()
            && position.y < grid.y() + grid.height()) {
            const int column = int((x - grid.x()) / m_metrics.cellWidth);
            const int row = int((position.y - grid.y()) / m_metrics.cellHeight);
            const int index = m_currentPage * m_metrics.pageCount + row * m_metrics.columns + column;
            const auto &rows = m_launcher.applications().rows();
            if (column < m_metrics.columns && row < m_metrics.rows && index < int(rows.size()))
                hovered = rows[size_t(index)].id;
        }
    }
    if (hovered == m_draggedId)
        hovered.clear();
    if (hovered != m_hoveredId) {
        if (LauncherCell *old = cell(m_hoveredId)) {
            old->setFolderPreview(false);
            old->setDragHover(false);
        }
        m_hoveredId = hovered;
        m_folderPreviewEligible = false;
        if (!m_hoveredId.empty()) {
            if (LauncherCell *target = cell(m_hoveredId)) {
                target->setDragHover(true);
                const ui::PointF local = target->mapFromWindow(mapToWindow(position));
                m_folderPreviewEligible = target->iconRect().contains(local) && !target->isFolder();
            }
            m_folderDropState = FolderDropState::Hovering;
        } else if (!m_draggedId.empty()) {
            m_folderDropState = FolderDropState::Dragging;
        }
        m_reorderPending = !hovered.empty();
        if (m_reorderPending)
            m_reorderTimer.start();
        else
            m_reorderTimer.stop();
    } else if (!m_hoveredId.empty()) {
        if (LauncherCell *target = cell(m_hoveredId)) {
            const ui::PointF local = target->mapFromWindow(mapToWindow(position));
            const bool eligible = target->iconRect().contains(local) && !target->isFolder();
            if (eligible != m_folderPreviewEligible) {
                m_folderPreviewEligible = eligible;
                m_reorderTimer.stop();
                target->setFolderPreview(false);
                // Moving from the icon to the surrounding cell cancels only
                // folder preview. Keep the ordinary reorder dwell alive so
                // the normal displacement animation still happens.
                if (m_reorderPending) {
                    m_folderDropState = FolderDropState::Hovering;
                    m_reorderTimer.start();
                }
            }
        }
    }
}

void LauncherView::preloadIcons()
{
    m_preloadTimer.stop();
    ui::Window *window = this->window();
    if (!window || window->size().width <= 0)
        return;
    // A page whose icons rasterise as it slides in stalls the slide, so the
    // icons rasterise on the pool ahead of the first show and of every page.
    const LauncherMetrics metrics = LauncherMetrics::compute(
            base::Size(int(std::lround(window->size().width)), int(std::lround(window->size().height))),
            m_margins);
    std::vector<std::string> names;
    for (const LauncherItem &item : m_launcher.applications().allItems())
        names.push_back(item.iconName.empty() ? "application-x-desktop" : item.iconName);
    std::sort(names.begin(), names.end());
    names.erase(std::unique(names.begin(), names.end()), names.end());
    const std::string key = std::to_string(metrics.iconSize) + "@" + std::to_string(window->scale());
    if (names.empty() || (names == m_preloadedNames && key == m_preloadedKey))
        return;
    m_preloadedNames = names;
    m_preloadedKey = key;
    std::weak_ptr<bool> alive = m_alive;
    ui::preloadThemeIcons(std::move(names), metrics.iconSize, window->scale(), [this, alive] {
        if (!alive.expired())
            requestPaint();
    });
}

void LauncherView::retire(std::unique_ptr<ui::View> view)
{
    if (!view)
        return;
    m_retired.push_back(std::move(view));
    std::weak_ptr<bool> alive = m_alive;
    core::mainLoop()->post([this, alive] {
        if (!alive.expired())
            m_retired.clear();
    });
}

} // namespace Lingmo::Shell
