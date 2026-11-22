// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_cell.h"

#include <algorithm>
#include <cmath>

#include "shell/common/count_badge.h"
#include "shell/launcher/launcher_view.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/text_field.h"

namespace Lingmo::Shell {

// How far a press moves before it starts a drag.
static constexpr double kStartDragDistance = 10;

static int jsRound(double value)
{
    return int(std::floor(value + 0.5));
}

static double srgbToLinear(double value)
{
    return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
}

// Ink.on(): white over what white keeps 3:1 contrast with, black otherwise.
static ui::Color inkOn(ui::Color background, double alpha)
{
    const double luminance = 0.2126 * srgbToLinear(background.r / 255.0)
            + 0.7152 * srgbToLinear(background.g / 255.0) + 0.0722 * srgbToLinear(background.b / 255.0);
    const bool white = 1.05 / (luminance + 0.05) >= 3.0;
    return (white ? ui::Color {255, 255, 255, 255} : ui::Color {0, 0, 0, 255}).withAlpha(alpha);
}

// FishUI.IconItem, the only part of the cell that takes the pointer.
class LauncherCell::Icon final : public ui::ThemeIcon
{
public:
    explicit Icon(LauncherCell *cell)
        : m_cell(cell)
    {
    }

    bool acceptsPointer() const override { return true; }

protected:
    void pointerPress(const ui::PointerEvent &event) override { m_cell->iconPressed(event); }
    void pointerMove(const ui::PointerEvent &event) override { m_cell->iconMoved(event); }
    void pointerRelease(const ui::PointerEvent &event) override { m_cell->iconReleased(event); }

private:
    LauncherCell *m_cell;
};

LauncherCell::LauncherCell(LauncherView *view, std::string id)
    : m_view(view)
    , m_id(std::move(id))
{
    setName("launcher-cell:" + m_id);
    m_icon = addChild<Icon>(this);
    m_label = addChild<ui::Label>();
    m_label->setColor(ui::Color {255, 255, 255, 255});
    m_label->setWrap(true);
    m_label->setMaximumLineCount(2);
    m_label->setElide(ui::Label::Elide::Right);
    m_label->setTextAlignment(ui::Label::TextAlignment::Center);
    m_renameField = addChild<ui::TextField>();
    m_renameField->setHorizontallyCentered(true);
    m_renameField->setVisible(false);
    m_renameField->onAccepted = [this] { finishRename(); };
    m_renameField->onEditingFinished = [this] { finishRename(); };
}

void LauncherCell::finishRename()
{
    if (!m_renameField->isVisible())
        return;
    m_view->renameFolderInline(m_id, m_renameField->text());
    m_renameField->setVisible(false);
    m_label->setVisible(true);
    requestLayout();
}

LauncherCell::~LauncherCell() = default;

void LauncherCell::setItem(const LauncherItem &item)
{
    m_isFolder = item.folder;
    const std::string iconName = item.folder ? std::string() : item.iconName;
    if (iconName != m_iconName) {
        m_iconName = iconName;
        m_icon->setIconName(m_iconName.empty() ? "application-x-desktop" : m_iconName);
        if (m_mask)
            m_mask->setIconName(m_icon->iconName());
    }
    if (item.name != m_label->text())
        m_label->setText(item.name);
    if (item.newInstalled != m_newInstalled) {
        m_newInstalled = item.newInstalled;
        requestPaint();
    }
    if (item.folder) {
        m_folderIconCount = std::min<size_t>(4, item.memberIcons.size());
        while (m_folderIcons.size() < m_folderIconCount)
            m_folderIcons.push_back(addChild<ui::ThemeIcon>());
        for (size_t i = 0; i < m_folderIcons.size(); ++i) {
            const bool visible = !m_dragging && i < m_folderIconCount;
            m_folderIcons[i]->setVisible(visible);
            if (i < m_folderIconCount)
                m_folderIcons[i]->setIconName(item.memberIcons[i].empty()
                                                      ? "application-x-desktop"
                                                      : item.memberIcons[i]);
        }
        m_icon->setVisible(true);
        m_icon->setOpacity(0);
        requestLayout();
    } else {
        m_icon->setOpacity(1);
        m_icon->setVisible(!m_dragging);
        for (ui::ThemeIcon *icon : m_folderIcons)
            icon->setVisible(false);
    }
}

void LauncherCell::setCount(int64_t count)
{
    if (!m_countBadge) {
        if (count <= 0)
            return;
        m_countBadge = addChild<CountBadge>();
        m_countBadge->setHeightRatio(CountBadge::kLauncherRatio);
    }
    m_countBadge->setCount(count);
    updateCountBadge();
}

void LauncherCell::updateCountBadge()
{
    if (!m_countBadge)
        return;
    raiseChild(m_countBadge);
    m_countBadge->setIconRect(iconRect());
    m_countBadge->setVisible(m_countBadge->count() > 0 && !m_dragging);
}

void LauncherCell::setMetrics(const LauncherMetrics &metrics)
{
    if (metrics == m_metrics)
        return;
    m_metrics = metrics;
    m_icon->setSize(metrics.iconSize);
    for (ui::ThemeIcon *icon : m_folderIcons)
        icon->setSize(metrics.iconSize * 0.38);
    if (m_dropIcon)
        m_dropIcon->setSize(metrics.iconSize * 0.36);
    if (m_mask)
        m_mask->setSize(metrics.iconSize);
    m_label->setFontSize(metrics.labelPixelSize);
    requestLayout();
    requestPaint();
}

void LauncherCell::setSelected(bool selected)
{
    if (selected == m_selected)
        return;
    m_selected = selected;
    requestPaint();
}

void LauncherCell::setDragging(bool dragging)
{
    if (dragging == m_dragging)
        return;
    m_dragging = dragging;
    m_pressed = false;
    m_icon->setVisible(!dragging);
    m_label->setVisible(!dragging);
    for (size_t i = 0; i < m_folderIcons.size(); ++i)
        m_folderIcons[i]->setVisible(!dragging && m_isFolder && i < m_folderIconCount);
    updateMask();
    updateCountBadge();
    requestPaint();
}

void LauncherCell::setMenuOpen(bool open)
{
    if (open == m_menuOpen)
        return;
    m_menuOpen = open;
    updateMask();
}

void LauncherCell::beginRename()
{
    if (!m_isFolder)
        return;
    m_renameField->setText(m_label->text());
    m_label->setVisible(false);
    m_renameField->setVisible(true);
    m_renameField->setFocus();
    requestLayout();
}

void LauncherCell::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || iconRect().contains(event.position))
        return;
    // A folder label is an inline rename target; all other cell whitespace is
    // part of the launcher's dismissible background.
    if (m_isFolder && m_label->geometry().contains(event.position))
        return;
    m_backgroundPress = true;
    m_view->backgroundPressed(mapToWindow(event.position));
}

