// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/decoration.h"

#include <algorithm>
#include <cmath>

#include "compositor/compositor.h"
#include "core/logging.h"
#include "decoration/decoration_node.h"
#include "decoration/decoration_theme.h"
#include "decoration/text.h"
#include "render/rounded_shape.h"
#include "scene/node.h"
#include "wm/resize_cursor.h"
#include "wm/resize_handles.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"
#include "wm/window_visual_controller.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {

namespace {

using wm::FrameArea;
using wm::ResizeEdge;

// Two presses closer together than this, on nearly the same spot, are a double
// click. Ash uses the platform's double-click interval; Lingmo has no
// settings service yet.
constexpr int kDoubleClickMs = 400;
constexpr int kDoubleClickSlop = 4;
// A finger lands less precisely: GestureConfiguration's
// max_distance_between_taps_for_double_tap.
constexpr int kDoubleTapSlop = 20;


} // namespace

////////////////////////////////////////////////////////////////////////////////
// Decoration:

Decoration::Decoration(wm::Window *window, Theme *theme)
    : m_window(window)
    , m_theme(theme)
{
    m_buttons.emplace_back(CaptionButtonType::Minimize);
    m_buttons.emplace_back(CaptionButtonType::Maximize);
    m_buttons.emplace_back(CaptionButtonType::Close);

    if (scene::Node *windowNode = m_window->visualNode()) {
        auto node = std::make_unique<DecorationNode>(this, m_theme);
        m_node = static_cast<DecorationNode *>(windowNode->addChild(std::move(node)));
        // Behind the client's surface tree. The two do not overlap, but the
        // client's content is what sits in the frame, not the other way round.
        windowNode->lower(m_node);
    }

    m_window->addObserver(this);
    m_theme->addObserver(this);
    layout();
    updateNode();
}

Decoration::~Decoration()
{
    m_theme->removeObserver(this);
    if (m_window)
        m_window->removeObserver(this);
    if (m_node) {
        scene::Node *node = m_node;
        m_node = nullptr;
        if (node->parent())
            node->parent()->removeChild(node);
    }
    scheduleRepaint();
}

bool Decoration::isActive() const
{
    return m_window && m_window->isActive();
}

std::string Decoration::title() const
{
    // The window's title is the only copy; this reads it.
    return m_window ? m_window->title() : std::string();
}

bool Decoration::isRestoreButton() const
{
    // Snapped counts too: the button takes the window back out of a state the
    // pointer did not put it in.
    return m_window
            && (m_window->state()->isMaximized()
                || (m_window->state()->isSnapped() && !m_window->isRestoringFromDrag()));
}

wm::Insets Decoration::extents() const
{
    auto insets = m_theme->metrics().insets();
    if (m_window && m_window->state()->isMaximized())
        return wm::Insets {0, insets.top, 0, 0};
    return insets;
}

base::Rect Decoration::frameRect() const
{
    return m_window ? base::Rect(m_window->presentedBounds().size()) : base::Rect();
}

base::Rect Decoration::titleBarRect() const
{
    const base::Rect frame = frameRect();
    return base::Rect(0, 0, frame.width(),
                      std::min(frame.height(), m_theme->metrics().titleBarHeight));
}

base::Rect Decoration::titleBarBounds() const
{
    if (!m_window || m_window->frameInsets().isNull())
        return base::Rect();
    return titleBarRect().translated(m_window->bounds().topLeft()
                                                        + m_window->presentedBounds().topLeft());
}

std::vector<base::Rect> Decoration::captionButtonBounds() const
{
    std::vector<base::Rect> bounds;
    if (!m_window || m_window->frameInsets().isNull())
        return bounds;
    // The inverse of toFrame().
    const base::Point origin = m_window->bounds().topLeft() + m_window->presentedBounds().topLeft();
    for (const CaptionButton &button : m_buttons)
        bounds.push_back(button.geometry().translated(origin));
    return bounds;
}

