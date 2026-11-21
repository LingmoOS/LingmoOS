// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/window.h"
#include "display/display_manager.h"

#include <algorithm>
#include <utility>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "scene/node.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "wm/activation.h"
#include "wm/transient_manager.h"
#include "wm/window_manager.h"
#include "wm/window_observer.h"
#include "wm/window_state.h"
#include "wm/window_visual_controller.h"
#include "wm/wm_event.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

base::Rect Window::minimizedGeometry() const
{
    if (!m_minimizedGeometryPanel)
        return m_minimizedGeometry;
    for (const auto *panel : m_manager->windows()) {
        if (panel->id() == m_minimizedGeometryPanel)
            return display::DisplayManager::surfaceLocalToGlobal(m_minimizedGeometry,
                                                                 panel->contentBounds().topLeft());
    }
    return {};
}

void Window::setMinimizedGeometry(const base::Rect &geometry)
{
    m_minimizedGeometryPanel = 0;
    m_minimizedGeometry = geometry;
    m_state->updateMinimizeTarget();
}

void Window::setMinimizedGeometry(Window *panel, const base::Rect &geometry)
{
    m_minimizedGeometryPanel = panel ? panel->id() : 0;
    m_minimizedGeometry = panel ? geometry : base::Rect();
    m_state->updateMinimizeTarget();
}

namespace {

// Observers may remove themselves - or other observers - from inside a
// callback, so iterate a copy and re-check membership.
template <typename F>
void forEachObserver(Window *window, const std::vector<WindowObserver *> &observers, F &&fn)
{
    for (WindowObserver *observer : std::vector<WindowObserver *>(observers)) {
        if (window->hasObserver(observer))
            fn(observer);
    }
}

} // namespace

namespace {

// The scene node new windows are grouped under. Null in tests that only
// exercise protocol behavior.
scene::Node *surfaceContainer(WindowManager *manager)
{
    compositor::Compositor *compositor = manager ? manager->compositor() : nullptr;
    return compositor ? compositor->surfaceContainer() : nullptr;
}

scene::WindowAnimation::Style animationStyleFor(WindowRole role)
{
    switch (role) {
    case WindowRole::Launcher:
        return scene::WindowAnimation::Style::Launcher;
    case WindowRole::Desktop:
    case WindowRole::Osd:
    case WindowRole::Picker:
        return scene::WindowAnimation::Style::Fade;
    case WindowRole::Panel:
    case WindowRole::Dock:
    case WindowRole::Capture:
        return scene::WindowAnimation::Style::None;
    default:
        return scene::WindowAnimation::Style::Window;
    }
}

} // namespace

Window::Window(WindowManager *manager, compositor::Surface *rootSurface, const std::string &title,
               WindowType type)
    : m_type(type)
    , m_id([] {
        static uint64_t next = 0;
        return ++next;
    }())
    , m_manager(manager)
    , m_rootSurface(rootSurface)
    , m_title(title)
    , m_state(std::make_unique<WindowState>(this))
{
    if (m_rootSurface) {
        m_rootSurface->addSurfaceObserver(this);
        // Hints are usually set before the first commit, so before this window exists.
        m_surfaceAlwaysOnTop = m_rootSurface->alwaysOnTop();
        m_surfaceOpacity = m_rootSurface->opacity();
        if (isPopup())
            m_rootSurface->setHidden(false);
    }

    // One node per window, holding the frame and the client's surface tree, so
    // that stacking, hiding and moving a window is one operation on one node.
    if (scene::Node *container = surfaceContainer(m_manager)) {
        if (m_rootSurface && m_rootSurface->node()) {
            auto animation = std::make_unique<scene::WindowAnimation>();
            // A window placed against an anchor opens in place, like a popup.
            if (isPopup() || m_rootSurface->placementAnchor())
                animation->setStyle(scene::WindowAnimation::Style::Popup);
            animation->setOpacity(m_rootSurface->opacity());
            m_node = container->addChild(std::move(animation));
        }
    }
    m_visual = std::make_unique<WindowVisualController>(this);
    if (m_node)
        m_rootSurface->reparentNode(visualNode());
}

