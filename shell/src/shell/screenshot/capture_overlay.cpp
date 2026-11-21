// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/capture_overlay.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "base/settings.h"
#include "core/preferences.h"
#include "shell/common/file_thread.h"
#include "shell/screenshot/capture_settings.h"
#include "shell/screenshot/capture_toolbar.h"
#include "shell/screenshot/screen_capture.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/bubble_shape.h"
#include "ui/embedded_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

namespace Lingmo::Shell {

namespace {

// JavaScript's Math.round: halves go up.
double jsRound(double value)
{
    return std::floor(value + 0.5);
}

ui::RectF between(ui::PointF a, ui::PointF b)
{
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::abs(a.x - b.x), std::abs(a.y - b.y)};
}

bool intersects(const ui::RectF &a, const ui::RectF &b)
{
    return a.width > 0 && a.height > 0 && b.width > 0 && b.height > 0 && a.x < b.x + b.width
            && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}

// Text.Outline: the text over its copies a device pixel off each way.
class OutlinedLabel final : public ui::Label
{
public:
    using Label::Label;

    void setOutline(ui::Color color) { m_outline = color; }

protected:
    void paint(ui::Painter &painter) override
    {
        const double step = 1 / painter.scale();
        for (const ui::PointF shift : {ui::PointF {-step, 0}, ui::PointF {step, 0}, ui::PointF {0, -step},
                                       ui::PointF {0, step}})
            paintText(painter, shift, m_outline);
        Label::paint(painter);
    }

private:
    ui::Color m_outline;
};

// The timer counting down over what is about to be captured.
class CountdownBadge final : public ui::View
{
public:
    CountdownBadge()
    {
        m_label = addChild<ui::Label>();
        m_label->setFontSize(44);
        m_label->setWeight(600);
        m_label->setColor(ui::Color {255, 255, 255, 255});
        m_label->setElide(ui::Label::Elide::None);
    }

    void setNumber(int number) { m_label->setText(std::to_string(number)); }

protected:
    void layout() override
    {
        const ui::SizeF size = m_label->sizeHint();
        const ui::RectF rect = localRect();
        m_label->setGeometry({(rect.width - size.width) / 2, (rect.height - size.height) / 2, size.width, size.height});
    }

    void paint(ui::Painter &painter) override
    {
        painter.drawRoundedRect(localRect(), {20, 0, ui::Corners::Circular}, ui::Color {0, 0, 0, 255}.withAlpha(0.55));
    }

private:
    ui::Label *m_label = nullptr;
};

} // namespace

// The overlay's content, everything in this output's coordinates; the
// capture speaks global ones.
class CaptureOverlay::View final : public ui::View
{
public:
    View(CaptureOverlay *overlay, ScreenCapture *capture, Shell *shell)
        : m_overlay(overlay)
        , m_capture(capture)
    {
        setTracksHover(true);
        m_coordinates = addChild<OutlinedLabel>();
        m_coordinates->setFontSize(11);
        m_coordinates->setWeight(700);
        m_coordinates->setColor(ui::Color {255, 255, 255, 255});
        m_coordinates->setOutline(ui::Color {0, 0, 0, 255}.withAlpha(0.6));
        m_coordinates->setElide(ui::Label::Elide::None);
        m_coordinates->setVisible(false);
        m_toolbar = addChild<CaptureToolbar>(overlay, capture, shell);
        m_toolbar->setVisible(false);
        m_toolbar->onMoved = [this] {
            const ui::RectF rect = m_toolbar->geometry();
            m_capture->settings().setToolbarGeometry(globalRect(rect));
        };
        m_toolbar->onOptionsChanged = [this] {
            // The menu's popup took the keys; Escape and Return are the
            // overlay's again.
            if (!m_toolbar->isOptionsOpen() && m_capture->isActive()) {
                m_overlay->activate();
                setFocus();
            }
            m_overlay->updateCursor();
        };
        m_badge = addChild<CountdownBadge>();
        m_badge->setVisible(false);
    }

    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    CaptureToolbar *toolbar() const { return m_toolbar; }
    OutlinedLabel *coordinates() const { return m_coordinates; }