void LauncherCell::pointerMove(const ui::PointerEvent &event)
{
    if (m_backgroundPress)
        m_view->backgroundMoved(mapToWindow(event.position));
}

void LauncherCell::pointerRelease(const ui::PointerEvent &event)
{
    if (m_backgroundPress) {
        m_backgroundPress = false;
        m_view->backgroundReleased(mapToWindow(event.position));
        return;
    }
    if (event.button == ui::MouseButton::Left && m_isFolder && m_label->geometry().contains(event.position))
        beginRename();
}

void LauncherCell::setDragHover(bool active)
{
    const double target = active ? 1.0 : 0.0;
    if (m_hoverTween.target() == target && (m_hoverProgress == target || m_hoverTween.isRunning()))
        return;
    m_hoverTween.start(window(), m_hoverProgress, target, std::chrono::milliseconds(150), easing::outCubic,
                       [this](double value) {
                           m_hoverProgress = value;
                           requestLayout();
                           requestPaint();
                       });
}

void LauncherCell::setFolderPreview(bool active, const std::string &incomingIconName)
{
    if (active) {
        if (m_isFolder || m_dragging)
            return;
        if (!m_dropIcon) {
            m_dropIcon = addChild<ui::ThemeIcon>();
            m_dropIcon->setSize(m_metrics.iconSize * 0.36);
        }
        m_dropIcon->setIconName(incomingIconName.empty() ? "application-x-desktop" : incomingIconName);
        m_dropIcon->setOpacity(0);
        m_dropIcon->setVisible(true);
        m_folderPreview = true;
    } else if (!m_folderPreview && m_dropProgress == 0 && !m_dropTween.isRunning()) {
        setDragHover(false);
        return;
    }

    const double target = active ? 1.0 : 0.0;
    if (m_dropTween.target() == target && m_dropTween.isRunning())
        return;
    m_dropTween.start(window(), m_dropProgress, target, std::chrono::milliseconds(260),
                      active ? easing::outBack : easing::outCubic,
                      [this](double value) {
                          m_dropProgress = value;
                          if (value <= 0) {
                              m_folderPreview = false;
                              if (m_dropIcon)
                                  m_dropIcon->setVisible(false);
                          }
                          requestLayout();
                          requestPaint();
                      });
}

