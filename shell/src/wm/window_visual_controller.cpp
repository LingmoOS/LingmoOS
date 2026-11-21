// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/window_visual_controller.h"

#include <algorithm>

#include "compositor/surface.h"
#include "decoration/window_visual_style.h"
#include "ui/geometry_conversions.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_modality_controller.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

namespace {

// A glass rim is the edge itself, so the outline stays transparent under it
// rather than going away: the shadow still follows the outline's shape.
void setShellOutlineColors(scene::WindowVisualState &state, const Window &window,
                           const decoration::WindowVisualStyle &style)
{
    const auto *surface = window.rootSurface();
    if (surface && surface->glass().enabled && surface->glass().rim > 0) {
        state.outlineColor = {0, 0, 0, 0};
        state.highlightColor = {0, 0, 0, 0};
        return;
    }
    const bool dock = window.role() == WindowRole::Dock;
    state.outlineColor = dock ? style.dockOutline : style.shellOutline;
    state.highlightColor = dock ? style.dockHighlight : style.highlight;
}

} // namespace

scene::WindowVisualState
WindowVisualController::evaluate(const Window &window, const decoration::WindowVisualStyle &style)
{
    using Mode = scene::WindowVisualMode;
    scene::WindowVisualState state;
    state.shape.bounds = base::RectF(window.presentedBounds());
    state.surfaceOrigin = window.frameInsets().topLeft() + window.surfaceOrigin();
    // A sheet is a dialog: clipped and outlined like an application window.
    if (window.isPopup()
        || (window.role() != WindowRole::Normal && window.role() != WindowRole::Sheet)) {
        // The status bar has no body: its blur is only behind the plate of an item.
        if (!window.isPopup() && window.role() == WindowRole::Panel)
            return state;
        // Menus and shell panels describe their rounded body through the blur request.
        const auto *surface = window.rootSurface();
        const base::Point origin = state.surfaceOrigin;
        const ui::Path path = surface ? surface->backgroundBlurShape() : ui::Path();
        if (!path.isEmpty()) {
            state.outlinePath = path.translated(origin.x(), origin.y());
            state.outlineShape = {ui::toBase(state.outlinePath.boundingRect()), 0};
            state.drawOutline = true;
            state.outlineWidthPx = style.outlineWidthPx;
            setShellOutlineColors(state, window, style);
            return state;
        }
        // A body the client outlines itself casts the shadow, unoutlined.
        if (surface && !surface->bodyShape().isEmpty()) {
            state.outlinePath = surface->bodyShape().translated(origin.x(), origin.y());
            state.outlineShape = {ui::toBase(state.outlinePath.boundingRect()), 0};
            return state;
        }
        const base::Rect body = surface
                ? surface->backgroundBlurRect().intersected(base::Rect(surface->contentSize()))
                : base::Rect();
        if (!body.isEmpty() && surface->backgroundBlurRadius() > 0) {
            const double radius = std::min({double(surface->backgroundBlurRadius()),
                                           body.width() / 2.0, body.height() / 2.0});
            state.outlineShape = {base::RectF(body.translated(origin)), radius};
            state.drawOutline = true;
            state.outlineWidthPx = style.outlineWidthPx;
            setShellOutlineColors(state, window, style);
        }
        return state;
    }
    // A body the client outlines itself: kept as drawn, its shadow cast by it.
    const auto *root = window.rootSurface();
    if (!window.isDecorated() && root && !root->bodyShape().isEmpty()) {
        const base::Point origin = state.surfaceOrigin;
        state.outlinePath = root->bodyShape().translated(origin.x(), origin.y());
        state.outlineShape = {ui::toBase(state.outlinePath.boundingRect()), 0};
        return state;
    }
    if (window.isDecorated())
        state.mode = Mode::ServerFrame;
    else
        state.mode = Mode::ServerExterior;
    const auto exterior = window.clientExteriorInsets();
    state.shape.bounds = state.shape.bounds.adjusted(exterior.left, exterior.top, -exterior.right,
                                                     -exterior.bottom);
    // Exterior pixels stay hidden even when the body has square, tiled edges.
    state.clipContent = !exterior.isNull();
    if (isRoundedStateType(window.decoratedStateType())) {
        state.shape.radius = style.radiusFor(state.shape.bounds.size().toSize());
        state.clipContent = true;
        state.drawOutline = true;
        state.outlineWidthPx = style.outlineWidthPx;
    }
    state.outlineColor = window.isActive() ? style.activeOutline : style.inactiveOutline;
    state.highlightColor = style.highlight;
    if (modalTransient(&window) || !window.isResponsive())
        state.dimOpacity = style.dimOpacity;
    return state;
}

scene::WindowVisualState WindowVisualController::evaluate(const Window &window)
{
    return evaluate(window, decoration::WindowVisualStyle());
}

WindowVisualController::WindowVisualController(Window *window)
    : m_window(window)
{
    if (window->node())
        m_node = static_cast<scene::WindowVisualNode *>(
                window->node()->addChild(std::make_unique<scene::WindowVisualNode>()));
    window->addObserver(this);
    update();
}

WindowVisualController::~WindowVisualController()
{
    m_window->removeObserver(this);
}

void WindowVisualController::update()
{
    if (m_node)
        m_node->setState(evaluate(*m_window, m_window->manager()->visualStyle()));
}

void WindowVisualController::refresh(Window *window)
{
    if (auto *node = window ? dynamic_cast<scene::WindowVisualNode *>(window->visualNode()) : nullptr)
        node->setState(evaluate(*window, window->manager()->visualStyle()));
}

void WindowVisualController::onWindowVisibilityChanged(Window *window, bool)
{
    refresh(window->transientParent());
}

void WindowVisualController::onWindowModalChanged(Window *window)
{
    refresh(window->transientParent());
}

void WindowVisualController::onWindowTransientParentChanged(Window *window, Window *oldParent)
{
    refresh(oldParent);
    refresh(window->transientParent());
}

} // namespace Lingmo::WM
