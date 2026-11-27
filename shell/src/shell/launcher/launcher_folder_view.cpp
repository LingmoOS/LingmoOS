// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_folder_view.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "base/strings.h"
#include "core/main_loop.h"
#include "shell/common/application_badges.h"
#include "shell/launcher/launcher_cell.h"
#include "shell/launcher/launcher_model.h"
#include "shell/launcher/launcher_page_indicator.h"
#include "shell/launcher/launcher_view.h"
#include "shell/launcher/launcher_wallpaper.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/text_field.h"
#include "ui/window.h"

namespace Lingmo::Shell {

static const ui::Color kWhite{255, 255, 255, 255};
static constexpr double kNameScale = 2.0;
static constexpr std::chrono::milliseconds kSlideDuration{300};
static constexpr double kFlickThreshold = 10;
static constexpr double kFlickVelocity = 400;

static int jsRound(double value)
{
    return int(std::floor(value + 0.5));
}

// ---- NameField -------------------------------------------------------

class LauncherFolderView::NameField final : public ui::TextField
{
public:
    explicit NameField(LauncherFolderView *owner)
        : m_owner(owner)
    {
        setFrameVisible(false);
        setHorizontallyCentered(true);
        setTextColor(kWhite);
        setSelectByMouse(true);
        setMaximumLength(64);
    }

    std::string original;

protected:
    void focusOut() override
    {
        ui::TextField::focusOut();
        m_owner->commitName();
    }
    bool keyPress(const ui::KeyEvent &event) override
    {
        switch (event.key) {
        case ui::key::Escape:
            setText(original);
            m_owner->dismiss();
            return true;
        case ui::key::Return:
        case ui::key::KeypadEnter:
            m_owner->commitName();
            m_owner->m_view->setFocus();
            return true;
        default:
            return ui::TextField::keyPress(event);
        }
    }

private:
    LauncherFolderView *m_owner;
};

// ---- Panel -----------------------------------------------------------

class LauncherFolderView::Panel final : public ui::View
{
public:
    explicit Panel(LauncherFolderView *owner)
        : m_owner(owner)
    {
        setName("launcher-folder-panel");
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const ui::PointerEvent &event) override
    {
        if (m_owner->m_name->hasFocus())
            m_owner->m_view->setFocus();
        if (event.button == ui::MouseButton::Left)
            m_owner->panelPressed(event.position);
    }
    void pointerMove(const ui::PointerEvent &event) override
    {
        m_owner->panelMoved(event.position);
    }
    void pointerRelease(const ui::PointerEvent &event) override
    {
        if (event.button == ui::MouseButton::Left)
            m_owner->panelReleased(event.position);
    }