scene::Node *Window::visualNode() const
{
    return m_visual ? m_visual->node() : nullptr;
}

Window::~Window()
{
    if (isPopup() && m_rootSurface)
        m_rootSurface->setHidden(true);
    if (m_rootSurface && m_rootSurface->hasSurfaceObserver(this))
        m_rootSurface->removeSurfaceObserver(this);

    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node))
        animation->close();

    // The frame's node lives under m_node; drop it before the node goes.
    m_frame.reset();
    m_visual.reset();
    detachSurfaceNode();
    if (m_node) {
        scene::Node *node = m_node;
        m_node = nullptr;
        if (node->parent() && !dynamic_cast<scene::WindowAnimation *>(node))
            node->parent()->removeChild(node);
    }

    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowDestroying(this);
    });
    m_observers.clear();
}

void Window::onSurfaceDestroying(compositor::Surface *surface)
{
    if (surface != m_rootSurface)
        return;
    // The observer list is being walked by the surface; drop out of it before
    // this object goes away. The surface's node has to leave this window's node
    // first: the surface still refers to it, and destroying the window would
    // otherwise take it down with it.
    m_rootSurface->removeSurfaceObserver(this);
    detachSurfaceNode();
    m_rootSurface = nullptr;
    m_manager->destroyWindow(this);
}

void Window::onSurfaceCommit(compositor::Surface *surface)
{
    if (surface != m_rootSurface)
        return;
    // The surface applied its node geometry before notifying, so this is the
    // commit that made the new content current.
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowContentChanged(this);
    });
    updateSurfaceTargetBounds();
    placeSheets();
}

void Window::onSurfaceWindowHintsChanged(compositor::Surface *surface)
{
    if (surface != m_rootSurface)
        return;
    if (surface->bodyShape() != m_bodyShape || surface->castsShadow() != m_castsShadow) {
        m_bodyShape = surface->bodyShape();
        m_castsShadow = surface->castsShadow();
        if (m_visual)
            m_visual->update();
        forEachObserver(this, m_observers, [this](WindowObserver *observer) {
            observer->onWindowClientDecorationChanged(this);
        });
    }
    if (surface->opacity() != m_surfaceOpacity) {
        m_surfaceOpacity = surface->opacity();
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node))
            animation->setOpacity(m_opacity * m_surfaceOpacity);
    }
    // Every hint lands here; restacking for the others would raise the window
    // within its layer.
    if (surface->alwaysOnTop() == m_surfaceAlwaysOnTop)
        return;
    const bool wasAlwaysOnTop = isAlwaysOnTop();
    m_surfaceAlwaysOnTop = surface->alwaysOnTop();
    if (isAlwaysOnTop() != wasAlwaysOnTop && m_manager)
        m_manager->onWindowStackingChanged(this);
}

base::Rect Window::presentedBounds() const
{
    const base::Rect requested(base::Point(0, 0), m_bounds.size());
    const scene::Node *node = surfaceNode();
    if (!node || node->geometry().size().isEmpty())
        return requested;
    const auto insets = frameInsets();
    return base::Rect(m_surfaceOrigin, insets.outset(node->geometry().size()));
}

scene::Node *Window::surfaceNode() const
{
    return m_rootSurface ? static_cast<scene::Node *>(m_rootSurface->node()) : nullptr;
}

void Window::detachSurfaceNode()
{
    scene::Node *container = surfaceContainer(m_manager);
    if (!m_rootSurface || !m_node || !container)
        return;
    // Back in the container the position is an output coordinate again.
    m_rootSurface->reparentNode(container);
    m_rootSurface->setPosition(m_bounds.topLeft() + frameInsets().topLeft() + m_surfaceOrigin);
}

Insets Window::frameInsets() const
{
    // Fullscreen means the client owns the whole output; nothing is left for a
    // frame to occupy.
    if (!m_frame || m_state->isFullscreen())
        return Insets();
    return m_frame->extents();
}