    // --- What the overlay's views read ---

    double width() const { return m_overlay->bounds().width(); }
    double height() const { return m_overlay->bounds().height(); }
    bool toolbarStyle() const { return m_capture->isToolbarStyle(); }
    int source() const { return m_capture->source(); }
    bool counting() const { return m_capture->countdown() > 0; }
    bool hasToolbar() const { return toolbarStyle() && m_capture->toolbarOutput() == m_overlay->outputName(); }
    bool toolbarShown() const { return hasToolbar() && !counting(); }

    ui::RectF localRect(const base::Rect &rect) const
    {
        const base::Rect screen = m_overlay->bounds();
        return {double(rect.x() - screen.x()), double(rect.y() - screen.y()), double(rect.width()),
                double(rect.height())};
    }

    // Edges rounded, not the size, so an edge on the screen's stays there.
    base::Rect globalRect(const ui::RectF &rect) const
    {
        const base::Rect screen = m_overlay->bounds();
        const int left = int(jsRound(rect.x + screen.x()));
        const int top = int(jsRound(rect.y + screen.y()));
        return base::Rect(left, top, int(jsRound(rect.x + rect.width + screen.x())) - left,
                          int(jsRound(rect.y + rect.height + screen.y())) - top);
    }

    bool containsCentre(const base::Rect &rect) const
    {
        const base::Rect screen = m_overlay->bounds();
        const double cx = rect.x() + rect.width() / 2.0;
        const double cy = rect.y() + rect.height() / 2.0;
        return cx >= screen.x() && cx < screen.x() + screen.width() && cy >= screen.y()
                && cy < screen.y() + screen.height();
    }

    ui::RectF selection() const { return localRect(m_capture->selection()); }
    bool selectionHere() const
    {
        return m_capture->selection().width() > 0 && containsCentre(m_capture->selection());
    }

    // The pointer stops on the last pixel, short of the right and bottom
    // edges; within a pixel of them it takes the whole screen.
    ui::PointF clampPoint(ui::PointF p) const
    {
        const double x = width() - p.x <= 1 ? width() : std::max(0.0, p.x);
        const double y = height() - p.y <= 1 ? height() : std::max(0.0, p.y);
        return {x, y};
    }

    // The pointer rests a third of a pixel inside the far edges at 150%, so
    // jitter along them would flip a selection edge between the last two
    // pixels. Once on an edge, the drag keeps to it until the pointer is clear.
    ui::PointF stickToEdges(ui::PointF p)
    {
        m_atRightEdge = width() - p.x <= (m_atRightEdge ? 2 : 1);
        m_atBottomEdge = height() - p.y <= (m_atBottomEdge ? 2 : 1);
        return {m_atRightEdge ? width() : p.x, m_atBottomEdge ? height() : p.y};
    }

    // Which of the selection's handles or edges is at p; 8 inside, -1 outside.
    int handleAt(ui::PointF p) const
    {
        if (!selectionHere())
            return -1;
        const ui::RectF s = selection();
        const ui::PointF corners[4] = {{s.x, s.y}, {s.x + s.width, s.y}, {s.x + s.width, s.y + s.height},
                                       {s.x, s.y + s.height}};
        for (int i = 0; i < 4; ++i) {
            if (std::abs(p.x - corners[i].x) <= 8 && std::abs(p.y - corners[i].y) <= 8)
                return i;
        }
        const bool withinX = p.x >= s.x && p.x <= s.x + s.width;
        const bool withinY = p.y >= s.y && p.y <= s.y + s.height;
        if (withinX && std::abs(p.y - s.y) <= 5)
            return 4;
        if (withinY && std::abs(p.x - (s.x + s.width)) <= 5)
            return 5;
        if (withinX && std::abs(p.y - (s.y + s.height)) <= 5)
            return 6;
        if (withinY && std::abs(p.x - s.x) <= 5)
            return 7;
        return withinX && withinY ? 8 : -1;
    }