    void paint(ui::Painter &painter) override
    {
        // Frosted white panel.
        painter.drawRoundedRect(localRect(),
                                {double(jsRound(m_owner->m_metrics.iconSize * 0.3)), 0,
                                 ui::Corners::Continuous},
                                kWhite.withAlpha(0.16));
    }

private:
    LauncherFolderView *m_owner;
};

// ---- LauncherFolderView -----------------------------------------------

LauncherFolderView::LauncherFolderView(LauncherView *view, LauncherModel &model,
                                       ApplicationBadges &badges, LauncherWallpaper &wallpaper)
    : m_view(view)
    , m_model(model)
    , m_badges(badges)
    , m_wallpaper(wallpaper)
    , m_wheelReset(core::mainLoop())
{
    setName("launcher-folder");
    // Layered so it draws above the pages' own layers without repainting them.
    setLayered(true);
    setVisible(false);
    m_wallpaperChanged = m_wallpaper.changed.connect([this] { requestPaint(); });
    m_panel = addChild<Panel>(this);
    m_name = addChild<NameField>(this);
    m_grid = m_panel->addChild<ui::View>();
    m_grid->setClipsChildren(true);
    m_indicator = addChild<LauncherPageIndicator>();
    m_indicator->onPick = [this](int page) { goToPage(page); };
    m_wheelReset.setSingleShot(true);
    m_wheelReset.setInterval(std::chrono::milliseconds(250));
    m_wheelReset.onTimeout = [this] { m_wheelAccumulated = 0; };
}

LauncherFolderView::~LauncherFolderView()
{
    *m_alive = false;
    m_scroller.reset();
}

void LauncherFolderView::setMetrics(const LauncherMetrics &metrics)
{
    if (metrics == m_metrics)
        return;
    m_metrics = metrics;
    m_name->setFontSize(nameFontSize());
    m_indicator->setDotSize(metrics.indicatorDotSize);
    for (const auto &[id, cell] : m_cells)
        cell->setMetrics(metrics);
    requestLayout();
}

void LauncherFolderView::open(const std::string &folderId, bool animate, ui::RectF from)
{
    if (!m_folder.empty())
        close();
    m_folder = folderId;
    m_detached = false;
    m_from = from;
    resetPages();
    setVisible(true);
    sync();
    setProgress(0);
    m_fade.start(animate ? window() : nullptr, 0, 1, kOpenDuration, easing::outCubic,
                 [this](double value) { setProgress(value); });
}

void LauncherFolderView::dismiss()
{
    if (!isOpen())
        return;
    const bool focused = m_name->hasFocus();
    commitName();
    m_closing = true;
    setEnabled(false);
    if (focused)
        m_view->setFocus();
    const int serial = ++m_closeSerial;
    std::weak_ptr<bool> alive = m_alive;
    m_fade.start(window(), m_progress, 0, kCloseDuration, easing::outCubic,
                 [this, alive, serial](double value) {
                     setProgress(value);
                     if (value > 0)
                         return;
                     core::mainLoop()->post([this, alive, serial] {
                         if (!alive.expired() && m_closing && serial == m_closeSerial)
                             close();
                     });
                 });
}

void LauncherFolderView::setProgress(double progress)
{
    m_progress = progress;
    setOpacity(progress);
    const ui::RectF panel = panelRect();
    if (panel.width <= 0)
        return;
    // Scale the panel from the origin cell rect towards its full size.
    const double start = m_from.width > 0 ? m_from.width / panel.width : 0.9;
    ui::PointF translation;
    if (m_from.width > 0) {
        translation = {(m_from.x + m_from.width / 2 - (panel.x + panel.width / 2)) * (1 - progress),
                       (m_from.y + m_from.height / 2 - (panel.y + panel.height / 2)) * (1 - progress)};
    }
    m_panel->setTransform({translation, start + (1 - start) * progress});
}

void LauncherFolderView::close()
{
    if (m_folder.empty())
        return;
    const bool focused = m_name->hasFocus();
    commitName();
    m_folder.clear();
    m_detached = false;
    m_order.clear();
    for (const auto &[id, cell] : m_cells)
        retire(cell->parent()->takeChild(cell));
    m_cells.clear();
    m_fade.stop();
    resetPages();
    m_closing = false;
    setEnabled(true);
    m_from = {};
    setProgress(1);
    setVisible(false);
    if (focused)
        m_view->setFocus();
}

void LauncherFolderView::detach()
{
    if (m_folder.empty())
        return;
    commitName();
    m_detached = true;
    setVisible(false);
}

void LauncherFolderView::sync()
{
    if (m_folder.empty() || m_detached)
        return;

    // Look up the folder in the model.
    const int folderRow = m_model.findById(m_folder);
    if (folderRow < 0) {
        close();
        return;
    }
    const std::vector<LauncherItem> members = m_model.folderItems(m_folder);
    const std::vector<LauncherItem> &all = m_model.allItems();
    const auto folderIt = std::find_if(all.begin(), all.end(),
                                       [&](const LauncherItem &i) { return i.id == m_folder; });
    if (folderIt == all.end()) {
        close();
        return;
    }

    if (!m_name->hasFocus()) {
        m_name->setText(folderIt->name);
        m_name->original = folderIt->name;
    }

    std::set<std::string> present;
    m_order.clear();
    const bool animate = m_view->settling();
    for (size_t i = 0; i < members.size(); ++i) {
        const LauncherItem &item = members[i];
        present.insert(item.id);
        m_order.push_back(item.id);
        LauncherCell *c = cell(item.id);
        const bool placed = (c != nullptr);
        if (!c) {
            c = m_grid->addChild<LauncherCell>(m_view, item.id);
            m_cells[item.id] = c;
        }
        c->setItem(item);
        c->setCount(m_badges.count(item.id));
        c->setMetrics(m_metrics);
        c->setSlot(0, int(i));
        c->setDragging(item.id == m_view->draggedId());
        c->moveTo(slotPosition(int(i)), animate && placed);
    }
    for (auto it = m_cells.begin(); it != m_cells.end();) {
        if (present.count(it->first)) {
            ++it;
            continue;
        }
        retire(it->second->parent()->takeChild(it->second));
        it = m_cells.erase(it);
    }
    if (m_currentPage >= pageTotal()) {
        m_slide.stop();
        m_currentPage = pageTotal() - 1;
        setContentX(m_currentPage * panelRect().width);
    }
    m_indicator->setDots(pageTotal(), m_currentPage, animate);
    requestLayout();
    requestPaint();
}

void LauncherFolderView::editName()
{
    if (!isOpen())
        return;
    m_name->setFocus();
    m_name->selectAll();
}

ui::TextField *LauncherFolderView::nameField() const
{
    return m_name;
}

void LauncherFolderView::commitName()
{
    if (m_folder.empty())
        return;
    const std::string name(base::strings::trimmed(m_name->text()));
    if (name.empty()) {
        m_name->setText(m_name->original);
        return;
    }
    m_name->original = name;
    m_model.renameFolder(m_folder, name);
}

LauncherCell *LauncherFolderView::cell(const std::string &id) const
{
    const auto it = m_cells.find(id);
    return it != m_cells.end() ? it->second : nullptr;
}

int LauncherFolderView::columns() const
{
    return std::clamp(m_metrics.columns, 1, kMaximumColumns);
}

int LauncherFolderView::visibleRows() const
{
    const int rows = std::max(1, (int(m_order.size()) + columns() - 1) / columns());
    return std::clamp(rows, 1, std::max(1, m_metrics.rows - 1));
}

int LauncherFolderView::pageSize() const
{
    return columns() * visibleRows();
}

int LauncherFolderView::pageTotal() const
{
    return std::max(1, (int(m_order.size()) + pageSize() - 1) / pageSize());
}

double LauncherFolderView::padding() const
{
    return m_metrics.cellSpacing;
}

double LauncherFolderView::nameGap() const
{
    return 4 * m_metrics.cellSpacing;
}

double LauncherFolderView::indicatorGap() const
{
    return 3 * m_metrics.cellSpacing;
}

double LauncherFolderView::indicatorHeight() const
{
    return pageTotal() > 1
            ? indicatorGap() + m_metrics.indicatorDotSize + LauncherPageIndicator::kPadding
            : 0;
}

int LauncherFolderView::nameFontSize() const
{
    return jsRound(m_metrics.labelPixelSize * kNameScale);
}

double LauncherFolderView::nameHeight() const
{
    return jsRound(nameFontSize() * 1.6);
}

double LauncherFolderView::titleHeight() const
{
    // The name field's own height, and the gap above the panel. A text field
    // has no text baseline to measure from, unlike a label.
    return nameHeight() + nameGap();
}

ui::RectF LauncherFolderView::panelRect() const
{
    const double width = columns() * m_metrics.cellWidth + 2 * padding();
    const double height = visibleRows() * m_metrics.cellHeight + 2 * padding();
    const base::Rect grid = m_metrics.gridArea;
    const double group = titleHeight() + height + indicatorHeight();
    return {double(grid.x() + jsRound((grid.width() - width) / 2)),
            double(grid.y() + jsRound((grid.height() - group) / 2)) + titleHeight(),
            width, height};
}

ui::RectF LauncherFolderView::gridRect() const
{
    const ui::RectF panel = m_panel->geometry();
    return {panel.x + padding(), panel.y + padding(),
            double(columns() * m_metrics.cellWidth),
            double(visibleRows() * m_metrics.cellHeight)};
}

ui::RectF LauncherFolderView::cellRect(int index) const
{
    if (index < 0)
        return {};
    const ui::RectF panel = m_panel->geometry();
    const ui::PointF slot = slotPosition(index);
    return {panel.x + slot.x, panel.y + slot.y,
            double(m_metrics.cellWidth), double(m_metrics.cellHeight)};
}

int LauncherFolderView::indexAt(ui::PointF position) const
{
    const ui::RectF grid = gridRect();
    const double x = position.x + m_contentX - m_currentPage * m_panel->geometry().width;
    if (x < grid.x || x >= grid.right() || position.y < grid.y || position.y >= grid.bottom())
        return -1;
    const int column = int((x - grid.x) / m_metrics.cellWidth);
    const int row = int((position.y - grid.y) / m_metrics.cellHeight);
    const int index = m_currentPage * pageSize() + row * columns() + column;
    return column < columns() && index < int(m_order.size()) ? index : -1;
}

int LauncherFolderView::edgeAt(ui::PointF position) const
{
    const ui::RectF panel = panelRect();
    if (!panel.contains(position))
        return 0;
    const double side = padding() + (m_metrics.cellWidth - m_metrics.iconSize) / 2.0;
    if (position.x < panel.x + side)
        return m_currentPage > 0 ? -1 : 0;
    if (position.x >= panel.right() - side)
        return m_currentPage < pageTotal() - 1 ? 1 : 0;
    return 0;
}

std::vector<ui::PointF> LauncherFolderView::indicatorDots() const
{
    std::vector<ui::PointF> dots;
    const ui::RectF bounds = m_indicator->geometry();
    for (int i = 0; i < pageTotal(); ++i) {
        const ui::RectF dot = m_indicator->dotRect(i);
        dots.push_back({bounds.x + dot.x + dot.width / 2,
                        bounds.y + dot.y + dot.height / 2});
    }
    return dots;
}

void LauncherFolderView::layout()
{
    const ui::RectF panel = panelRect();
    const double nameWidth = std::min(panel.width, double(m_metrics.searchFieldWidth));
    m_panel->setGeometry(panel);
    m_name->setGeometry({panel.x + jsRound((panel.width - nameWidth) / 2),
                         panel.y - titleHeight(), nameWidth, nameHeight()});
    m_grid->setGeometry({0, 0, panel.width, panel.height});
    m_indicator->setDots(pageTotal(), m_currentPage, false);
    const ui::SizeF dots = m_indicator->implicitSize();
    m_indicator->setGeometry(
        {panel.x + jsRound((panel.width - dots.width) / 2),
         panel.bottom() + indicatorGap() - LauncherPageIndicator::kPadding,
         dots.width, dots.height});
    if (!m_slide.isRunning() && !m_swiping && !m_flicking)
        m_contentX = m_currentPage * panel.width;
    placeCells();
    setProgress(m_progress);
}

ui::PointF LauncherFolderView::slotPosition(int index) const
{
    const int page = index / pageSize();
    const int slot = index % pageSize();
    return {padding() + page * panelRect().width - m_contentX
                    + (slot % columns()) * m_metrics.cellWidth,
            padding() + (slot / columns()) * m_metrics.cellHeight};
}

void LauncherFolderView::placeCells()
{
    const double width = panelRect().width;
    for (size_t i = 0; i < m_order.size(); ++i) {
        LauncherCell *c = cell(m_order[i]);
        if (!c)
            continue;
        const ui::PointF slot = slotPosition(int(i));
        c->setVisible(slot.x > -m_metrics.cellWidth && slot.x < width);
        c->moveTo(slot, c->isMoving());
    }
}

void LauncherFolderView::setContentX(double x)
{
    if (x == m_contentX)
        return;
    m_contentX = x;
    placeCells();
}

int LauncherFolderView::nearestPage() const
{
    const double width = panelRect().width;
    if (width <= 0)
        return 0;
    return std::clamp(int(std::lround(m_contentX / width)), 0, pageTotal() - 1);
}

void LauncherFolderView::setCurrentPage(int page, bool slide)
{
    m_currentPage = std::clamp(page, 0, pageTotal() - 1);
    m_indicator->setDots(pageTotal(), m_currentPage, slide);
    const double x = m_currentPage * panelRect().width;
    if (slide) {
        m_slide.start(window(), m_contentX, x, kSlideDuration, easing::inOutQuad,
                      [this](double value) { setContentX(value); });
    } else {
        m_slide.stop();
        setContentX(x);
    }
}

void LauncherFolderView::goToPage(int page)
{
    if (!isOpen() || page < 0 || page >= pageTotal() || page == m_currentPage)
        return;
    if (m_scroller)
        m_scroller->cancel(false);
    m_swiping = false;
    setCurrentPage(page, true);
}

void LauncherFolderView::resetPages()
{
    if (m_scroller)
        m_scroller->cancel(false);
    m_slide.stop();
    m_swiping = false;
    m_panelPressed = false;
    m_flicking = false;
    m_wheelAccumulated = 0;
    m_currentPage = 0;
    m_contentX = 0;
}

ui::Scroller::Axis LauncherFolderView::horizontalAxis() const
{
    const double width = panelRect().width;
    return {0, (pageTotal() - 1) * width, width, pageTotal() > 1};
}

ui::Scroller::Axis LauncherFolderView::verticalAxis() const
{
    return {};
}

ui::PointF LauncherFolderView::position() const
{
    return {m_contentX, 0};
}

void LauncherFolderView::applyPosition(ui::PointF position)
{
    setContentX(position.x);
}

void LauncherFolderView::movingChanged(bool moving)
{
    if (moving) {
        m_slide.stop();
        m_swiping = true;
    } else if (m_swiping) {
        m_swiping = false;
        setCurrentPage(nearestPage(), false);
    }
}

void LauncherFolderView::panelPressed(ui::PointF position)
{
    m_panelPressed = true;
    m_flicking = false;
    m_pressPosition = position;
    m_pressContentX = m_contentX;
    m_flickSamples.clear();
}

void LauncherFolderView::panelMoved(ui::PointF position)
{
    if (!m_panelPressed || pageTotal() < 2)
        return;
    const double dx = position.x - m_pressPosition.x;
    if (!m_flicking) {
        if (std::abs(dx) < kFlickThreshold)
            return;
        m_flicking = true;
        if (m_scroller)
            m_scroller->cancel(false);
        m_slide.stop();
        m_pressContentX = m_contentX + dx;
    }
    double x = m_pressContentX - dx;
    const double maximum = (pageTotal() - 1) * panelRect().width;
    if (x < 0)
        x /= 2;
    else if (x > maximum)
        x = maximum + (x - maximum) / 2;
    setContentX(x);
    const auto now = std::chrono::steady_clock::now();
    m_flickSamples.push_back({now, position.x});
    while (m_flickSamples.size() > 2
           && now - m_flickSamples.front().first > std::chrono::milliseconds(100))
        m_flickSamples.erase(m_flickSamples.begin());
}

void LauncherFolderView::panelReleased(ui::PointF position)
{
    if (!m_panelPressed)
        return;
    m_panelPressed = false;
    if (!m_flicking)
        return;
    m_flicking = false;
    double velocity = 0;
    if (m_flickSamples.size() >= 2) {
        const auto &[t0, x0] = m_flickSamples.front();
        const double seconds =
                std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        if (seconds > 0)
            velocity = (position.x - x0) / seconds;
    }
    int page = nearestPage();
    const int from = std::clamp(int(std::lround(m_pressContentX
                                                / std::max(1.0, panelRect().width))),
                                0, pageTotal() - 1);
    if (std::abs(velocity) > kFlickVelocity)
        page = velocity < 0 ? from + 1 : from - 1;
    setCurrentPage(std::clamp(page, from - 1, from + 1), true);
}

void LauncherFolderView::handleWheel(const ui::ScrollEvent &event)
{
    const double delta = event.value120Y != 0 ? -event.value120Y : -event.value120X;
    if (delta == 0)
        return;
    m_wheelReset.start();
    if (m_slide.isRunning() || m_swiping || m_flicking) {
        m_wheelAccumulated = 0;
        return;
    }
    if ((m_wheelAccumulated > 0) != (delta > 0))
        m_wheelAccumulated = 0;
    m_wheelAccumulated += delta;
    if (std::abs(m_wheelAccumulated) < 120)
        return;
    m_wheelAccumulated = 0;
    goToPage(m_currentPage + (delta > 0 ? -1 : 1));
}

void LauncherFolderView::paint(ui::Painter &painter)
{
    // Draw the wallpaper or its solid fallback, then dim it.
    const ui::RectF bounds = localRect();
    if (!m_wallpaper.isPicture()) {
        painter.fillRect(bounds, m_wallpaper.backdropColor());
    } else if (m_wallpaper.image().isNull()) {
        painter.fillRect(bounds, m_wallpaper.backdropColor());
    } else {
        painter.drawImage(m_wallpaper.image(), bounds);
    }
    painter.fillRect(bounds, ui::Color{0, 0, 0, 38});
}

void LauncherFolderView::pointerPress(const ui::PointerEvent &event)
{
    if (!panelRect().contains(event.position))
        dismiss();
}

bool LauncherFolderView::scroll(const ui::ScrollEvent &event)
{
    if (event.precise || event.stop) {
        if (!m_scroller && window()) {
            m_scroller = std::make_unique<ui::Scroller>(
                    window(), static_cast<ui::Scroller::Target *>(this));
            m_scroller->setPaged(true);
        }
        if (m_scroller && isOpen())
            m_scroller->handleScroll(event);
        return true;
    }
    if (isOpen())
        handleWheel(event);
    return true;
}

void LauncherFolderView::retire(std::unique_ptr<ui::View> view)
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