base::Rect Decoration::inputBounds() const
{
    if (!m_window)
        return base::Rect();
    const int margin = m_theme->metrics().resizeMargin;
    return m_window->bounds().adjusted(-margin, -margin, margin, margin);
}

base::Rect Decoration::captionArea() const
{
    const Metrics &metrics = m_theme->metrics();
    const base::Rect area = titleBarRect();
    int buttonsWidth = 0;
    if (!m_buttons.empty()) {
        buttonsWidth = static_cast<int>(m_buttons.size()) * metrics.buttonSize
                + (static_cast<int>(m_buttons.size()) - 1) * metrics.buttonSpacing
                + metrics.buttonsRightMargin;
    }
    return area.adjusted(metrics.captionMargin, 0, -(buttonsWidth + metrics.captionMargin), 0);
}

void Decoration::layout()
{
    const Metrics &metrics = m_theme->metrics();
    const base::Rect titleBar = titleBarRect();

    // The group is right-aligned, in the order the Lingmo KWin configuration
    // lists it: minimize, maximize, close.
    int right = titleBar.right() - metrics.buttonsRightMargin;
    const int top = titleBar.top() + std::max(0, (titleBar.height() - metrics.buttonSize) / 2);
    for (auto button = m_buttons.rbegin(); button != m_buttons.rend(); ++button) {
        const int left = right - metrics.buttonSize;
        button->setGeometry(base::Rect(left, top, metrics.buttonSize, metrics.buttonSize));
        right = left - metrics.buttonSpacing;
    }
}

void Decoration::updateNode()
{
    if (!m_node || !m_window)
        return;
    m_node->setGeometry(m_window->presentedBounds());
    // Fullscreen takes the frame away entirely; the window keeps it so that
    // leaving fullscreen brings it straight back.
    m_node->setVisible(!m_window->frameInsets().isNull());
}

void Decoration::scheduleRepaint()
{
    if (m_node)
        m_node->notifyChanged();
    if (m_window && m_window->manager() && m_window->manager()->compositor())
        m_window->manager()->compositor()->scheduleRepaint(true);
}

base::Point Decoration::toFrame(const base::PointF &location) const
{
    if (!m_window)
        return base::Point();
    return location.toPoint() - m_window->bounds().topLeft()
            - m_window->presentedBounds().topLeft();
}

int Decoration::buttonIndexAt(const base::Point &framePoint) const
{
    for (size_t index = 0; index < m_buttons.size(); ++index) {
        if (m_buttons[index].geometry().contains(framePoint))
            return static_cast<int>(index);
    }
    return -1;
}

input::CursorShape Decoration::cursorShapeAt(const base::PointF &location) const
{
    return wm::resizeCursor(resizeEdgeAt(toFrame(location)));
}

ResizeEdge Decoration::resizeEdgeAt(const base::Point &framePoint) const
{
    // A snapped window keeps its resize borders: pulling the shared edge is
    // how its share of the screen is changed.
    if (!m_window || !m_window->isResizable()
        || (!m_window->state()->isNormal() && !m_window->state()->isSnapped()))
        return ResizeEdge::None;

    const Metrics &metrics = m_theme->metrics();
    const auto visual = wm::WindowVisualController::evaluate(*m_window, m_theme->visualStyle());
    const render::RoundedShape shape {
            visual.shape.bounds.translated(-base::PointF(m_window->presentedBounds().topLeft())),
            visual.shape.radius, visual.shape.radiusScale};
    return wm::resizeEdgeFor(shape, frameRect(), framePoint,
                             metrics.resizeMargin, metrics.resizeCorner);
}

FrameArea Decoration::areaAt(const base::PointF &at) const
{
    if (!m_window || !m_window->isVisible() || m_window->frameInsets().isNull())
        return FrameArea::None;
    if (!inputBounds().contains(at.toPoint()))
        return FrameArea::None;

    const base::Point framePoint = toFrame(at);
    if (resizeEdgeAt(framePoint) != ResizeEdge::None)
        return FrameArea::ResizeBorder;
    if (buttonIndexAt(framePoint) >= 0)
        return FrameArea::CaptionButton;
    if (titleBarRect().contains(framePoint))
        return FrameArea::TitleBar;
    // The borders down the sides and along the bottom, when they are too
    // narrow to have been caught as a resize edge.
    if (frameRect().contains(framePoint)
        && !m_window->frameInsets().inset(frameRect()).contains(framePoint))
        return FrameArea::ResizeBorder;
    return FrameArea::None;
}