    // The toolbar's selection after a drag that started at the anchor.
    ui::RectF adjusted(ui::PointF p) const
    {
        const ui::RectF s = m_dragStart;
        const double dx = p.x - m_dragAnchor.x;
        const double dy = p.y - m_dragAnchor.y;
        if (m_dragHandle == 8) {
            const double x = std::max(0.0, std::min(width() - s.width, s.x + dx));
            const double y = std::max(0.0, std::min(height() - s.height, s.y + dy));
            return {x, y, s.width, s.height};
        }
        if (m_dragHandle < 0)
            return between(m_dragAnchor, clampPoint(p));
        double left = s.x;
        double top = s.y;
        double right = s.x + s.width;
        double bottom = s.y + s.height;
        const int h = m_dragHandle;
        if (h == 0 || h == 3 || h == 7)
            left += dx;
        if (h == 1 || h == 2 || h == 5)
            right += dx;
        if (h == 0 || h == 1 || h == 4)
            top += dy;
        if (h == 2 || h == 3 || h == 6)
            bottom += dy;
        return between(clampPoint({left, top}), clampPoint({right, bottom}));
    }

    // The crosshair's selection: Space moves it, Option grows it from the
    // centre, Shift keeps every edge but the one being dragged.
    ui::RectF drafted(ui::PointF p, uint32_t modifiers)
    {
        if (m_spaceHeld) {
            const double dx = p.x - m_dragLast.x;
            const double dy = p.y - m_dragLast.y;
            m_dragAnchor = {m_dragAnchor.x + dx, m_dragAnchor.y + dy};
            return {m_draft.x + dx, m_draft.y + dy, m_draft.width, m_draft.height};
        }
        ui::RectF rect;
        if (modifiers & ui::AltModifier) {
            const double w = std::abs(p.x - m_dragAnchor.x);
            const double h = std::abs(p.y - m_dragAnchor.y);
            rect = {m_dragAnchor.x - w, m_dragAnchor.y - h, w * 2, h * 2};
        } else {
            rect = between(m_dragAnchor, p);
        }
        if (modifiers & ui::ShiftModifier) {
            if (m_shiftPoint.x < 0) {
                m_shiftPoint = p;
                m_shiftDraft = m_draft;
            }
            if (std::abs(p.x - m_shiftPoint.x) >= std::abs(p.y - m_shiftPoint.y)) {
                rect.y = m_shiftDraft.y;
                rect.height = m_shiftDraft.height;
            } else {
                rect.x = m_shiftDraft.x;
                rect.width = m_shiftDraft.width;
            }
        } else {
            m_shiftPoint = {-1, -1};
        }
        const ui::PointF topLeft = clampPoint({rect.x, rect.y});
        const ui::PointF bottomRight = clampPoint({rect.x + rect.width, rect.y + rect.height});
        return between(topLeft, bottomRight);
    }

    // Where it was last dragged to while that is on this screen, kept whole
    // on it; centred above the dock otherwise.
    void placeToolbar()
    {
        const ui::SizeF size = m_toolbar->implicitSize();
        const base::Rect saved = m_capture->settings().toolbarGeometry();
        double x;
        double y;
        if (saved.width() > 0 && containsCentre(saved)) {
            const ui::RectF local = localRect(saved);
            x = std::max(0.0, std::min(jsRound(local.x), width() - size.width));
            y = std::max(0.0, std::min(jsRound(local.y), height() - size.height));
        } else {
            const ui::RectF area = localRect(m_capture->toolbarArea());
            x = jsRound(area.x + (area.width - size.width) / 2);
            y = jsRound(area.y + area.height - size.height);
        }
        m_toolbar->setGeometry({x, y, size.width, size.height});
        m_placedArea = m_capture->toolbarArea();
        m_placedWidth = size.width;
    }

    // --- Following the capture ---