base::Size Window::minimumOuterSize() const
{
    return frameInsets().outset(m_minimumSize);
}

base::Size Window::maximumOuterSize() const
{
    // Empty means unbounded, and stays that way.
    if (m_maximumSize.isEmpty())
        return m_maximumSize;
    return frameInsets().outset(m_maximumSize);
}

void Window::setDecorated(bool decorated)
{
    if (decorated == m_decorated)
        return;
    m_decorated = decorated;
    updateFrame();
    m_visual->update();
}

void Window::updateFrame()
{
    // Sampled before the frame changes: this is what the client keeps.
    const base::Rect content = contentBounds();

    FrameProvider *provider = m_manager ? m_manager->frameProvider() : nullptr;
    if (m_decorated && !m_frame && provider)
        m_frame = provider->createFrame(this);
    else if (!m_decorated && m_frame)
        m_frame.reset();
    else
        return;

    if (!m_bounds.isEmpty()) {
        // The client keeps the area it already has; the outer bounds grow or
        // shrink around it. A window whose bounds the state machine owns is
        // recomputed from the state instead.
        if (m_state->isNormal()) {
            setBounds(frameInsets().outset(content));
        } else {
            const WMEvent event(WMEventType::WorkAreaChanged);
            m_state->onEvent(event);
        }
    }

    updateSurfacePlacement();
    // Mapping logs the initial state; this is for a mode change later on.
    if (!m_bounds.isEmpty()) {
        LM_DEBUG(lingmoAurora) << "decoration" << (m_frame ? "on:" : "off:") << m_applicationId
                             << m_title << "outer" << m_bounds << "content"
                             << contentBounds();
    }
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowFrameChanged(this);
    });
}

WindowStateType Window::stateType() const
{
    return m_state->stateType();
}

void Window::setBounds(const base::Rect &bounds)
{
    if (bounds == m_bounds)
        return;

    const base::Rect oldBounds = m_bounds;
    m_bounds = bounds;
    updateSurfacePlacement();
    if (m_manager)
        m_manager->onWindowTargetBoundsChanged(this, oldBounds, bounds);

    forEachObserver(this, m_observers, [this, &oldBounds](WindowObserver *observer) {
        observer->onWindowBoundsChanged(this, oldBounds);
    });

    placeSheets();
    if (m_role == WindowRole::Sheet && bounds.size() != oldBounds.size())
        m_manager->placeSheet(this);
}

void Window::placeSheets()
{
    for (Window *child : std::vector<Window *>(m_transientChildren)) {
        if (child->role() == WindowRole::Sheet)
            m_manager->placeSheet(child);
    }
}

void Window::setSurfaceOrigin(const base::Point &origin)
{
    if (origin == m_surfaceOrigin)
        return;
    m_surfaceOrigin = origin;
    updateSurfacePlacement();
    // A resize anchored to a top or left edge moves the client's pixels inside
    // the window without the buffer changing.
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowContentChanged(this);
    });
    placeSheets();
}

void Window::setPresentationOffset(const base::Point &offset)
{
    if (offset == m_presentationOffset)
        return;
    m_presentationOffset = offset;
    updateSurfacePlacement();
}

void Window::setPresentationHidden(bool hidden)
{
    if (hidden == m_presentationHidden)
        return;
    m_presentationHidden = hidden;
    updateNodeVisibility();
}

void Window::updateNodeVisibility()
{
    if (m_node)
        m_node->setVisible(isVisible() && !m_presentationHidden);
}

void Window::updateSurfacePlacement()
{
    if (m_node)
        m_node->setGeometry(m_bounds.translated(m_presentationOffset));
    if (!m_rootSurface)
        return;
    // Only the origin crosses into the scene. The node's size stays the
    // committed buffer's until the client resizes; see the class comment.
    const base::Point origin = frameInsets().topLeft() + m_surfaceOrigin;
    m_rootSurface->setPosition(m_node ? origin : m_bounds.topLeft() + origin);
    updateSurfaceTargetBounds();
}