bool Decoration::setHoveredButton(int index)
{
    if (index == m_hoveredButton)
        return false;
    if (m_hoveredButton >= 0)
        m_buttons[m_hoveredButton].setHovered(false);
    m_hoveredButton = index;
    if (m_hoveredButton >= 0)
        m_buttons[m_hoveredButton].setHovered(true);
    scheduleRepaint();
    return true;
}

bool Decoration::onPointerMotion(input::Timestamp, const base::PointF &at)
{
    if (!m_window)
        return false;

    const base::Point framePoint = toFrame(at);
    if (m_pressedButton >= 0 && m_pressTouch)
        return true;
    if (m_pressedButton >= 0) {
        // A press follows the pointer: the button only stays lit while the
        // pointer is still on it.
        const bool inside = m_buttons[m_pressedButton].geometry().contains(framePoint);
        if (m_buttons[m_pressedButton].setHovered(inside))
            scheduleRepaint();
        return true;
    }

    setHoveredButton(buttonIndexAt(framePoint));
    return true;
}

bool Decoration::onPointerButton(input::Timestamp timestamp, const base::PointF &at, bool pressed)
{
    if (!m_window)
        return false;

    LM_DEBUG(lingmoInputTransitions) << "decoration target" << m_window
                                 << (pressed ? "press" : "release") << "caption-capture"
                                 << m_pressedButton;
    if (pressed)
        return press(timestamp, at, std::nullopt);
    // A button a touch point holds is released by that point.
    if (m_pressedButton < 0 || m_pressTouch)
        return false;
    releaseButton(toFrame(at));
    return true;
}

bool Decoration::onTouchDown(input::Timestamp timestamp, int32_t id, const base::PointF &at)
{
    if (!m_window)
        return false;
    LM_DEBUG(lingmoInputTransitions) << "decoration target" << m_window << "touch down" << id;
    return press(timestamp, at, id);
}

bool Decoration::onTouchMotion(input::Timestamp, int32_t id, const base::PointF &at)
{
    if (!m_window || m_pressedButton < 0 || m_pressTouch != id)
        return false;
    m_pressTouchPoint = toFrame(at);
    const bool inside = m_buttons[m_pressedButton].geometry().contains(m_pressTouchPoint);
    if (m_buttons[m_pressedButton].setHovered(inside))
        scheduleRepaint();
    return true;
}

bool Decoration::onTouchUp(input::Timestamp, int32_t id)
{
    if (!m_window || m_pressedButton < 0 || m_pressTouch != id)
        return false;
    m_pressTouch.reset();
    // No pointer is over the button a finger lets go of.
    setHoveredButton(-1);
    releaseButton(m_pressTouchPoint);
    return true;
}

void Decoration::onTouchCancel()
{
    if (m_pressTouch)
        cancelPress();
}

void Decoration::cancelPress()
{
    if (m_pressedButton < 0)
        return;
    m_pressTouch.reset();
    m_buttons[m_pressedButton].setPressed(false);
    m_pressedButton = -1;
    setHoveredButton(-1);
    scheduleRepaint();
}

