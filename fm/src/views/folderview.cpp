// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/colors.h"
#include "views/folderview.h"
#include "model/foldermodel.h"
#include "views/renameeditor.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/url.h"
#include "ui/bitmap_painter.h"
#include "ui/clipboard.h"
#include "ui/color_provider.h"
#include "ui/drag.h"
#include "ui/header_backdrop.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

// How far the pointer moves before a press becomes a drag.
static constexpr double kStartDragDistance = 10;
static constexpr double kDragScrollRamp = 180;
static constexpr double kDragScrollMaxSpeed = 560;

// The scrolled area: as tall as the items need, at least the view's height,
// so a press anywhere in the view lands here.
class FolderView::Content : public ui::View
{
public:
    explicit Content(FolderView* view)
        : m_view(view)
    {
    }

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }
    ui::SizeF sizeHint() const override { return {0, heightForWidth(0)}; }
    double heightForWidth(double) const override { return m_view->minimumContentHeight(); }

    void setPointerHandlers()
    {
    }

protected:
    void pointerPress(const ui::PointerEvent& event) override { m_view->pointerPressed(event); }
    void pointerMove(const ui::PointerEvent& event) override { m_view->pointerMoved(event); }
    void pointerRelease(const ui::PointerEvent& event) override { m_view->pointerReleased(event); }
    void pointerEnter() override { m_view->m_pointerInside = true; }
    void pointerLeave() override
    {
        m_view->m_pointerInside = false;
        m_view->pointerLeft();
    }
    // Keys go up to the view itself, the content's parent: not forwarded, or it would hear each twice.
    void layout() override { m_view->updateItems(); }

private:
    FolderView* m_view;
};

// The rubber band: a RoundedRectangle in the base colour,
// faded out over 150 ms once closed.
class FolderView::RubberBand : public ui::View
{
public:
    explicit RubberBand(bool desktop)
        : m_desktop(desktop)
    {
        setName("rubberBand");
    }

    void close(std::function<void()> done)
    {
        if (!window()) {
            done();
            return;
        }
        m_fade = std::make_unique<FrameTicker>(window());
        m_start = 0;
        m_fade->onTick = [this, done](uint64_t now) {
            if (!m_start)
                m_start = now;
            const double t = double(now - m_start) / 150e6;
            setOpacity(1 - style::bezier(0.4, 0, 1, 1, std::min(1.0, t)));
            if (t >= 1) {
                m_fade->stop();
                base::EventLoop::current()->post(done);
            }
        };
        m_fade->start();
    }

protected:
    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = ui::Theme::current();
        // The desktop's band: white over the wallpaper in either mode.
        if (m_desktop) {
            const ui::Color white = ui::Color::rgb(0xffffff);
            painter.drawRoundedRect(localRect(), {theme.smallRadius, 0, ui::Corners::Circular},
                                    style::withAlpha(white, 0.12), 1.0 / painter.scale(), style::withAlpha(white, 0.7));
            return;
        }
        painter.drawRoundedRect(localRect(), {theme.smallRadius, 0, ui::Corners::Circular},
                                ui::color(FmColor::RubberBandFill), 1.0 / painter.scale(),
                                ui::color(FmColor::RubberBandBorder));
    }

private:
    bool m_desktop;
    std::unique_ptr<FrameTicker> m_fade;
    uint64_t m_start = 0;
};

FolderView::FolderView(FolderModel& model)
    : m_model(model)
{
    m_content = setContent<Content>(this);
    m_content->setName("content");
    setClipsChildren(true);

    m_subscriptions.push_back(model.selectionChanged.connect([this] {
        stateChanged();
    }));
    m_subscriptions.push_back(model.rowsChanged.connect([this](const ContentsChange&) {
        if (m_currentIndex >= m_model.count())
            m_currentIndex = -1;
        m_hoveredIndex = -1;
        m_content->requestLayout();
        requestLayout();
        updateItems();
    }));
    m_subscriptions.push_back(model.rowDataChanged.connect([this](int) {
        updateItems();
    }));
    // The dragged items are blank on the desktop until the drag ends, which
    // comes after a drop into the same view has moved them.
    m_subscriptions.push_back(model.draggingChanged.connect([this] {
        updateItems();
    }));
    m_subscriptions.push_back(model.scrollToItem.connect([this](int row) {
        setCurrentIndex(indexOfRow(row));
    }));
    m_subscriptions.push_back(model.requestRename.connect([this] {
        // The model's current item, such as a folder just made, is the one renamed.
        if (const int row = m_model.currentIndex(); row >= 0 && m_model.isSelected(row))
            setCurrentIndex(indexOfRow(row));
        rename();
    }));
}