void LauncherCell::animateFolderCreated()
{
    const double width = m_metrics.cellWidth;
    const double height = m_metrics.cellHeight;
    const auto apply = [this, width, height](double progress) {
        const double scale = 0.82 + 0.18 * progress;
        setTransform({{width * (1 - scale) / 2, height * (1 - scale) / 2}, scale});
        setOpacity(std::clamp(0.58 + 0.42 * progress, 0.0, 1.0));
        if (progress >= 1) {
            setTransform({});
            setOpacity(1);
        }
        requestPaint();
    };
    apply(0);
    m_folderCreateTween.start(window(), 0, 1, std::chrono::milliseconds(240), easing::outCubic, apply);
}

ui::RectF LauncherCell::folderMiniRect(const ui::RectF &icon, int index) const
{
    const double mini = icon.width * 0.36;
    const double gap = icon.width * 0.10;
    const double startX = icon.x + (icon.width - mini * 2 - gap) / 2;
    const double startY = icon.y + (icon.height - mini * 2 - gap) / 2;
    return {startX + (index % 2) * (mini + gap), startY + (index / 2) * (mini + gap), mini, mini};
}

void LauncherCell::setBackdrop(ui::Color color)
{
    if (color == m_backdrop)
        return;
    m_backdrop = color;
    if (m_selected || m_folderPreview || m_isFolder)
        requestPaint();
}

void LauncherCell::updateMask()
{
    // Made on the first press, then only hidden: rasterising it for every
    // icon would cost each icon twice.
    if (isPressDimmed() && !m_mask) {
        m_mask = addChild<ui::ThemeIcon>(m_icon->iconName(), m_metrics.iconSize);
        m_mask->setColor(ui::Color {0, 0, 0, 255});
        m_mask->setOpacity(0.4);
        m_mask->setGeometry(iconRect());
        updateCountBadge();
    }
    if (m_mask)
        m_mask->setVisible(isPressDimmed());
}

void LauncherCell::moveTo(const ui::PointF &position, bool animate)
{
    const ui::RectF current = geometry();
    const auto place = [this](double x, double y) {
        setGeometry({x, y, double(m_metrics.cellWidth), double(m_metrics.cellHeight)});
    };
    if (!animate || !window()) {
        m_moveX.stop();
        m_moveY.stop();
        place(position.x, position.y);
        return;
    }
    if (m_moveX.isRunning() && m_moveX.target() == position.x && m_moveY.target() == position.y)
        return;
    if (current.x == position.x && current.y == position.y && !isMoving())
        return;
    m_moveX.start(window(), current.x, position.x, [this, place](double x) { place(x, geometry().y); });
    m_moveY.start(window(), current.y, position.y, [this, place](double y) { place(geometry().x, y); });
}

void LauncherCell::finishMove()
{
    if (m_moveX.isRunning())
        m_moveX.finish();
    if (m_moveY.isRunning())
        m_moveY.finish();
}