bool Decoration::press(input::Timestamp timestamp, const base::PointF &location,
                       std::optional<int32_t> touchId)
{
    // One press at a time holds a button, whichever device made it.
    if (m_pressedButton >= 0)
        return true;

    const base::Point framePoint = toFrame(location);
    const int index = buttonIndexAt(framePoint);
    if (index >= 0) {
        m_pressedButton = index;
        m_pressTouch = touchId;
        m_pressTouchPoint = framePoint;
        m_buttons[index].setPressed(true);
        setHoveredButton(index);
        scheduleRepaint();
        return true;
    }

    // A touch on a resize border resizes at once, as ash starts one on the
    // tap-down of a resize component.
    const ResizeEdge edge = resizeEdgeAt(framePoint);
    if (edge != ResizeEdge::None) {
        m_window->manager()->eventHandler()->beginResize(m_window, edge, location, touchId);
        return true;
    }

    if (titleBarRect().contains(framePoint)) {
        const int slop = touchId ? kDoubleTapSlop : kDoubleClickSlop;
        const bool doubleClick = m_hadTitleBarPress
                && (timestamp - m_lastTitleBarPress) <= std::chrono::milliseconds(kDoubleClickMs)
                && std::abs(framePoint.x() - m_lastTitleBarPressPoint.x()) <= slop
                && std::abs(framePoint.y() - m_lastTitleBarPressPoint.y()) <= slop;
        m_lastTitleBarPress = timestamp;
        m_lastTitleBarPressPoint = framePoint;
        m_hadTitleBarPress = !doubleClick;

        LM_DEBUG(lingmoInputTransitions)
                << "decoration titlebar action" << (doubleClick ? "maximize-toggle" : "begin-move");
        if (doubleClick)
            toggleMaximize();
        else
            m_window->manager()->eventHandler()->beginMove(m_window, location, touchId);
        return true;
    }

    return true;
}

void Decoration::releaseButton(const base::Point &framePoint)
{
    const int index = m_pressedButton;
    m_pressedButton = -1;
    m_buttons[index].setPressed(false);
    scheduleRepaint();
    // A press that wandered off its button does nothing, as everywhere else.
    if (m_buttons[index].geometry().contains(framePoint))
        runButtonAction(m_buttons[index].type());
}

void Decoration::onPointerLeave()
{
    setHoveredButton(-1);
}

void Decoration::runButtonAction(CaptionButtonType type)
{
    // Every action is a request on the window, never anything done to the
    // client's process.
    switch (type) {
    case CaptionButtonType::Close:
        LM_DEBUG(lingmoAurora) << "decoration: close" << m_window->title();
        m_window->requestClose();
        break;
    case CaptionButtonType::Minimize:
        m_window->minimize();
        break;
    case CaptionButtonType::Maximize:
        toggleMaximize();
        break;
    }
}

void Decoration::toggleMaximize()
{
    if (m_window->state()->isMaximized())
        m_window->restore();
    else
        m_window->maximize();
}

void Decoration::onWindowBoundsChanged(wm::Window *window, const base::Rect &)
{
    if (window != m_window)
        return;
    layout();
    updateNode();
    scheduleRepaint();
}

void Decoration::onWindowStateTypeChanged(wm::Window *window, wm::WindowStateType)
{
    if (window != m_window)
        return;
    layout();
    updateNode();
    scheduleRepaint();
}

void Decoration::onWindowActivationChanged(wm::Window *window, bool)
{
    if (window != m_window)
        return;
    scheduleRepaint();
}

void Decoration::onWindowTitleChanged(wm::Window *window)
{
    if (window != m_window)
        return;
    scheduleRepaint();
}

void Decoration::onWindowDestroying(wm::Window *window)
{
    if (window != m_window)
        return;
    // The window is going; drop out of it before it walks its observers again.
    m_window->removeObserver(this);
    m_window = nullptr;
    if (m_node) {
        scene::Node *node = m_node;
        m_node = nullptr;
        if (node->parent())
            node->parent()->removeChild(node);
    }
}

////////////////////////////////////////////////////////////////////////////////
// DecorationProvider:

DecorationProvider::DecorationProvider(Theme *theme)
    : m_theme(theme)
{
}

std::unique_ptr<wm::WindowFrame> DecorationProvider::createFrame(wm::Window *window)
{
    return std::make_unique<Decoration>(window, m_theme);
}

wm::Insets DecorationProvider::defaultExtents() const
{
    return m_theme->metrics().insets();
}

} // namespace Lingmo::Decoration