FolderView::~FolderView()
{
    *m_alive = false;
}

ui::View* FolderView::contentView() const
{
    return m_content;
}

ui::PointF FolderView::toContent(ui::PointF view) const
{
    return m_content->mapFromWindow(mapToWindow(view));
}

ui::PointF FolderView::fromContent(ui::PointF content) const
{
    return mapFromWindow(m_content->mapToWindow(content));
}

void FolderView::setTopInset(double inset)
{
    if (m_topInset == inset)
        return;
    m_topInset = inset;
    setVerticalBarPadding(headerHeight(), m_bottomInset);
    requestLayout();
    m_content->requestLayout();
}

void FolderView::setBottomInset(double inset)
{
    if (m_bottomInset == inset)
        return;
    m_bottomInset = inset;
    setVerticalBarPadding(headerHeight(), m_bottomInset);
    requestLayout();
    m_content->requestLayout();
}

void FolderView::setBackdropBase(std::optional<ui::ColorRecipe> color)
{
    m_backdropBase = color;
    if (m_backdrop)
        m_backdrop->setBaseColor(color);
}

double FolderView::minimumContentHeight() const
{
    return geometry().height;
}

void FolderView::layout()
{
    setVerticalBarPadding(headerHeight(), m_bottomInset);
    ScrollView::layout();
    updateItems();
}

void FolderView::positionChanged()
{
    // A scroll ends a rename; a layout that leaves the offset alone does not.
    const ui::PointF position = scrollPosition();
    if (position.x != m_renamePosition.x || position.y != m_renamePosition.y)
        cancelRename();
    m_renamePosition = position;
    if (m_rubberBand)
        updateRubberBand(m_lastMouse);
    else if (m_pointerInside)
        hover(m_lastMouse);
    updateItems();
}

void FolderView::forceLayout()
{
    layout();
}

void FolderView::setContentY(double y)
{
    setScrollPosition({scrollPosition().x, std::clamp(y, 0.0, maximumContentY())});
}

double FolderView::maximumContentY() const
{
    return std::max(0.0, contentSize().height - geometry().height);
}

void FolderView::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    currentIndexChanged.emit();
    if (index >= 0)
        revealIndex(index);
}

void FolderView::reset()
{
    m_pressActive = false;
    m_currentIndex = -1;
    m_anchorIndex = 0;
    cancelRename();
    m_hoveredIndex = -1;
    m_pressedIndex = -1;
    m_contentPress.reset();
}

void FolderView::retranslate()
{
    updateItems();
    requestLayout();
}

bool FolderView::isRenaming() const
{
    return m_editor != nullptr;
}

void FolderView::rename()
{
    if (m_currentIndex < 0 || m_editor)
        return;
    // What the selection allows now, as the menu would see it.
    m_model.prepareContextMenu();
    if (!m_model.action("rename").enabled)
        return;

    const int row = rowOf(m_currentIndex);
    const FileItemPtr item = m_model.item(row);
    const auto label = labelRect(m_currentIndex);
    if (!item || !label)
        return;

    auto editor = std::make_unique<RenameEditor>(item->name, m_model.fileExtensionBoundary(row), centeredRename());
    m_editor = editor.get();
    m_editor->setFont(labelPixelSize(), labelWeight());
    const int index = m_currentIndex;
    // The row moves if files come or go while the name is typed.
    m_editor->onSubmitted = [this, url = item->url](const std::string& name) {
        if (const int current = m_model.rowForUrl(url); current >= 0)
            m_model.rename(current, name);
    };
    m_editor->onFinished = [this, index] {
        RenameEditor* editor = m_editor;
        m_editor = nullptr;
        m_content->setFocus();
        updateItems();
        // Not from inside the editor's own key or focus handling.
        std::shared_ptr<View> gone = takeChild(editor);
        base::EventLoop::current()->post([gone] {});
        (void)index;
    };
    m_renamePosition = scrollPosition();
    m_editor->setTarget(*label, geometry().width);
    addChild(std::move(editor));
    m_editor->begin();
    updateItems();
}

