// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/toplevel_drag.h"

#include <algorithm>

#include "clipboard/selection.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "input/hit_test.h"
#include "input/seat.h"
#include "scene/node.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

namespace {

scene::WindowAnimation *animationFor(Window *window)
{
    return dynamic_cast<scene::WindowAnimation *>(window->node());
}

// The pointer in `window`'s bounds, which the preview shrinks towards and
// grows back from.
base::PointF grabPoint(WindowManager *manager, Window *window)
{
    input::Seat *seat = manager->seat();
    return seat ? seat->pointerLocation() - base::PointF(window->bounds().topLeft()) : base::PointF();
}

double previewScale(Window *window)
{
    return std::min(ToplevelDrag::kPreviewMaxScale,
                    ToplevelDrag::kPreviewWidth / std::max(1, window->bounds().width()));
}

void settlePreview(WindowManager *manager, Window *preview)
{
    preview->setDragPreview(false);
    preview->setOpacity(1);
    preview->setPresentationScale(1, grabPoint(manager, preview), true);
}

} // namespace

ToplevelDrag::ToplevelDrag(WindowManager *manager, std::shared_ptr<clipboard::DataSource> source)
    : m_manager(manager)
    , m_source(std::move(source))
{
    if (input::Seat *seat = m_manager->seat()) {
        m_dragStarted = seat->drag()->started.connect([this] {
            onDragStarted();
        });
    }
    m_windowAdded = m_manager->windowAdded.connect([this](Window *window) {
        onWindowAdded(window);
    });
    m_windowRemoved = m_manager->windowRemoved.connect([this](Window *window) {
        onWindowRemoved(window);
    });
}

ToplevelDrag::~ToplevelDrag()
{
    // Only a client going away mid-drag gets here with the drag running; the
    // drag carries on without its window.
    if (input::Seat *seat = m_manager->seat()) {
        if (seat->drag()->windowDelegate() == this)
            seat->drag()->setWindowDelegate(nullptr);
    }
    unwatchDockTarget();
    stopMove();
    detach();
}

void ToplevelDrag::attach(compositor::Surface *surface, const base::Point &offset)
{
    detach();
    m_surface = surface;
    m_offset = offset;
    m_surface->addSurfaceObserver(this);
    // An existing window moves from where it is, as a caption drag would.
    if (Window *window = windowForSurface(); window && m_dragging)
        startMove(window);
}

void ToplevelDrag::onDragStarted()
{
    input::Seat *seat = m_manager->seat();
    if (m_dragging || !m_source || seat->drag()->source() != m_source)
        return;
    m_dragging = true;
    seat->drag()->setWindowDelegate(this);
    if (Window *window = windowForSurface())
        startMove(window);
}

void ToplevelDrag::onWindowAdded(Window *window)
{
    if (!m_surface || window->rootSurface() != m_surface)
        return;
    // Mapping it is the moment it appears under the pointer: with no open
    // animation, at the offset the client asked for.
    if (auto *animation = animationFor(window)) {
        m_animationStyle = animation->style();
        animation->setStyle(scene::WindowAnimation::Style::None);
    }
    if (input::Seat *seat = m_manager->seat()) {
        const base::Point origin = seat->pointerLocation().toPoint() - m_offset;
        window->setBounds(window->bounds().translated(origin - window->contentBounds().topLeft()));
    }
    if (m_dragging) {
        window->setDragPreview(true);
        window->setOpacity(kPreviewOpacity);
        window->setPresentationScale(previewScale(window), grabPoint(m_manager, window), false);
        m_preview = window->id();
        m_previewWidth = window->bounds().width();
        startMove(window);
    }
}

void ToplevelDrag::onWindowRemoved(Window *window)
{
    if (window == m_moving)
        m_moving = nullptr;
    if (!m_surface || window->rootSurface() != m_surface)
        return;
    detach();
    if (Window *target = m_manager->windowById(std::exchange(m_dropWindow, 0)))
        target->activate();
}

Window *ToplevelDrag::windowForSurface() const
{
    if (!m_surface)
        return nullptr;
    for (Window *window : m_manager->windows()) {
        if (window->rootSurface() == m_surface)
            return window;
    }
    return nullptr;
}

void ToplevelDrag::startMove(Window *window)
{
    input::Seat *seat = m_manager->seat();
    if (!seat || m_moving == window)
        return;
    stopMove();
    if (!m_manager->eventHandler()->beginMove(window, seat->pointerLocation())) {
        LM_DEBUG(lingmoAurora) << "toplevel drag could not move" << window;
        return;
    }
    m_moving = window;
    if (scene::Node *node = window->node())
        node->setEventTargetingBlocked(true);
}