void Window::updateSurfaceTargetBounds()
{
    // The presentation offset is left out, as aura leaves a layer transform
    // out of a window's bounds.
    const scene::Node *node = surfaceNode();
    const base::Rect bounds(m_bounds.topLeft() + frameInsets().topLeft() + m_surfaceOrigin,
                            node ? node->geometry().size() : base::Size());
    const base::Rect oldBounds = std::exchange(m_surfaceTargetBounds, bounds);
    if (bounds != oldBounds && m_manager)
        m_manager->onWindowTargetBoundsChanged(this, oldBounds, bounds);
}

void Window::setMinimumSize(const base::Size &size)
{
    m_minimumSize = base::Size(std::max(1, size.width()), std::max(1, size.height()));
}

void Window::setMaximumSize(const base::Size &size)
{
    m_maximumSize = size;
}

void Window::setVisible(bool visible)
{
    if (visible == m_visible)
        return;
    const bool wasVisible = isVisible();
    m_visible = visible;
    onEffectiveVisibilityChanged(wasVisible);
    for (Window *child : std::vector<Window *>(m_transientChildren)) {
        if (child->isPopup() || child->role() == WindowRole::Sheet)
            child->setVisible(visible);
    }
    m_manager->onWindowModalityInputsChanged(this);
}

void Window::setDeskHidden(bool hidden)
{
    if (hidden == m_deskHidden)
        return;
    const bool wasVisible = isVisible();
    m_deskHidden = hidden;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowDeskHiddenChanged(this);
    });
    onEffectiveVisibilityChanged(wasVisible);
}

void Window::onEffectiveVisibilityChanged(bool wasVisible)
{
    const bool visible = isVisible();
    if (visible == wasVisible)
        return;
    updateNodeVisibility();
    if (m_rootSurface)
        m_rootSurface->setHidden(!visible);
    forEachObserver(this, m_observers, [this, visible](WindowObserver *observer) {
        observer->onWindowVisibilityChanged(this, visible);
    });
    if (!visible && m_active)
        m_manager->activation()->deactivate(this);
    m_manager->onWindowVisibilityChanged(this);
}

void Window::setHidden(bool hidden)
{
    if (hidden == m_hidden)
        return;
    m_hidden = hidden;
    setVisible(!m_hidden && !m_state->isMinimized());
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowHiddenChanged(this);
    });
}

void Window::setOpacity(float opacity)
{
    m_opacity = opacity;
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node))
        animation->setOpacity(m_opacity * m_surfaceOpacity);
}

void Window::setPresentationScale(double scale, const base::PointF &anchor, bool animate)
{
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node))
        animation->setPresentationScale(scale, anchor, animate);
}

void Window::setModal(bool modal)
{
    if (modal == m_modal)
        return;
    m_modal = modal;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowModalChanged(this);
    });
    m_manager->onWindowModalChanged(this);
}

void Window::setSystemDialog(bool systemDialog)
{
    if (systemDialog == m_systemDialog)
        return;
    m_systemDialog = systemDialog;
    if (m_manager)
        m_manager->onWindowStackingChanged(this);
}

bool Window::isAlwaysOnTop() const
{
    return m_alwaysOnTop || m_surfaceAlwaysOnTop;
}

void Window::setAlwaysOnTop(bool alwaysOnTop)
{
    if (alwaysOnTop == m_alwaysOnTop)
        return;
    m_alwaysOnTop = alwaysOnTop;
    if (m_manager)
        m_manager->onWindowStackingChanged(this);
}

bool Window::isActive() const
{
    return m_active;
}

void Window::activate()
{
    m_manager->activation()->activate(this);
}

void Window::setActiveInternal(bool active)
{
    if (active == m_active)
        return;
    m_active = active;
    forEachObserver(this, m_observers, [this, active](WindowObserver *observer) {
        observer->onWindowActivationChanged(this, active);
    });
}