void FolderView::cancelRename()
{
    if (m_editor)
        m_editor->cancel();
}

void FolderView::selectRange(int from, int to)
{
    m_model.setRangeSelected(rowOf(from), rowOf(to));
}

void FolderView::updateSelection(uint32_t modifiers)
{
    if ((modifiers & ui::ShiftModifier) && (!m_chooser || m_chooserMultiple)) {
        selectRange(m_anchorIndex, m_currentIndex);
    } else {
        m_model.clearSelection();
        m_model.setSelected(rowOf(m_currentIndex));
    }
}

int FolderView::keyboardIndex(bool forward) const
{
    if (m_currentIndex >= 0 && m_model.isSelected(rowOf(m_currentIndex)))
        return m_currentIndex;

    int found = -1;
    for (const int row : m_model.selectedRows()) {
        const int index = indexOfRow(row);
        if (index < 0)
            continue;
        if (found < 0 || (forward ? index > found : index < found))
            found = index;
    }
    return found >= 0 ? found : m_currentIndex;
}

void FolderView::applyRectangleSelection(const std::vector<int>& indices)
{
    // Ctrl toggles what the band covers against the selection it started
    // from, Shift adds to it.
    const int mode = m_ctrlPressed ? 2 : m_shiftPressed ? 1 : 0;
    // Pointer moves far outnumber changes to what the band covers.
    if (mode == m_cachedRectangleSelectionMode && indices == m_cachedRectangleSelection)
        return;
    m_cachedRectangleSelection = indices;
    m_cachedRectangleSelectionMode = mode;

    std::vector<int> rows;
    rows.reserve(indices.size());
    for (const int index : indices)
        rows.push_back(rowOf(index));
    m_model.updateSelection(rows, mode == 2, mode == 1);
}

void FolderView::hover(ui::PointF view)
{
    int hovered = -1;
    if (view.y >= headerHeight()) {
        const ui::PointF content = toContent(view);
        const int index = indexAt(content);
        if (index >= 0 && hitsItem(index, content) && !m_model.isBlank(rowOf(index)))
            hovered = index;
    }
    if (hovered != m_hoveredIndex) {
        m_hoveredIndex = hovered;
        stateChanged();
    }
}

void FolderView::pointerLeft()
{
    if (m_rubberBand)
        return;
    if (m_hoveredIndex >= 0) {
        m_hoveredIndex = -1;
        stateChanged();
    }
}

void FolderView::openContextMenu(ui::PointF view)
{
    // A new folder or file belongs on the cell that was right-clicked.
    if (const int cell = cellAt(toContent(view)); cell >= 0)
        m_model.setNewDocumentPosition(int(std::round(view.x)), int(std::round(view.y)), cell);
    m_model.prepareContextMenu();
    if (onContextMenu)
        onContextMenu(mapToWindow(view));
}