void ToplevelDrag::stopMove()
{
    Window *window = std::exchange(m_moving, nullptr);
    if (!window)
        return;
    if (scene::Node *node = window->node())
        node->setEventTargetingBlocked(false);
    ToplevelEventHandler *handler = m_manager->eventHandler();
    if (handler->draggedWindow() == window)
        handler->completeDrag(ToplevelEventHandler::DragResult::Revert);
}

void ToplevelDrag::dragMoved(const base::PointF &location)
{
    ToplevelEventHandler *handler = m_manager->eventHandler();
    if (m_moving && handler->draggedWindow() == m_moving)
        handler->onPointerMotion(location);
}

void ToplevelDrag::dragTargetChanged(compositor::Surface *target, bool accepted)
{
    Window *preview = m_manager->windowById(m_preview);
    Window *below = accepted ? m_manager->windowForSurface(target) : nullptr;
    const bool docked = preview && below && below->processId() > 0
            && below->processId() == preview->processId();
    if (!docked) {
        unwatchDockTarget();
        m_docked = 0;
        if (preview)
            preview->setOpacity(kPreviewOpacity);
        return;
    }
    if (m_docked == below->id())
        return;
    unwatchDockTarget();
    m_docked = below->id();
    m_dockTarget = below->rootSurface();
    m_dockTarget->addSurfaceObserver(this);
}

void ToplevelDrag::onSurfaceCommit(compositor::Surface *surface)
{
    // A client that drew the state it mapped without at first resizes now;
    // the thumbnail keeps its width.
    if (Window *preview = m_manager->windowById(m_preview);
        preview && surface == m_surface && m_dragging
        && preview->bounds().width() != m_previewWidth) {
        m_previewWidth = preview->bounds().width();
        preview->setPresentationScale(previewScale(preview), grabPoint(m_manager, preview), false);
    }
    if (surface != m_dockTarget)
        return;
    unwatchDockTarget();
    if (Window *preview = m_manager->windowById(m_preview))
        preview->setOpacity(0);
}

void ToplevelDrag::unwatchDockTarget()
{
    if (m_dockTarget && m_dockTarget != m_surface)
        m_dockTarget->removeSurfaceObserver(this);
    m_dockTarget = nullptr;
}

void ToplevelDrag::dragEnded(bool released)
{
    m_dragging = false;
    unwatchDockTarget();
    // Let go where nothing takes it, the preview is the new window at once.
    // Otherwise its client takes it back: hidden until it has, and shown only
    // if it keeps it after all. The drag object may be gone by then.
    const bool docked = std::exchange(m_docked, 0) != 0;
    if (Window *preview = m_manager->windowById(std::exchange(m_preview, 0))) {
        if (released && !docked) {
            settlePreview(m_manager, preview);
            preview->activate();
        } else {
            preview->setOpacity(0);
            m_manager->runLater(std::chrono::milliseconds(kSettleDelayMs),
                                [manager = m_manager, id = preview->id()] {
                if (Window *kept = manager->windowById(id))
                    settlePreview(manager, kept);
            });
        }
    }
    Window *window = std::exchange(m_moving, nullptr);
    // Looked up while the dragged window still takes no part in the hit test.
    if (window && released) {
        if (input::Seat *seat = m_manager->seat()) {
            base::PointF local;
            Window *below = m_manager->windowForSurface(input::findTargetSurface(
                    m_manager->compositor(), seat->pointerLocation(), &local));
            if (below && below != window && below->processId() > 0
                && below->processId() == window->processId())
                m_dropWindow = below->id();
        }
    }
    if (window) {
        if (scene::Node *node = window->node())
            node->setEventTargetingBlocked(false);
        ToplevelEventHandler *handler = m_manager->eventHandler();
        if (handler->draggedWindow() == window) {
            handler->completeDrag(released ? ToplevelEventHandler::DragResult::Success
                                           : ToplevelEventHandler::DragResult::Revert);
        }
    }
    if (Window *attached = windowForSurface(); attached && m_animationStyle) {
        if (auto *animation = animationFor(attached))
            animation->setStyle(*m_animationStyle);
    }
    m_animationStyle.reset();
}

void ToplevelDrag::onSurfaceDestroying(compositor::Surface *surface)
{
    if (surface == m_dockTarget)
        m_dockTarget = nullptr;
    if (surface == m_surface)
        detach();
}

void ToplevelDrag::detach()
{
    if (!m_surface)
        return;
    if (Window *window = windowForSurface(); window && window == m_moving)
        stopMove();
    if (Window *window = windowForSurface(); window && m_animationStyle) {
        if (auto *animation = animationFor(window))
            animation->setStyle(*m_animationStyle);
    }
    m_animationStyle.reset();
    // The dock watch shares this registration when it is on this surface.
    if (m_dockTarget == m_surface)
        m_dockTarget = nullptr;
    m_surface->removeSurfaceObserver(this);
    m_surface = nullptr;
}

} // namespace Lingmo::WM