void Window::setResizingInternal(bool resizing)
{
    if (resizing == m_resizing)
        return;
    m_resizing = resizing;
    forEachObserver(this, m_observers, [this, resizing](WindowObserver *observer) {
        observer->onWindowResizingChanged(this, resizing);
    });
}

void Window::setRestoringFromDragInternal(bool restoring)
{
    if (restoring == m_restoringFromDrag)
        return;
    m_restoringFromDrag = restoring;
    // The state type itself has not changed, but everything that keys off it -
    // the frame, the rounded corners, the shadow and the client's configure -
    // has to be re-evaluated.
    const WindowStateType type = m_state->stateType();
    forEachObserver(this, m_observers, [this, type](WindowObserver *observer) {
        observer->onWindowStateTypeChanged(this, type);
    });
}

void Window::setDraggedInternal(bool dragged)
{
    if (dragged == m_dragged)
        return;
    m_dragged = dragged;
    forEachObserver(this, m_observers, [this, dragged](WindowObserver *observer) {
        observer->onWindowDragStateChanged(this, dragged);
    });
}

void Window::setClientExteriorInsets(const Insets &insets)
{
    if (insets == m_clientExteriorInsets)
        return;
    m_clientExteriorInsets = insets;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowClientDecorationChanged(this);
    });
}

void Window::setClientInputInsets(const Insets &insets)
{
    if (insets == m_clientInputInsets)
        return;
    m_clientInputInsets = insets;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowClientDecorationChanged(this);
    });
}

void Window::setMinimizedSnapshot(const ui::Bitmap &snapshot)
{
    if (snapshot.isNull() && m_minimizedSnapshot.isNull())
        return;
    m_minimizedSnapshot = snapshot;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowMinimizedSnapshotChanged(this);
    });
}

void Window::beginMinimizeHandoff()
{
    auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node);
    if (!animation || !animation->isHolding() || !m_state->isMinimized())
        return;
    const uint64_t handoff = ++m_minimizeHandoff;
    const uint64_t id = m_id;
    m_manager->runLater(std::chrono::milliseconds(kMinimizeHandoffTimeoutMs),
                        [manager = m_manager, id, handoff] {
        Window *window = manager->windowById(id);
        if (window && window->m_minimizeHandoff == handoff)
            window->endMinimizeHandoff();
    });
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowMinimizeLanded(this);
    });
}

void Window::endMinimizeHandoff()
{
    ++m_minimizeHandoff;
    auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node);
    if (!animation || !animation->isHolding())
        return;
    animation->releaseHold();
    m_manager->restackScene();
    if (m_manager->compositor())
        m_manager->compositor()->scheduleRepaint();
}

void Window::setTitle(const std::string &title)
{
    if (title == m_title)
        return;
    m_title = title;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowTitleChanged(this);
    });
}

bool Window::skipTaskbar() const
{
    return isShellWindow() || isPopup()
            || (rootSurface() && rootSurface()->skipTaskbar());
}

void Window::setApplicationId(const std::string &id)
{
    if (id == m_applicationId)
        return;
    m_applicationId = id;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowApplicationIdChanged(this);
    });
}

void Window::setRole(WindowRole role)
{
    if (role == m_role)
        return;
    m_role = role;
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_node); animation && !isPopup()) {
        const bool anchored = m_rootSurface && m_rootSurface->placementAnchor();
        animation->setStyle(role == WindowRole::Normal && anchored ? scene::WindowAnimation::Style::Popup
                                                                   : animationStyleFor(role));
    }
    m_visual->update();
    // A shell window is placed where it asked to be, and a titlebar around it
    // would move it there plus an inset; it also never resizes interactively.
    if (m_role != WindowRole::Normal) {
        setDecorated(false);
        m_resizable = false;
    }
    if (m_manager)
        m_manager->onWindowStackingChanged(this);
}

void Window::setPanelBehavior(PanelBehavior behavior)
{
    if (behavior == m_panelBehavior)
        return;
    m_panelBehavior = behavior;
    if (m_manager)
        m_manager->onWindowStackingChanged(this);
}