void FolderView::pointerPressed(const ui::PointerEvent& event)
{
    const ui::PointF view = fromContent(event.position);
    m_content->setFocus(ui::FocusReason::Mouse);

    if (m_editor && !m_editor->commit())
        return;

    // Modifiers pressed while another window had focus never reached the keys.
    m_ctrlPressed = event.modifiers & ui::ControlModifier;
    m_shiftPressed = event.modifiers & ui::ShiftModifier;
    if (m_chooser && !m_chooserMultiple)
        m_ctrlPressed = m_shiftPressed = false;

    // Hover goes stale when the wheel scrolls under a still pointer.
    hover(view);

    m_pressActive = true;
    m_press = view;
    m_contentPress = event.position;

    const bool right = event.button == ui::MouseButton::Right;
    if (m_hoveredIndex >= 0) {
        m_pressedIndex = m_hoveredIndex;
        const int row = rowOf(m_hoveredIndex);

        // Moving the current index here would move the anchor of a Shift range.
        if (!m_shiftPressed)
            setCurrentIndex(m_pressedIndex);

        if (m_shiftPressed && m_currentIndex != -1) {
            selectRange(m_anchorIndex, m_hoveredIndex);
        } else {
            // Ctrl keeps the rest of the selection.
            if (!m_ctrlPressed && !m_model.isSelected(row))
                m_model.clearSelection();
            if (m_ctrlPressed)
                m_model.toggleSelected(row);
            else
                m_model.setSelected(row);
        }

        if (right) {
            clearPressState();
            openContextMenu(view);
        }
    } else if (m_chooser) {
        // The chooser's rows take presses; the space under them does nothing.
        clearPressState();
        return;
    } else {
        // A press on empty space clears the selection, unless a band started
        // with a modifier is to build on it.
        if (!m_ctrlPressed && !m_shiftPressed) {
            setCurrentIndex(-1);
            m_model.clearSelection();
        }
        if (right) {
            clearPressState();
            openContextMenu(view);
        }
    }

    if (event.clickCount == 2 && event.button == ui::MouseButton::Left && m_pressedIndex >= 0) {
        if (onActivated)
            onActivated(rowOf(m_pressedIndex));
        else
            m_model.openSelected();
    }
}

void FolderView::pointerMoved(const ui::PointerEvent& event)
{
    const ui::PointF view = fromContent(event.position);
    m_lastMouse = view;
    m_ctrlPressed = event.modifiers & ui::ControlModifier;
    m_shiftPressed = event.modifiers & ui::ShiftModifier;

    // No hover highlight follows the pointer while the band is out.
    if (m_rubberBand) {
        updateRubberBand(view);
        updateDragScrollPosition(view.y);
        return;
    }

    hover(view);

    if (!m_pressActive || !m_press || m_chooser)
        return;
    if (std::abs(view.x - m_press->x) < kStartDragDistance && std::abs(view.y - m_press->y) < kStartDragDistance)
        return;

    if (m_pressedIndex >= 0 && m_model.isSelected(rowOf(m_pressedIndex))) {
        startDrag(view);
        clearPressState();
        return;
    }

    if (m_editor)
        return;

    m_model.pinSelection();
    m_hoveredIndex = -1;
    auto band = std::make_unique<RubberBand>(m_desktopStyle);
    m_rubberBand = band.get();
    m_content->addChild(std::move(band));
    // Otherwise the band first shows on the next move, a step behind.
    updateRubberBand(view);
    updateDragScrollPosition(view.y);
    stateChanged();
}

void FolderView::pointerReleased(const ui::PointerEvent& event)
{
    (void)event;
    // A plain click inside a multiple selection narrows it to that item.
    if (m_pressedIndex >= 0 && !m_rubberBand && !m_shiftPressed && !m_ctrlPressed && !m_model.dragging()) {
        m_model.clearSelection();
        m_model.setSelected(rowOf(m_pressedIndex));
    }
    pressCanceled();
}

void FolderView::clearPressState()
{
    m_pressActive = false;
    m_pressedIndex = -1;
    m_press.reset();
}

void FolderView::pressCanceled()
{
    stopDragScroll();

    if (m_rubberBand) {
        RubberBand* band = m_rubberBand;
        m_rubberBand = nullptr;
        band->close([this, alive = m_alive, band] {
            if (!*alive)
                return;
            std::shared_ptr<View> gone = m_content->takeChild(band);
            base::EventLoop::current()->post([gone] {});
        });
        m_cachedRectangleSelection.clear();
        m_cachedRectangleSelectionMode = -1;
        m_model.unpinSelection();
        stateChanged();
    }

    clearPressState();
}