ui::RectF LauncherCell::iconRect() const
{
    const double size = m_metrics.iconSize;
    return {double(jsRound((m_metrics.cellWidth - size) / 2)), double(m_metrics.cellPaddingV), size, size};
}

static ui::RectF labelArea(const LauncherMetrics &metrics)
{
    const double width = metrics.selectionWidth - metrics.cellSpacing;
    return {double(jsRound((metrics.cellWidth - width) / 2)),
            double(metrics.cellPaddingV + metrics.iconSize + metrics.cellSpacing), width,
            double(metrics.labelLineHeight * 2)};
}

ui::RectF LauncherCell::selectionRect() const
{
    if (!m_selected || m_dragging)
        return {};
    const ui::RectF icon = iconRect();
    const ui::RectF label = labelArea(m_metrics);
    // Follows the painted text, not the two-line reservation.
    const double contentHeight = m_label->heightForWidth(label.width);
    const double y = std::max(0.0, icon.y - m_metrics.selectionPadding);
    const double height = std::min(m_metrics.cellHeight - y,
                                   label.y + contentHeight + jsRound(m_metrics.iconSize * 0.14) - y);
    return {double(jsRound((m_metrics.cellWidth - m_metrics.selectionWidth) / 2.0)), y,
            double(m_metrics.selectionWidth), height};
}

ui::RectF LauncherCell::newMarkRect() const
{
    if (!m_newInstalled || m_dragging || m_label->text().empty())
        return {};
    const ui::RectF label = labelArea(m_metrics);
    const double size = std::max(5, jsRound(m_metrics.iconSize * 0.06));
    // The first line's advance and centre, as onLineLaidOut reports them.
    const std::optional<ui::Label::LineGeometry> line = m_label->lineGeometry(0);
    if (!line)
        return {};
    const double lineWidth = line->width;
    const double lineCentre = line->top + line->height / 2;
    const double x = label.x + (label.width - lineWidth) / 2 - size
            - std::max(2, jsRound(m_metrics.cellSpacing / 3.0)) - 2;
    return {x, label.y + lineCentre - size / 2, size, size};
}

void LauncherCell::layout()
{
    const ui::RectF baseIcon = iconRect();
    const auto interpolate = [](const ui::RectF &from, const ui::RectF &to, double progress) {
        return ui::RectF {from.x + (to.x - from.x) * progress,
                          from.y + (to.y - from.y) * progress,
                          from.width + (to.width - from.width) * progress,
                          from.height + (to.height - from.height) * progress};
    };
    const double hoverScale = 1.0 + 0.055 * m_hoverProgress;
    const double hoverSize = baseIcon.width * hoverScale;
    const ui::RectF hoveredIcon {baseIcon.x + (baseIcon.width - hoverSize) / 2,
                                 baseIcon.y + (baseIcon.height - hoverSize) / 2,
                                 hoverSize,
                                 hoverSize};
    ui::RectF icon = hoveredIcon;
    if ((m_folderPreview || m_dropProgress > 0) && m_dropProgress > 0) {
        icon = interpolate(hoveredIcon, folderMiniRect(baseIcon, 0), m_dropProgress);
        if (m_dropIcon) {
            const ui::RectF finalIncoming = folderMiniRect(baseIcon, 1);
            const double startSize = finalIncoming.width * 0.18;
            const ui::RectF startIncoming {baseIcon.x + (baseIcon.width - startSize) / 2,
                                           baseIcon.y + (baseIcon.height - startSize) / 2,
                                           startSize,
                                           startSize};
            m_dropIcon->setGeometry(interpolate(startIncoming, finalIncoming, m_dropProgress));
            m_dropIcon->setOpacity(std::clamp(m_dropProgress, 0.0, 1.0));
        }
    }
    m_icon->setGeometry(icon);
    const double mini = m_metrics.iconSize * 0.36;
    const double gap = m_metrics.iconSize * 0.10;
    const double startX = icon.x + (icon.width - mini * 2 - gap) / 2;
    const double startY = icon.y + (icon.height - mini * 2 - gap) / 2;
    for (size_t i = 0; i < m_folderIcons.size(); ++i) {
        const int column = int(i % 2);
        const int row = int(i / 2);
        m_folderIcons[i]->setGeometry({startX + column * (mini + gap), startY + row * (mini + gap), mini, mini});
    }
    if (m_mask)
        m_mask->setGeometry(icon);
    updateCountBadge();
    const ui::RectF label = labelArea(m_metrics);
    // Lines the label's height cannot hold go before the maximum:
    // with a font taller than 1.3 em the two-line area holds one.
    const double line = m_label->ascent() + m_label->descent();
    m_label->setMaximumLineCount(std::clamp(int(std::floor(label.height / std::max(1.0, line))), 1, 2));
    m_label->setGeometry(label);
    m_renameField->setGeometry(label);
}