    void sync()
    {
        const bool active = m_capture->isActive();
        if (active != m_active) {
            m_active = active;
            m_dragging = false;
            m_spaceHeld = false;
            m_toolbar->setSelecting(false);
            if (active) {
                placeToolbar();
                setFocus();
            }
        }
        m_toolbar->update();
        if (active && (m_capture->toolbarArea() != m_placedArea || m_toolbar->implicitSize().width != m_placedWidth))
            placeToolbar();
        const bool toolbar = toolbarShown();
        if (toolbar != m_toolbar->isVisible()) {
            m_toolbar->setVisible(toolbar);
            m_toolbar->setEnabled(toolbar);
            m_toolbar->visibilityChanged();
        }
        updateCoordinates();
        updateBadge();
        requestPaint();
    }

    void updateCoordinates()
    {
        const bool shown = !toolbarStyle() && source() == ScreenCapture::Region && m_pointerInside && m_active;
        m_coordinates->setVisible(shown);
        if (!shown)
            return;
        const ui::PointF p = m_pointer;
        m_coordinates->setText(m_dragging ? std::format("{}\n{}", int(jsRound(m_draft.width)), int(jsRound(m_draft.height)))
                                          : std::format("{}\n{}", int(jsRound(p.x)), int(jsRound(p.y))));
        const ui::SizeF size = m_coordinates->sizeHint();
        const bool fitsRight = p.x + 12 + size.width < width();
        const bool fitsBelow = p.y + 12 + size.height < height();
        m_coordinates->setGeometry({fitsRight ? p.x + 12 : p.x - 12 - size.width,
                                    fitsBelow ? p.y + 12 : p.y - 12 - size.height, size.width, size.height});
    }

    void updateBadge()
    {
        const ui::RectF area = localRect(m_capture->countdownArea());
        const double left = std::max(0.0, area.x);
        const double top = std::max(0.0, area.y);
        const double right = std::min(width(), area.x + area.width);
        const double bottom = std::min(height(), area.y + area.height);
        const ui::RectF shown {left, top, std::max(0.0, right - left), std::max(0.0, bottom - top)};
        const bool visible = counting() && shown.width > 0 && shown.height > 0;
        m_badge->setVisible(visible);
        if (!visible)
            return;
        m_badge->setNumber(m_capture->countdown());
        m_badge->setGeometry({shown.x + (shown.width - 96) / 2, shown.y + (shown.height - 96) / 2, 96, 96});
    }

    CaptureOverlay::Shape shape() const
    {
        if (counting() || m_toolbar->isOptionsOpen())
            return Shape::Arrow;
        // The toolbar is not a place to capture from.
        if (m_toolbar->isVisible() && !m_dragging && m_pointerInside) {
            const ui::RectF rect = m_toolbar->geometry();
            if (m_pointer.x >= rect.x && m_pointer.x < rect.x + rect.width && m_pointer.y >= rect.y
                && m_pointer.y < rect.y + rect.height)
                return Shape::Arrow;
        }
        if (source() != ScreenCapture::Region)
            return Shape::Camera;
        const int handle = m_dragging ? m_dragHandle : toolbarStyle() ? handleAt(m_pointer) : -1;
        switch (handle) {
        case 0:
        case 2:
            return Shape::ResizeDiagonal;
        case 1:
        case 3:
            return Shape::ResizeAntiDiagonal;
        case 4:
        case 6:
            return Shape::ResizeVertical;
        case 5:
        case 7:
            return Shape::ResizeHorizontal;
        case 8:
            return m_dragging ? Shape::ClosedHand : Shape::OpenHand;
        default:
            return Shape::Crosshair;
        }
    }

protected:
    // A theme change lays every window out again.
    void layout() override
    {
        if (m_overlay->m_glassDark != ui::Theme::current().darkMode)
            m_overlay->updateGlass();
    }

    void hoverChanged() override
    {
        if (!isHovered()) {
            m_pointerInside = false;
            updateCoordinates();
        }
    }

    void hoverMove(const ui::PointerEvent &event) override
    {
        if (!m_held)
            moved(event.position, event.modifiers, false);
    }

    void pointerMove(const ui::PointerEvent &event) override { moved(event.position, event.modifiers, true); }