void FolderView::updateRubberBand(ui::PointF view)
{
    if (!m_rubberBand || !m_contentPress)
        return;

    const ui::PointF press = *m_contentPress;
    const ui::PointF position = toContent(view);
    const ui::SizeF content = m_content->geometry().size();
    const double left = 0;
    const double right = std::max(geometry().width, content.width);
    const double top = 0;
    const double bottom = std::max(content.height, scrollPosition().y + geometry().height);

    double x;
    double width;
    if (position.x < press.x) {
        x = std::max(left, position.x);
        width = std::abs(x - press.x);
    } else {
        x = press.x;
        width = std::min(right - x, std::abs(x - position.x));
    }

    double y;
    double height;
    if (position.y < press.y) {
        y = std::max(top, position.y);
        height = std::abs(y - press.y);
    } else {
        y = press.y;
        height = std::min(bottom - y, std::abs(y - position.y));
    }

    // At least a pixel, or it shows nothing and matches nothing.
    const ui::RectF band {x, y, std::max(1.0, width), std::max(1.0, height)};
    m_rubberBand->setGeometry(band);
    m_content->raiseChild(m_rubberBand);
    rectangleSelect(band);
}

void FolderView::ensureTicker()
{
    if (m_ticker || !window())
        return;
    m_ticker = std::make_unique<FrameTicker>(window());
    m_ticker->onTick = [this](uint64_t now) {
        autoScroll(now);
    };
}

void FolderView::updateDragScrollPosition(double y)
{
    m_dragPositionY = y;
    m_dragScrollActive = true;

    const double edge = dragScrollEdge();
    const int direction = y < headerHeight() + edge ? -1 : y > geometry().height - edge ? 1 : 0;
    if (direction == m_dragScrollDirection)
        return;

    m_dragScrollDirection = direction;
    m_dragScrollElapsed = 0;
    m_dragScrollVelocity = 0;
    m_lastTickNs = 0;
    ensureTicker();
    if (m_ticker) {
        if (direction != 0 && autoScrolls())
            m_ticker->start();
        else
            m_ticker->stop();
    }
}

void FolderView::stopDragScroll()
{
    m_dragPositionY = -1;
    m_dragScrollDirection = 0;
    m_dragScrollElapsed = 0;
    m_dragScrollVelocity = 0;
    m_dragScrollActive = false;
    if (m_ticker)
        m_ticker->stop();
}

void FolderView::autoScroll(uint64_t now)
{
    if (!m_dragScrollActive || m_dragScrollDirection == 0)
        return;

    const double frameTime = m_lastTickNs ? double(now - m_lastTickNs) / 1e9 : 1.0 / 60;
    m_lastTickNs = now;
    // A stalled frame must not jump the view.
    const double interval = std::min(50.0, frameTime * 1000);
    m_dragScrollElapsed += interval;
    const double maximum = maximumContentY();
    const double edge = dragScrollEdge();
    const double distance = m_dragScrollDirection < 0 ? headerHeight() + edge - m_dragPositionY
                                                       : m_dragPositionY - (geometry().height - edge);
    const double edgeProgress = std::clamp(distance / edge, 0.0, 1.0);
    const double rampProgress = std::clamp(m_dragScrollElapsed / kDragScrollRamp, 0.0, 1.0);
    const double ramp = 1 - std::pow(1 - rampProgress, 3);
    const double target = kDragScrollMaxSpeed * std::pow(edgeProgress, 1.5) * ramp;

    // Eases a quarter of the way per 16 ms frame at any refresh rate.
    m_dragScrollVelocity += (target - m_dragScrollVelocity) * (1 - std::pow(0.75, interval / 16));
    const double next = scrollPosition().y + m_dragScrollDirection * m_dragScrollVelocity * interval / 1000;
    setScrollPosition({scrollPosition().x, std::clamp(next, 0.0, maximum)});
}

