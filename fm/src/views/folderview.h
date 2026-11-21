// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"
#include "ui/color_mixer.h"
#include "ui/frame_client.h"
#include "ui/scroll_view.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Lingmo::ui {
class DropOffer;
class HeaderBackdrop;
}

namespace fm {

class FolderModel;
class RenameEditor;

// Runs a callback on every frame of its window while started.
class FrameTicker : public ui::FrameClient
{
public:
    explicit FrameTicker(ui::Window* window)
        : FrameClient(window)
    {
    }

    std::function<void(uint64_t frameTimeNs)> onTick;
    void start() { startTicking(); }
    void stop() { stopTicking(); }

protected:
    void tick(uint64_t frameTimeNs) override
    {
        if (onTick)
            onTick(frameTimeNs);
    }
};

// What the grid and the list share: presses, selection, the rubber band and its
// autoscroll, drags out and drops in, keys, and the inline rename. Positions
// are in the view's coordinates unless named content positions, which
// scroll with the items.
class FolderView : public ui::ScrollView
{
public:
    explicit FolderView(FolderModel& model);
    ~FolderView() override;

    FolderModel& model() const { return m_model; }

    // Bars floating over the view's top, which the items scroll beneath.
    void setTopInset(double inset);
    double topInset() const { return m_topInset; }
    // A strip over the bottom, such as the search path strip.
    void setBottomInset(double inset);
    double bottomInset() const { return m_bottomInset; }
    // Where the visible rows start: under the bars and any column header.
    virtual double headerHeight() const { return m_topInset; }
    void setBackdropBase(std::optional<ui::ColorRecipe> color);

    int currentIndex() const { return m_currentIndex; }
    // The current item's model row, or -1.
    int currentRow() const { return m_currentIndex < 0 ? -1 : rowOf(m_currentIndex); }
    // A key from elsewhere, such as Quick Look, as if pressed in the view.
    bool sendKey(const ui::KeyEvent& event) { return keyPress(event); }
    void setCurrentIndex(int index);
    // Keeps the current item clear of the bars over the top.
    virtual void revealIndex(int index) = 0;
    void reset();
    // The items take their text from the model again; an open rename stays.
    virtual void retranslate();
    void rename();
    void cancelRename();
    bool isRenaming() const;
    // Lays out now, for a scroll offset to be applied against the new content size.
    void forceLayout();
    // The scroll offset in content coordinates, as the history keeps it.
    double contentY() const { return scrollPosition().y; }
    void setContentY(double y);
    double maximumContentY() const;
    // The scrolled area, which takes the keys.
    View* contentView() const;

    // Drags from elsewhere in the window (the header, the tab bar), at a
    // point in this view's coordinates.
    void updateDragScrollPosition(double y);
    void stopDragScroll();
    bool updateDropTarget(const std::vector<std::string>& urls, uint32_t sourceActions, uint32_t modifiers,
                          ui::PointF position);
    void clearDropTarget();
    void dropAt(const std::vector<std::string>& urls, uint32_t action, ui::PointF position);

    // The context menu for the selection, or the folder without one.
    std::function<void(ui::PointF windowPosition)> onContextMenu;
    // The file dialog's list: one item at a time unless multiple, no
    // drags, drops or rubber band, and presses on empty space do nothing.
    void setChooserMode(bool multiple)
    {
        m_chooser = true;
        m_chooserMultiple = multiple;
    }
    // A double click on an item, instead of opening the selection; its model row.
    std::function<void(int row)> onActivated;
    // A key the view does not take.
    std::function<bool(const ui::KeyEvent&)> onKeyPressed;
    base::Signal<> currentIndexChanged;

    // The drop action for a drag's modifiers and the actions the source allows.
    static uint32_t dropActionFor(uint32_t modifiers, uint32_t sourceActions, uint32_t proposed);

protected:
    class Content;

    // The item index under a content position, or -1.
    virtual int indexAt(ui::PointF content) const = 0;
    // An item's model row; the desktop's free cells map otherwise.
    virtual int rowOf(int index) const { return index; }
    // The item index of a model row, -1 when it has none.
    virtual int indexOfRow(int row) const { return row; }
    // Selects the items from one index to another, as a Shift range.
    virtual void selectRange(int from, int to);
    // Whether the content position lands on the item's icon or name.
    virtual bool hitsItem(int index, ui::PointF content) const = 0;
    // The folder row a drop at a content position goes into, -1 for the folder shown.
    virtual int dropRowAt(ui::PointF content) const = 0;
    // The cell a new file or a drop at a content position lands on, -1 where
    // files have no fixed cells.
    virtual int cellAt(ui::PointF) const { return -1; }
    // Selects what the band (content coordinates) covers.
    virtual void rectangleSelect(const ui::RectF& band) = 0;
    // The item rect a rename edits over, in this view's coordinates; the
    // label's rect, and whether the editor centres on it.
    virtual std::optional<ui::RectF> labelRect(int index) const = 0;
    virtual bool centeredRename() const { return false; }
    // The labels' pixel size (0 for the theme's) and weight, which the editor takes.
    virtual double labelPixelSize() const { return 0; }
    virtual int labelWeight() const { return 400; }
    virtual void updateItems() = 0;
    // The arrow keys and the rest of the navigation; true when taken.
    virtual bool navigate(const ui::KeyEvent& event) = 0;
    virtual double minimumContentHeight() const;
    // Pixels from the band's edge to the view's edges where autoscroll starts.
    virtual double dragScrollEdge() const { return m_rubberBand ? 24 : 48; }
    // Whether the content scrolls while dragging over the edges.
    virtual bool autoScrolls() const { return true; }
    // Each item view made for an index, for the drag image.
    virtual void forEachItemView(const std::function<void(int index, ui::View* item)>&) const {}
    // The item rect the drag image of an item covers, content coordinates.
    virtual std::optional<ui::RectF> dragRect(int index) const = 0;
    // Paints the item into a drag image.
    virtual void paintDragItem(int index, ui::Painter& painter, const ui::RectF& rect) = 0;
    // Called before the item views are told the selection, hover or drop target changed.
    virtual void stateChanged() { updateItems(); }