    void pointerPress(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left)
            return;
        m_held = true;
        if (counting() || source() != ScreenCapture::Region)
            return;
        m_dragging = true;
        m_atRightEdge = false;
        m_atBottomEdge = false;
        m_dragAnchor = clampPoint(stickToEdges(event.position));
        m_dragLast = m_dragAnchor;
        m_shiftPoint = {-1, -1};
        if (toolbarStyle()) {
            m_dragHandle = handleAt(event.position);
            m_dragStart = selection();
        } else {
            m_dragHandle = -1;
            m_draft = {m_dragAnchor.x, m_dragAnchor.y, 0, 0};
        }
        m_toolbar->setSelecting(true);
        m_overlay->updateCursor();
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent &event) override
    {
        if (event.button != ui::MouseButton::Left || !std::exchange(m_held, false))
            return;
        if (counting())
            return;
        const bool toClipboard = event.modifiers & ui::ControlModifier;
        const base::Rect screen = m_overlay->bounds();
        // Picking a screen or a window is a click.
        if (source() != ScreenCapture::Region) {
            m_capture->clickAt(base::PointF(event.position.x + screen.x(), event.position.y + screen.y()).toPoint(),
                               m_overlay->outputName(), toClipboard);
            return;
        }
        if (!m_dragging)
            return;
        m_dragging = false;
        m_spaceHeld = false;
        m_toolbar->setSelecting(false);
        m_overlay->updateCursor();
        if (toolbarStyle()) {
            requestPaint();
            return;
        }
        const ui::RectF draft = std::exchange(m_draft, ui::RectF {});
        if (draft.width >= 2 && draft.height >= 2)
            m_capture->finishSelection(globalRect(draft), toClipboard);
        requestPaint();
        updateCoordinates();
    }

    bool keyPress(const ui::KeyEvent &event) override
    {
        switch (event.key) {
        case ui::key::Escape:
            m_capture->cancel();
            return true;
        case ui::key::Return:
        case ui::key::KeypadEnter:
            if (toolbarStyle())
                m_capture->perform(event.modifiers & ui::ControlModifier);
            return true;
        case ui::key::Space:
            if (event.repeat)
                return true;
            if (!toolbarStyle() && m_dragging)
                m_spaceHeld = true;
            else if (!toolbarStyle())
                m_capture->toggleWindowSelection();
            return true;
        default:
            return false;
        }
    }

    bool keyRelease(const ui::KeyEvent &event) override
    {
        if (event.key == ui::key::Space && !event.repeat)
            m_spaceHeld = false;
        return false;
    }

    void paint(ui::Painter &painter) override;

private:
    void moved(ui::PointF position, uint32_t modifiers, bool held)
    {
        m_pointerInside = true;
        const base::Rect screen = m_overlay->bounds();
        m_capture->pointerMoved(base::PointF(position.x + screen.x(), position.y + screen.y()).toPoint());
        const bool dragged = m_dragging && held && !counting();
        const ui::PointF point = dragged ? stickToEdges(position) : position;
        if (dragged) {
            if (toolbarStyle())
                m_capture->setSelection(globalRect(adjusted(point)));
            else
                m_draft = drafted(point, modifiers);
        }
        m_dragLast = point;
        m_pointer = position;
        updateCoordinates();
        m_overlay->updateCursor();
        if (dragged)
            requestPaint();
    }

    CaptureOverlay *m_overlay;
    ScreenCapture *m_capture;
    CaptureToolbar *m_toolbar = nullptr;
    OutlinedLabel *m_coordinates = nullptr;
    CountdownBadge *m_badge = nullptr;
    bool m_active = false;
    base::Rect m_placedArea;
    double m_placedWidth = 0;

    ui::RectF m_draft;
    ui::PointF m_pointer;
    bool m_pointerInside = false;
    bool m_held = false;
    bool m_dragging = false;
    // What a drag in the toolbar's portion mode does: -1 draws a new
    // selection, 8 moves it, 0 to 7 pull a handle.
    int m_dragHandle = -1;
    ui::PointF m_dragAnchor;
    ui::PointF m_dragLast;
    ui::RectF m_dragStart;
    bool m_spaceHeld = false;
    ui::PointF m_shiftPoint {-1, -1};
    ui::RectF m_shiftDraft;
    bool m_atRightEdge = false;
    bool m_atBottomEdge = false;
};