void Window::setClientPosition(const base::Point &position)
{
    if (m_clientPosition == position && m_bounds.topLeft() == position)
        return;
    m_clientPosition = position;
    setBounds(base::Rect(position, m_bounds.size()));
}

void Window::setCloseCallback(CloseCallback callback)
{
    m_closeCallback = std::move(callback);
}

void Window::requestClose()
{
    // A client that stopped answering pings cannot act on a close request.
    if (!m_responsive && m_killCallback) {
        if (!m_manager->promptNotResponding(this))
            m_manager->forceQuit(this);
        return;
    }
    if (!m_closeCallback)
        return;
    // Asked first, while this window still exists: a client that hangs instead
    // of closing turns unresponsive, and the next close request kills it.
    ping();
    // A local copy: running the callback can destroy this window, and with it
    // the std::function the captures live in.
    const CloseCallback callback = m_closeCallback;
    callback();
}

void Window::setPingCallback(PingCallback callback)
{
    m_pingCallback = std::move(callback);
}

void Window::ping()
{
    if (!m_pingCallback || isShellWindow())
        return;
    m_pingCallback();
    if (!m_responsive)
        m_manager->promptNotResponding(this);
}

void Window::setResponsive(bool responsive)
{
    if (responsive == m_responsive)
        return;
    m_responsive = responsive;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowResponsiveChanged(this);
    });
}

void Window::setDemandsAttention(bool demandsAttention)
{
    if (demandsAttention == m_demandsAttention)
        return;
    m_demandsAttention = demandsAttention;
    forEachObserver(this, m_observers, [this](WindowObserver *observer) {
        observer->onWindowDemandsAttentionChanged(this);
    });
}

void Window::setKillCallback(KillCallback callback)
{
    m_killCallback = std::move(callback);
}

void Window::kill()
{
    if (!m_killCallback)
        return;
    // A local copy, as for the close callback.
    const KillCallback callback = m_killCallback;
    callback();
}

WindowStateType Window::presentedStateType() const
{
    return m_restoringFromDrag ? WindowStateType::Normal : m_state->stateType();
}

WindowStateType Window::decoratedStateType() const
{
    // Client exterior margins do not make the restored window body oversized.
    const base::Rect presented = m_clientExteriorInsets.inset(presentedBounds());
    if (m_restoringFromDrag
        && (presented.width() > m_bounds.width() || presented.height() > m_bounds.height())) {
        return m_state->stateType();
    }
    return presentedStateType();
}

void Window::maximize()
{
    const WMEvent event(WMEventType::Maximize);
    m_state->onEvent(event);
}

void Window::snapPrimary()
{
    const WMEvent event(WMEventType::SnapPrimary);
    m_state->onEvent(event);
}

void Window::snapSecondary()
{
    const WMEvent event(WMEventType::SnapSecondary);
    m_state->onEvent(event);
}

void Window::minimize()
{
    const WMEvent event(WMEventType::Minimize);
    m_state->onEvent(event);
}

void Window::restore()
{
    const WMEvent event(WMEventType::Restore);
    m_state->onEvent(event);
}

void Window::setFullscreen(bool fullscreen)
{
    if (fullscreen) {
        const WMEvent event(WMEventType::Fullscreen);
        m_state->onEvent(event);
    } else if (m_state->isFullscreen()) {
        const WMEvent event(WMEventType::Restore);
        m_state->onEvent(event);
    }
}

void Window::requestBounds(const base::Rect &bounds)
{
    const SetBoundsWMEvent event(bounds);
    m_state->onEvent(event);
}

bool Window::setTransientParent(Window *parent)
{
    return m_manager->transientManager()->setTransientParent(this, parent);
}

void Window::addObserver(WindowObserver *observer)
{
    if (!hasObserver(observer))
        m_observers.push_back(observer);
}

void Window::removeObserver(WindowObserver *observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                      m_observers.end());
}

bool Window::hasObserver(const WindowObserver *observer) const
{
    return std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end();
}

} // namespace Lingmo::WM