    void layout() override;
    void positionChanged() override;
    bool keyPress(const ui::KeyEvent& event) override;
    bool keyRelease(const ui::KeyEvent& event) override;
    bool canDrop(const ui::DropOffer& offer) const override;
    void dragEnter(const ui::DragEvent& event) override;
    uint32_t dragUpdate(const ui::DragEvent& event) override;
    void dragLeave() override;
    uint32_t drop(const ui::DragEvent& event) override;

    void updateSelection(uint32_t modifiers);
    // The index the arrow keys move from: the current item while it is
    // selected, otherwise the selection's last item going forward and its
    // first going back, as a band or the model may have selected it.
    int keyboardIndex(bool forward) const;
    void applyRectangleSelection(const std::vector<int>& indices);

    int hoveredIndex() const { return m_hoveredIndex; }
    int pressedIndex() const { return m_pressedIndex; }
    int dropTargetIndex() const { return m_dropTargetIndex; }
    bool dropTargetAllowed() const { return m_dropTargetAllowed; }
    int anchorIndex() const { return m_anchorIndex; }
    void setAnchorIndex(int index) { m_anchorIndex = index; }
    bool ctrlPressed() const { return m_ctrlPressed; }
    bool shiftPressed() const { return m_shiftPressed; }
    bool rubberBandActive() const { return m_rubberBand != nullptr; }
    // The item a running drag of this view started on, and the press in content coordinates.
    int dragIndex() const { return m_dragIndex; }
    ui::PointF dragGrab() const { return m_dragGrab; }
    // The desktop's look for the rubber band, over the wallpaper.
    void setDesktopStyle(bool desktop) { m_desktopStyle = desktop; }

    // The view's own size less the scroll bar, in which the items lay out.
    double viewportWidth() const { return geometry().width; }
    double viewportHeight() const { return geometry().height; }
    // Where in content coordinates the scrolled area's left edge is.
    double contentLeft() const { return 0; }
    ui::PointF toContent(ui::PointF view) const;
    ui::PointF fromContent(ui::PointF content) const;

private:
    class RubberBand;
    friend class Content;

    void pointerPressed(const ui::PointerEvent& event);
    void pointerMoved(const ui::PointerEvent& event);
    void pointerReleased(const ui::PointerEvent& event);
    void pointerLeft();
    void hover(ui::PointF view);
    void updateRubberBand(ui::PointF view);
    void clearPressState();
    void pressCanceled();
    void autoScroll(uint64_t frameTimeNs);
    void startDrag(ui::PointF view);
    void ensureTicker();
    void openContextMenu(ui::PointF view);

    FolderModel& m_model;
    Content* m_content;
    ui::HeaderBackdrop* m_backdrop = nullptr;
    std::optional<ui::ColorRecipe> m_backdropBase;
    double m_topInset = 0;
    double m_bottomInset = 0;

    int m_currentIndex = -1;
    bool m_chooser = false;
    bool m_desktopStyle = false;
    int m_dragIndex = -1;
    ui::PointF m_renamePosition;
    ui::PointF m_dragGrab;
    bool m_chooserMultiple = false;
    int m_anchorIndex = 0;
    int m_hoveredIndex = -1;
    int m_pressedIndex = -1;
    int m_dropTargetIndex = -1;
    bool m_dropTargetAllowed = false;
    bool m_pressActive = false;
    bool m_ctrlPressed = false;
    bool m_shiftPressed = false;
    std::optional<ui::PointF> m_press;
    std::optional<ui::PointF> m_contentPress;
    ui::PointF m_lastMouse {-1, -1};
    bool m_pointerInside = false;

    RubberBand* m_rubberBand = nullptr;
    std::vector<int> m_cachedRectangleSelection;
    int m_cachedRectangleSelectionMode = -1;

    double m_dragPositionY = -1;
    int m_dragScrollDirection = 0;
    double m_dragScrollElapsed = 0;
    double m_dragScrollVelocity = 0;
    bool m_dragScrollActive = false;
    uint64_t m_lastTickNs = 0;
    std::unique_ptr<FrameTicker> m_ticker;

    // The drag over the view and what it carries.
    std::vector<std::string> m_dropUrls;
    bool m_dropUrlsKnown = false;
    uint32_t m_dropSourceActions = 0;

    RenameEditor* m_editor = nullptr;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