namespace {

// What the overlay paints around the toolbar. The toolbar's glass is the
// compositor's, behind the window, so nothing may cover the toolbar: shapes
// over it lose that part.
class Covers
{
public:
    Covers(ui::Painter &painter, std::optional<ui::RectF> hole, double radius)
        : m_painter(painter)
    {
        if (!hole)
            return;
        // Snapped as FishUI's RoundedRectangle in the mask is.
        const ui::Rect device = painter.snappedRect(*hole);
        const double scale = painter.scale();
        m_holeRect = {device.x / scale, device.y / scale, device.width / scale, device.height / scale};
        ui::BubbleShape shape;
        shape.width = m_holeRect.width;
        shape.height = m_holeRect.height;
        shape.radius = radius;
        shape.continuousCorners = ui::Theme::current().continuousCorners;
        m_hole = shape.path().translated(m_holeRect.x, m_holeRect.y);
        m_active = true;
    }

    // A plain rectangle: its edges on the device grid.
    void fillRect(const ui::RectF &rect, ui::Color color)
    {
        if (rect.width <= 0 || rect.height <= 0)
            return;
        if (!m_active || !intersects(rect, m_holeRect)) {
            m_painter.fillRect(rect, color);
            return;
        }
        ui::Path path;
        path.addRect(snap(m_painter.deviceRect(rect)));
        m_painter.fillPath(path.subtracted(m_hole), color);
    }

    // FishUI's RoundedRectangle with circular corners and a border inside.
    void roundedRect(const ui::RectF &rect, double radius, ui::Color fill, double borderWidth = 0,
                     ui::Color border = {})
    {
        if (rect.width <= 0 || rect.height <= 0)
            return;
        if (!m_active || !intersects(rect, m_holeRect)) {
            m_painter.drawRoundedRect(rect, {radius, 0, ui::Corners::Circular}, fill, borderWidth, border);
            return;
        }
        const ui::RectF outer = snap(m_painter.snappedRect(rect));
        ui::Path shape;
        shape.addRoundedRect(outer, radius, radius);
        if (fill.a > 0)
            m_painter.fillPath(shape.subtracted(m_hole), fill);
        if (borderWidth > 0 && border.a > 0) {
            const double width = std::max(std::round(borderWidth * m_painter.scale()), 1.0) / m_painter.scale();
            const ui::RectF inner {outer.x + width, outer.y + width, outer.width - 2 * width, outer.height - 2 * width};
            ui::Path inside;
            if (inner.width > 0 && inner.height > 0)
                inside.addRoundedRect(inner, std::max(0.0, radius - width), std::max(0.0, radius - width));
            m_painter.fillPath(shape.subtracted(inside).subtracted(m_hole), border);
        }
    }

private:
    ui::RectF snap(const ui::Rect &device) const
    {
        const double scale = m_painter.scale();
        return {device.x / scale, device.y / scale, device.width / scale, device.height / scale};
    }

    ui::Painter &m_painter;
    bool m_active = false;
    ui::RectF m_holeRect;
    ui::Path m_hole;
};

// The outline of a selected portion and the eight handles it is resized by,
// kept on the screen.
void paintFrame(Covers &covers, double scale, const ui::RectF &rect, double screenWidth, double screenHeight,
                bool handles)
{
    const double border = 1 / scale;
    const double left = std::max(rect.x - border, 0.0);
    const double top = std::max(rect.y - border, 0.0);
    const double right = std::min(rect.x + rect.width + border, screenWidth);
    const double bottom = std::min(rect.y + rect.height + border, screenHeight);
    covers.roundedRect({left, top, right - left, bottom - top}, 0, ui::Color {0, 0, 0, 0}, border,
                       ui::Color {255, 255, 255, 255}.withAlpha(0.9));
    if (!handles)
        return;
    static constexpr double kHandle = 9;
    static constexpr ui::PointF kAnchors[8] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {0.5, 0}, {1, 0.5}, {0.5, 1}, {0, 0.5}};
    for (int i = 0; i < 8; ++i) {
        // A small portion keeps only the corners; the edge handles would crowd them.
        if (i >= 4 && std::min(rect.width, rect.height) < kHandle * 5)
            continue;
        const double x = std::max(0.0, std::min(screenWidth - kHandle, rect.x + kAnchors[i].x * rect.width - kHandle / 2));
        const double y = std::max(0.0, std::min(screenHeight - kHandle, rect.y + kAnchors[i].y * rect.height - kHandle / 2));
        covers.roundedRect({x, y, kHandle, kHandle}, kHandle / 2, ui::Color {255, 255, 255, 255}, border,
                           ui::Color {0, 0, 0, 255}.withAlpha(0.35));
    }
}

} // namespace