uint32_t FolderView::dropActionFor(uint32_t modifiers, uint32_t sourceActions, uint32_t proposed)
{
    if (modifiers == ui::NoModifier && (sourceActions & ui::MoveAction))
        return ui::MoveAction;
    if (proposed != ui::NoAction)
        return proposed;
    if (sourceActions & ui::MoveAction)
        return ui::MoveAction;
    if (sourceActions & ui::CopyAction)
        return ui::CopyAction;
    return ui::NoAction;
}

bool FolderView::updateDropTarget(const std::vector<std::string>& urls, uint32_t sourceActions,
                                  uint32_t modifiers, ui::PointF position)
{
    if (position.x < 0 || position.y < 0 || position.x > geometry().width || position.y > geometry().height) {
        clearDropTarget();
        return false;
    }

    const ui::PointF content = toContent(position);
    const int index = indexAt(content);
    const int row = dropRowAt(content);
    const FileItemPtr item = index >= 0 ? m_model.item(rowOf(index)) : nullptr;
    const bool dragged = m_model.dragging() && item && m_model.isSelected(rowOf(index));
    const int target = row >= 0 && item && !m_model.isBlank(rowOf(index)) && item->isDir && !dragged ? index : -1;
    const uint32_t action = dropActionFor(modifiers, sourceActions,
                                          (modifiers & ui::ControlModifier) ? ui::CopyAction : ui::NoAction);
    const bool allowed = m_model.canDropUrls(urls, row, action);
    if (target != m_dropTargetIndex || allowed != m_dropTargetAllowed) {
        m_dropTargetIndex = target;
        m_dropTargetAllowed = allowed;
        stateChanged();
    }
    return allowed;
}

void FolderView::clearDropTarget()
{
    if (m_dropTargetIndex < 0 && !m_dropTargetAllowed)
        return;
    m_dropTargetIndex = -1;
    m_dropTargetAllowed = false;
    stateChanged();
}

void FolderView::dropAt(const std::vector<std::string>& urls, uint32_t action, ui::PointF position)
{
    const ui::PointF content = toContent(position);
    const int row = dropRowAt(content);
    if (!m_model.canDropUrls(urls, row, action)) {
        clearDropTarget();
        return;
    }
    // Files dropped between icons land on the cell under the pointer.
    if (const int cell = row == -1 ? cellAt(content) : -1; cell >= 0)
        m_model.setNewDocumentPosition(int(std::round(position.x)), int(std::round(position.y)), cell);
    m_model.drop(urls, row, action, int(std::round(position.x)), int(std::round(position.y)));
    clearDropTarget();
}

bool FolderView::canDrop(const ui::DropOffer& offer) const
{
    return !m_chooser && offer.hasMimeType("text/uri-list");
}

void FolderView::dragEnter(const ui::DragEvent& event)
{
    m_dropUrls.clear();
    m_dropUrlsKnown = false;
    m_dropSourceActions = event.offer->sourceActions();
    event.offer->read("text/uri-list", [this, alive = m_alive](std::string data) {
        if (!*alive)
            return;
        ui::MimeData mime;
        mime.setData("text/uri-list", std::move(data));
        m_dropUrls = mime.urls();
        m_dropUrlsKnown = true;
    });
    updateDragScrollPosition(event.position.y);
}

uint32_t FolderView::dragUpdate(const ui::DragEvent& event)
{
    updateDragScrollPosition(event.position.y);
    if (!m_dropUrlsKnown)
        return ui::MoveAction;
    updateDropTarget(m_dropUrls, m_dropSourceActions, event.modifiers, event.position);
    // On the desktop, a drag between icons in the same view is always a move
    // (rearrange), not a copy.  Force MoveAction so the compositor accepts the
    // drop; the underlying canDropUrls / drop path checks m_dragging and routes
    // it to move.emit rather than OperationsClient.
    if (m_desktopStyle && m_model.dragging())
        return ui::MoveAction;
    return dropActionFor(event.modifiers, m_dropSourceActions, ui::NoAction);
}

void FolderView::dragLeave()
{
    stopDragScroll();
    clearDropTarget();
}