void LauncherCell::paint(ui::Painter &painter)
{
    const bool showFolderMaterial = (m_isFolder && !m_dragging) || (m_folderPreview && m_dropProgress > 0);
    if (showFolderMaterial) {
        const ui::RectF icon = iconRect();
        const double radius = std::max(10.0, icon.width * 0.20);
        const ui::Color backdrop = m_backdrop;
        const double previewAlpha = m_isFolder ? 1.0 : std::clamp(m_dropProgress, 0.0, 1.0);
        const ui::Color background {
            uint8_t(std::min(255, int(backdrop.r * 0.55 + 255 * 0.45))),
            uint8_t(std::min(255, int(backdrop.g * 0.55 + 255 * 0.45))),
            uint8_t(std::min(255, int(backdrop.b * 0.55 + 255 * 0.45))),
            uint8_t(188 + (m_isFolder ? 0 : std::lround(42 * previewAlpha)))};
        painter.drawShadow(icon, radius, 8, ui::Color {0, 0, 0, uint8_t(44 * previewAlpha)}, {0, 3});
        painter.drawRoundedRect(icon, {radius, 0, ui::Corners::Circular}, background);
        if (m_folderPreview && m_dropProgress > 0)
            painter.strokeRoundedRect(icon, radius, ui::Color {255, 255, 255, uint8_t(82 * previewAlpha)}, 1);
    }
    const ui::RectF selection = selectionRect();
    if (selection.width > 0)
        painter.drawRoundedRect(selection,
                                {double(jsRound(m_metrics.iconSize * 0.2)), 0, ui::themeCorners()},
                                inkOn(m_backdrop, 0.2));
    const ui::RectF mark = newMarkRect();
    if (mark.width > 0)
        painter.drawSmoothRectangle(mark, mark.height / 2, ui::Theme::current().highlight);
}

void LauncherCell::iconPressed(const ui::PointerEvent &event)
{
    m_pressed = true;
    m_dragArmed = event.button == ui::MouseButton::Left;
    m_pressPoint = event.position;
    updateMask();
}

void LauncherCell::iconMoved(const ui::PointerEvent &event)
{
    if (!m_dragArmed || !m_pressed || m_dragging)
        return;
    const double dx = event.position.x - m_pressPoint.x;
    const double dy = event.position.y - m_pressPoint.y;
    if (dx * dx + dy * dy < kStartDragDistance * kStartDragDistance)
        return;
    m_dragArmed = false;
    m_view->startCellDrag(this, m_icon);
}

void LauncherCell::iconReleased(const ui::PointerEvent &event)
{
    const bool wasPressed = m_pressed;
    m_pressed = false;
    m_dragArmed = false;
    updateMask();
    if (!wasPressed || m_dragging)
        return;
    // A click: the release over the icon it pressed.
    const ui::RectF bounds = m_icon->localRect();
    if (event.position.x < bounds.x || event.position.y < bounds.y
        || event.position.x >= bounds.x + bounds.width || event.position.y >= bounds.y + bounds.height)
        return;
    m_view->cellClicked(this, event.button, m_icon->mapToWindow(event.position));
}

} // namespace Lingmo::Shell