void CaptureOverlay::View::paint(ui::Painter &painter)
{
    if (!m_active)
        return;
    std::optional<ui::RectF> hole;
    if (m_toolbar->isVisible())
        hole = m_toolbar->geometry();
    Covers covers(painter, hole, m_toolbar->radius());
    const double w = width();
    const double h = height();

    // The rest of the screen, dimmed around a portion. A screen without the
    // portion is dimmed whole.
    const bool region = source() == ScreenCapture::Region;
    if (toolbarStyle() && region) {
        const ui::Color dim = ui::Color {0, 0, 0, 255}.withAlpha(0.4);
        const bool here = selectionHere();
        const ui::RectF cut = here ? selection() : ui::RectF {};
        covers.fillRect({0, 0, w, here ? cut.y : h}, dim);
        if (here) {
            covers.fillRect({0, cut.y + cut.height, w, h - (cut.y + cut.height)}, dim);
            covers.fillRect({0, cut.y, cut.x, cut.height}, dim);
            covers.fillRect({cut.x + cut.width, cut.y, w - (cut.x + cut.width), cut.height}, dim);
            paintFrame(covers, painter.scale(), cut, w, h, !counting());
        }
    }

    // The window a click would capture.
    if (source() == ScreenCapture::Window && m_capture->hoveredWindow().width() > 0)
        covers.roundedRect(localRect(m_capture->hoveredWindow()), m_capture->hoveredWindowRadius(),
                           ui::Color {89, 153, 255, 255}.withAlpha(0.3));

    // The crosshair's selection as it is drawn.
    if (!toolbarStyle() && m_dragging && region) {
        painter.fillRect(m_draft, ui::Color {153, 153, 153, 255}.withAlpha(0.3));
        Covers plain(painter, std::nullopt, 0);
        paintFrame(plain, painter.scale(), m_draft, w, h, false);
    }
}

CaptureOverlay::CaptureOverlay(ScreenCapture *capture, Shell *shell, const Shell::Output &output)
    : m_capture(capture)
    , m_shell(shell)
    , m_output(output)
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Capture);
    auto view = std::make_unique<View>(this, capture, shell);
    m_view = view.get();
    m_window->setContentView(std::move(view));
    m_window->setOutput(m_output.name);
    m_window->setGeometry(m_output.bounds);
    m_window->window()->onPointerPressed = [this](ui::PointF, ui::MouseButton) {
        m_view->toolbar()->pressedOutsideMenu();
    };
}

CaptureOverlay::~CaptureOverlay()
{
    m_view->toolbar()->releasePopups();
    m_window.reset();
}

CaptureToolbar *CaptureOverlay::toolbar() const
{
    return m_view->toolbar();
}

ui::RectF CaptureOverlay::toolbarRect() const
{
    return m_view->toolbar()->isVisible() ? m_view->toolbar()->geometry() : ui::RectF {};
}

std::string CaptureOverlay::coordinates() const
{
    const ui::Label *label = m_view->coordinates();
    return label->isVisible() ? label->text() : std::string();
}

void CaptureOverlay::setOutput(const Shell::Output &output)
{
    m_output = output;
    m_window->setOutput(m_output.name);
    m_window->setGeometry(m_output.bounds);
    update();
}