uint32_t FolderView::drop(const ui::DragEvent& event)
{
    stopDragScroll();
    const uint32_t action = dropActionFor(event.modifiers, m_dropSourceActions, ui::NoAction);
    dropAt(m_dropUrls, action, event.position);
    return action;
}

void FolderView::startDrag(ui::PointF view)
{
    const std::vector<std::string> urls = m_model.selectedUrls();
    if (urls.empty())
        return;

    ui::DragSource source;
    source.mimeTypes = {"text/uri-list"};
    source.data = [urls](const std::string&) {
        ui::MimeData mime;
        mime.setUrls(urls);
        const std::string* data = mime.data("text/uri-list");
        return data ? *data : std::string();
    };
    source.actions = ui::CopyAction | ui::MoveAction | ui::AskAction;
    source.onFinished = [this, alive = m_alive](ui::DropAction) {
        if (*alive)
            m_model.setDragging(false);
    };

    // The selected items as drawn, where they are relative to the pointer.
    const double scale = window() && window()->scale() > 0 ? window()->scale() : 1;
    struct Part
    {
        ui::RectF rect;
        ui::Bitmap image;
    };
    std::vector<Part> parts;
    std::optional<ui::RectF> bounds;
    forEachItemView([&](int index, ui::View* item) {
        const int row = rowOf(index);
        if (row < 0 || !m_model.isSelected(row) || m_model.isBlank(row) || !item->isVisible())
            return;
        const ui::RectF rect = item->geometry();
        ui::Bitmap image = item->grab(scale);
        if (image.isNull())
            return;
        parts.push_back({rect, std::move(image)});
        if (!bounds) {
            bounds = rect;
        } else {
            const double left = std::min(bounds->x, rect.x);
            const double top = std::min(bounds->y, rect.y);
            const double right = std::max(bounds->x + bounds->width, rect.x + rect.width);
            const double bottom = std::max(bounds->y + bounds->height, rect.y + rect.height);
            bounds = ui::RectF {left, top, right - left, bottom - top};
        }
    });
    if (bounds) {
        ui::Bitmap icon(int(std::ceil(bounds->width * scale)), int(std::ceil(bounds->height * scale)),
                        ui::Bitmap::Format::Argb32Premultiplied);
        icon.fill(0);
        {
            ui::BitmapPainter painter(icon);
            // On whole device pixels, so each snapshot is copied, not resampled.
            for (const Part& part : parts)
                painter.drawBitmap({std::round((part.rect.x - bounds->x) * scale),
                                    std::round((part.rect.y - bounds->y) * scale)},
                                   part.image);
        }
        const ui::PointF press = m_contentPress.value_or(toContent(view));
        source.icon = std::move(icon);
        source.iconScale = scale;
        source.hotSpot = {press.x - bounds->x, press.y - bounds->y};
    }

    // Where the drag was grabbed, for a drop back into this view.
    m_dragIndex = m_pressedIndex;
    m_dragGrab = m_contentPress.value_or(toContent(view));
    m_model.setDragging(true);
    if (!m_content->startDrag(std::move(source)))
        m_model.setDragging(false);
}

bool FolderView::keyPress(const ui::KeyEvent& event)
{
    if (event.key == style::key::ControlL || event.key == style::key::ControlR) {
        m_ctrlPressed = true;
        return false;
    }
    if (event.key == style::key::ShiftL || event.key == style::key::ShiftR) {
        m_shiftPressed = true;
        if (const int index = keyboardIndex(false); index != -1)
            m_anchorIndex = index;
        return false;
    }
    if (onKeyPressed && onKeyPressed(event))
        return true;
    if (event.key == ui::key::Escape && !m_editor) {
        m_model.clearSelection();
        return false;
    }
    if (!m_editor && navigate(event))
        return true;
    return false;
}

bool FolderView::keyRelease(const ui::KeyEvent& event)
{
    if (event.key == style::key::ControlL || event.key == style::key::ControlR) {
        m_ctrlPressed = false;
    } else if (event.key == style::key::ShiftL || event.key == style::key::ShiftR) {
        m_shiftPressed = false;
        m_anchorIndex = 0;
    }
    return false;
}

} // namespace fm