void CaptureOverlay::present(bool focus)
{
    // The pointer size of the moment the interface shows.
    m_cursorSizeLoad = runOnFileThread(
            [] { return int(base::Settings(preferencesFile("services")).intValue("Appearance/CursorSize", 24)); },
            [this](int size) {
                m_cursorSize = size > 0 ? size : 24;
                m_appliedShape.reset();
                updateCursor();
            });
    m_window->setGeometry(m_output.bounds);
    m_window->setInputRegion(std::nullopt);
    m_window->show();
    update();
    if (focus)
        activate();
}

void CaptureOverlay::dismiss()
{
    m_view->toolbar()->closeOptions();
    m_view->toolbar()->hideTooltip();
    m_window->hide();
    setPassThrough(false);
    update();
}

void CaptureOverlay::setPassThrough(bool passThrough)
{
    m_window->setInputRegion(passThrough ? std::optional<base::Region>(base::Region()) : std::nullopt);
}

void CaptureOverlay::activate()
{
    m_window->activate();
    m_view->setFocus();
}

void CaptureOverlay::update()
{
    m_view->sync();
    updateGlass();
    updateCursor();
}

void CaptureOverlay::updateGlass()
{
    const ui::RectF rect = toolbarRect();
    const bool dark = ui::Theme::current().darkMode;
    if (rect == m_glassRect && dark == m_glassDark)
        return;
    m_glassRect = rect;
    m_glassDark = dark;
    if (rect.width <= 0 || rect.height <= 0) {
        m_window->setBackgroundBlur({}, 0);
        return;
    }
    const int left = int(std::lround(rect.x));
    const int top = int(std::lround(rect.y));
    const base::Rect region(left, top, int(std::lround(rect.x + rect.width)) - left,
                            int(std::lround(rect.y + rect.height)) - top);
    m_window->setBackgroundBlur(region, int(std::lround(m_view->toolbar()->radius())),
                                dark ? popupGlass(0.17, 0.17, 0.19, 0.6) : popupGlass(1, 1, 1, 0.6));
}

void CaptureOverlay::updateCursor()
{
    m_shape = m_view->shape();
    const double scale = m_window->scale();
    if (m_appliedShape == m_shape && (m_shape != Shape::Camera || scale == m_appliedScale))
        return;
    m_appliedShape = m_shape;
    m_appliedScale = scale;
    ShellWindow::Cursor cursor;
    switch (m_shape) {
    case Shape::Arrow:
        cursor.shape = input::CursorShape::Default;
        break;
    case Shape::Crosshair:
        cursor.shape = input::CursorShape::Crosshair;
        break;
    case Shape::Camera: {
        // Drawn at the integer ratio above the output's and scaled down by
        // the compositor: a cursor buffer needs an integer scale.
        const int ratio = std::max(1, int(std::ceil(scale)));
        const int pixels = m_cursorSize * ratio;
        cursor.image = ui::Bitmap::decodeFile(shellDataFile("camera-cursor.svg"), base::Size(pixels, pixels));
        cursor.scale = ratio;
        cursor.hotspot = base::Point(m_cursorSize / 2, m_cursorSize / 2);
        if (cursor.image.isNull())
            cursor.shape = input::CursorShape::Crosshair;
        break;
    }
    case Shape::OpenHand:
        cursor.shape = input::CursorShape::Grab;
        break;
    case Shape::ClosedHand:
        cursor.shape = input::CursorShape::Grabbing;
        break;
    case Shape::ResizeHorizontal:
        cursor.shape = input::CursorShape::EwResize;
        break;
    case Shape::ResizeVertical:
        cursor.shape = input::CursorShape::NsResize;
        break;
    case Shape::ResizeDiagonal:
        cursor.shape = input::CursorShape::NwseResize;
        break;
    case Shape::ResizeAntiDiagonal:
        cursor.shape = input::CursorShape::NeswResize;
        break;
    }
    m_window->setCursor(std::move(cursor));
}

} // namespace Lingmo::Shell
