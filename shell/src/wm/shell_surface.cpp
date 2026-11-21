// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/shell_surface_base.cc
//           chromium/src/components/exo/shell_surface.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wm/shell_surface.h"

#include <algorithm>
#include <cassert>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "wm/activation.h"
#include "wm/placement.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_frame.h"
#include "wm/window_manager.h"
#include "wm/window_positioner.h"
#include "wm/window_state.h"
#include "wm/wm_event.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

////////////////////////////////////////////////////////////////////////////////
// ShellSurface, ScopedConfigure:

ShellSurface::ScopedConfigure::ScopedConfigure(ShellSurface *shellSurface, bool forceConfigure)
    : m_shellSurface(shellSurface)
    , m_alive(shellSurface->m_self)
    , m_forceConfigure(forceConfigure)
{
    // Instances cannot be nested.
    assert(!m_shellSurface->m_scopedConfigure);
    m_shellSurface->m_scopedConfigure = this;
}

ShellSurface::ScopedConfigure::~ScopedConfigure()
{
    // The body that opened this scope may have destroyed the shell surface.
    if (!*m_alive)
        return;

    m_shellSurface->m_scopedConfigure = nullptr;
    if (m_needsConfigure || m_forceConfigure)
        m_shellSurface->configure();
    if (*m_alive && m_shellSurface->m_window) {
        m_shellSurface->updateWindowBoundsFromClient();
        m_shellSurface->updateFromWindowGeometry();
    }
}

////////////////////////////////////////////////////////////////////////////////
// ShellSurface, public:

ShellSurface::ShellSurface(WindowManager *manager, compositor::Surface *surface)
    : m_self(std::make_shared<ShellSurface *>(this))
    , m_manager(manager)
    , m_surface(surface)
{
    if (m_surface)
        m_surface->addSurfaceObserver(this);
}

ShellSurface::~ShellSurface()
{
    // Anything still on the stack below a client callback learns from this that
    // it must not touch this object again.
    *m_self = nullptr;
    unmapChildPopups();

    // Out of every registry the rest of the compositor can reach before
    // anything is told this object is going away.
    if (m_parentShell)
        m_parentShell->removeChildShell(this);
    for (ShellSurface *child : std::vector<ShellSurface *>(m_childShells))
        child->m_parentShell = nullptr;
    m_childShells.clear();
    detachTransientRelationships();

    if (m_manager) {
        m_manager->removePopupGrab(this);
        if (m_surface)
            m_manager->clearSurfaceOwner(m_surface);
    }

    if (m_surface && m_surface->hasSurfaceObserver(this))
        m_surface->removeSurfaceObserver(this);

    if (m_window) {
        Window *window = m_window;
        m_window = nullptr;
        window->removeObserver(this);
        m_manager->destroyWindow(window);
    }
}

void ShellSurface::setConfigureCallback(ConfigureCallback callback)
{
    m_configureCallback = std::move(callback);
}

void ShellSurface::setPopupConfigureCallback(PopupConfigureCallback callback)
{
    m_popupConfigureCallback = std::move(callback);
}

void ShellSurface::setCloseCallback(CloseCallback callback)
{
    m_closeCallback = std::move(callback);
}

void ShellSurface::setPingCallback(PingCallback callback)
{
    m_pingCallback = std::move(callback);
}

void ShellSurface::setKillCallback(KillCallback callback)
{
    m_killCallback = std::move(callback);
}

bool ShellSurface::setToplevel()
{
    if (m_role != Role::None || !m_surface)
        return false;
    m_role = Role::Toplevel;
    m_surface->setHasRole(true);
    return true;
}

bool ShellSurface::setPopup(ShellSurface *parent, const base::Rect &bounds)
{
    if (m_role != Role::None || !m_surface || !parent || parent == this)
        return false;

    // A popup may not be its own ancestor.
    for (ShellSurface *ancestor = parent; ancestor; ancestor = ancestor->m_parentShell) {
        if (ancestor == this)
            return false;
    }

    m_role = Role::Popup;
    m_surface->setHasRole(true);
    m_parentShell = parent;
    parent->addChildShell(this);
    m_popupBounds = bounds;
    return true;
}

void ShellSurface::clearRole()
{
    if (m_role == Role::None)
        return;

    if (m_role == Role::Popup) {
        if (m_popupMapped)
            unmapPopup();
        if (m_parentShell) {
            m_parentShell->removeChildShell(this);
            m_parentShell = nullptr;
        }
        m_popupBounds = base::Rect();
    } else {
        if (m_window)
            unmapToplevel();
        detachTransientRelationships();
    }

    m_role = Role::None;
    if (m_surface)
        m_surface->setHasRole(false);
    // The role object took its decoration object with it, if it had one.
    m_hasDecorationObject = false;
    m_decorationRemovalPending = false;
    m_committed.reset();
    m_requestedMode.reset();
    m_hasDialogObject = false;
    m_modal = false;
    resetConfigureState();
    m_initialStateType = WindowStateType::Normal;
    m_initialStateInherited = false;
    m_initialOutput = base::Rect();
}

Window *ShellSurface::ownerWindow() const
{
    for (const ShellSurface *shell = this; shell; shell = shell->m_parentShell) {
        if (shell->m_role != Role::Popup && shell->m_window)
            return shell->m_window;
    }
    return nullptr;
}

void ShellSurface::setGeometry(const base::Rect &geometry)
{
    // Upstream refuses an empty geometry outright; an explicitly empty one is
    // how a client goes back to using its own content bounds.
    m_pendingGeometry = geometry.isEmpty() ? base::Rect() : geometry;
}

base::Rect ShellSurface::visibleBounds() const
{
    if (!m_geometry.isEmpty())
        return m_geometry;
    return base::Rect(base::Point(0, 0), m_surface ? m_surface->contentSize() : base::Size());
}

bool ShellSurface::acknowledgeConfigure(uint32_t serial)
{
    const auto match = std::find_if(m_pendingConfigs.begin(), m_pendingConfigs.end(),
                                    [serial](const Config &config) {
                                        return config.serial == serial;
                                    });
    // Never sent, or already superseded by a later acknowledgement.
    if (match == m_pendingConfigs.end())
        return false;

    // Apply every config older than or equal to `serial`. The surface origin
    // and the resize direction move at the next commit, not here.
    while (!m_pendingConfigs.empty()) {
        const Config config = m_pendingConfigs.front();
        m_pendingConfigs.pop_front();
        m_pendingOriginOffset += config.originOffset;
        m_pendingResizeEdge = config.resizeEdge;
        m_acknowledgedConfigure = config.state;
        if (config.popupBounds)
            m_pendingPopupBounds = config.popupBounds;
        if (config.serial == serial)
            break;
    }
    return true;
}

void ShellSurface::setTitle(const std::string &title)
{
    m_title = title;
    if (m_window)
        m_window->setTitle(title);
}

void ShellSurface::setApplicationId(const std::string &id)
{
    m_applicationId = id;
    if (m_window)
        m_window->setApplicationId(id);
}

void ShellSurface::setMinimumSize(const base::Size &size)
{
    m_pendingMinimumSize = size;
}

void ShellSurface::setMaximumSize(const base::Size &size)
{
    m_pendingMaximumSize = size;
}

void ShellSurface::setModal(bool modal)
{
    m_modal = modal;
    if (m_window)
        m_window->setModal(modal);
}

bool ShellSurface::isPositionManaged() const
{
    return m_role == Role::Toplevel && !m_transientParentShell
            && !m_manager->shellMetadata(m_surface);
}

bool ShellSurface::setTransientParent(ShellSurface *parent)
{
    // A client that parents to itself or to one of its own ancestors would
    // otherwise lock the transient chain into a cycle.
    for (ShellSurface *ancestor = parent; ancestor; ancestor = ancestor->m_transientParentShell) {
        if (ancestor == this)
            return false;
    }

    // Upstream: exo ShellSurface::SetParent takes an unmapped parent's missing
    // widget as no parent, and nothing restores it later.
    if (parent && (parent->m_role != Role::Toplevel || !parent->m_window))
        parent = nullptr;
    linkTransientParent(parent);
    return true;
}

void ShellSurface::linkTransientParent(ShellSurface *parent)
{
    if (parent != m_transientParentShell) {
        if (m_transientParentShell)
            std::erase(m_transientParentShell->m_transientChildShells, this);
        m_transientParentShell = parent;
        if (parent)
            parent->m_transientChildShells.push_back(this);
    }
    if (m_window)
        m_window->setTransientParent(parent ? parent->window() : nullptr);
}

void ShellSurface::releaseTransientChildren()
{
    for (ShellSurface *child : std::vector<ShellSurface *>(m_transientChildShells))
        child->linkTransientParent(m_transientParentShell);
}

void ShellSurface::detachTransientRelationships()
{
    releaseTransientChildren();
    if (m_transientParentShell) {
        std::erase(m_transientParentShell->m_transientChildShells, this);
        m_transientParentShell = nullptr;
    }
}

void ShellSurface::setHasDecorationObject(bool has)
{
    if (has == m_hasDecorationObject)
        return;
    m_hasDecorationObject = has;
    m_decorationRemovalPending = !has;
    if (has) {
        applyDecorationMode();
        return;
    }
    // xdg-decoration: the frame goes at the next commit, not now. Clients
    // destroy the object right before the toplevel, and the close animation
    // must still show the frame.
    m_requestedMode.reset();
}

void ShellSurface::setRequestedDecorationMode(std::optional<DecorationMode> mode)
{
    if (mode == m_requestedMode)
        return;
    m_requestedMode = mode;
    applyDecorationMode();
}

ShellSurface::DecorationMode ShellSurface::decorationMode() const
{
    return decorationModeFor(m_requestedMode);
}

ShellSurface::DecorationMode
ShellSurface::decorationModeFor(std::optional<DecorationMode> requested) const
{
    // A client that never asked to negotiate decorates itself, whatever the
    // compositor would prefer.
    if (!m_hasDecorationObject)
        return DecorationMode::ClientSide;
    return requested.value_or(kPreferredDecorationMode);
}

void ShellSurface::applyDecorationMode()
{
    if (!m_window)
        return;
    // The frame appearing or disappearing resizes the client area and changes
    // the state the client is in; the protocol asks for a configure either way,
    // and this keeps it to exactly one.
    ScopedConfigure scoped(this, /*forceConfigure=*/true);
    m_window->setDecorated(decorationMode() == DecorationMode::ServerSide);
}

void ShellSurface::maximize()
{
    if (!m_window) {
        m_initialStateInherited = false;
        m_initialStateType = WindowStateType::Maximized;
        return;
    }
    ScopedConfigure scoped(this, false);
    m_window->maximize();
}

void ShellSurface::unmaximize()
{
    if (!m_window) {
        m_initialStateInherited = false;
        if (m_initialStateType == WindowStateType::Maximized)
            m_initialStateType = WindowStateType::Normal;
        return;
    }
    ScopedConfigure scoped(this, false);
    if (m_window->state()->isMaximized())
        m_window->restore();
}

void ShellSurface::minimize()
{
    if (!m_window) {
        m_initialStateInherited = false;
        m_initialStateType = WindowStateType::Minimized;
        return;
    }
    ScopedConfigure scoped(this, false);
    m_window->minimize();
}

void ShellSurface::setFullscreen(bool fullscreen)
{
    if (!m_window) {
        m_initialStateInherited = false;
        if (fullscreen)
            m_initialStateType = WindowStateType::Fullscreen;
        else if (m_initialStateType == WindowStateType::Fullscreen)
            m_initialStateType = WindowStateType::Normal;
        if (!fullscreen)
            m_fullscreenOutputName.clear();
        return;
    }
    ScopedConfigure scoped(this, false);
    if (fullscreen && !m_fullscreenOutputName.empty())
        m_manager->setWindowOutput(m_window, m_fullscreenOutputName);
    m_window->setFullscreen(fullscreen);
    if (!fullscreen) {
        m_fullscreenOutputName.clear();
        m_manager->clearWindowOutput(m_window);
    }
}

void ShellSurface::dropInheritedState()
{
    if (m_window || !m_initialStateInherited)
        return;
    m_initialStateInherited = false;
    m_initialStateType = WindowStateType::Normal;
    if (m_initialConfigureSent)
        configure();
}

void ShellSurface::setFullscreenOutput(const std::string &waylandName)
{
    m_fullscreenOutputName = waylandName;
    if (!waylandName.empty() && m_window && m_window->stateType() == WindowStateType::Fullscreen) {
        m_manager->setWindowOutput(m_window, waylandName);
        m_window->state()->onEvent(WMEvent(WMEventType::WorkAreaChanged));
    }
}

bool ShellSurface::startMove(bool byTouch)
{
    if (!m_window || !m_manager->seat())
        return false;
    input::Seat *seat = m_manager->seat();
    if (byTouch) {
        const auto point = seat->touchPointOn(m_window->rootSurface());
        return point && m_manager->eventHandler()->beginMove(m_window, point->location, point->id);
    }
    return m_manager->eventHandler()->beginMove(m_window, seat->pointerLocation());
}

bool ShellSurface::startResize(ResizeEdge edge, bool byTouch)
{
    if (!m_window || !m_manager->seat())
        return false;
    input::Seat *seat = m_manager->seat();
    if (byTouch) {
        const auto point = seat->touchPointOn(m_window->rootSurface());
        return point
                && m_manager->eventHandler()->beginResize(m_window, edge, point->location, point->id);
    }
    return m_manager->eventHandler()->beginResize(m_window, edge, seat->pointerLocation());
}

void ShellSurface::close()
{
    if (!m_closeCallback)
        return;
    // The callback owns its captures, and running it can destroy this object -
    // a client that tears its menu chain down when it is told the popup is
    // gone. A local copy keeps the callable alive for the call.
    const CloseCallback callback = m_closeCallback;
    callback();
}

void ShellSurface::grab(const void *client)
{
    if (m_role != Role::Popup)
        return;
    m_hasGrab = true;
    m_grabClient = client;
    m_manager->pushPopupGrab(this);
}

void ShellSurface::reposition(const base::Rect &bounds)
{
    if (m_role != Role::Popup)
        return;
    if (m_popupConfigureCallback) {
        sendPopupConfigure(bounds, true);
        return;
    }
    m_popupBounds = bounds;
    updatePopupPosition();
}

void ShellSurface::configurePopup()
{
    if (m_role != Role::Popup || !m_popupConfigureCallback)
        return;
    sendPopupConfigure(m_popupBounds, false);
}

void ShellSurface::sendPopupConfigure(const base::Rect &bounds, bool reposition)
{
    const std::shared_ptr<ShellSurface *> self = m_self;
    const PopupConfigureCallback callback = m_popupConfigureCallback;
    const uint32_t serial = callback(bounds);
    if (!*self)
        return;
    const std::optional<base::Rect> popupBounds =
            reposition ? std::optional<base::Rect>(bounds) : std::nullopt;
    if (serial) {
        m_pendingConfigs.push_back(Config {serial, base::Point(), ResizeEdge::None, {}, popupBounds});
    } else if (popupBounds) {
        m_popupBounds = *popupBounds;
        updatePopupPosition();
    }
}

base::Point ShellSurface::absoluteOrigin() const
{
    if (m_role == Role::Popup && m_parentShell)
        return m_parentShell->absoluteOrigin() + m_popupBounds.topLeft();
    // A popup's coordinates are relative to its parent's window geometry, which
    // starts inside the frame, not at the outer bounds.
    if (m_window)
        return m_window->contentBounds().topLeft();
    return base::Point();
}

////////////////////////////////////////////////////////////////////////////////
// compositor::SurfaceObserver overrides:

void ShellSurface::onSurfaceDestroying(compositor::Surface *surface)
{
    if (surface != m_surface)
        return;

    unmapChildPopups();

    // Off every registry first; the teardown below re-enters the window manager
    // and nothing must be able to find a half-destroyed surface through it.
    m_surface->removeSurfaceObserver(this);
    if (m_manager) {
        m_manager->clearSurfaceOwner(m_surface);
        m_manager->removePopupGrab(this);
    }
    m_surface = nullptr;
    m_hasGrab = false;
    m_popupMapped = false;

    if (m_window) {
        Window *window = m_window;
        m_window = nullptr;
        window->removeObserver(this);
        m_manager->destroyWindow(window);
    }

    // Popups outlive their parent's surface as protocol objects; they simply
    // have nowhere left to be placed.
    for (ShellSurface *child : std::vector<ShellSurface *>(m_childShells))
        child->updatePopupPosition();
}

void ShellSurface::onSurfaceCommit(compositor::Surface *surface)
{
    if (surface != m_surface || m_role == Role::None)
        return;

    if (!m_committed)
        return;
    const CommittedState committed = *std::exchange(m_committed, std::nullopt);

    // Send at most one configure for whatever this commit set in motion.
    ScopedConfigure scoped(this, false);

    applyCommittedState(committed);

    if (committed.decorationRemoval) {
        if (m_window) {
            m_window->setDecorated(decorationMode() == DecorationMode::ServerSide);
            scoped.setNeedsConfigure();
        }
    }

    const bool hasContents = m_surface->hasContents();

    if (m_role == Role::Popup) {
        if (hasContents && !m_popupMapped)
            mapPopup();
        else if (!hasContents && m_popupMapped)
            unmapPopup();
        else if (m_popupMapped)
            updatePopupPosition();
        return;
    }

    if (!m_window) {
        if (!hasContents) {
            // xdg-shell: the first commit carries no buffer and asks for the
            // initial configure. The client attaches only after it acks.
            if (!m_initialConfigureSent) {
                m_initialConfigureSent = true;
                m_initialOutput = isPositionManaged() ? m_manager->outputForNewWindow() : base::Rect();
                if (isPositionManaged() && computeResizable()) {
                    const WindowStateType requested = m_initialStateType;
                    m_initialStateType = window_positioner::showStateForNewWindow(
                            m_manager, m_initialOutput, m_applicationId, requested);
                    m_initialStateInherited = m_initialStateType != requested;
                }
                configure();
            }
            return;
        }
        mapToplevel();
        return;
    }

    if (!hasContents) {
        unmapToplevel();
        return;
    }

    // Qt's render thread can attach an old-size frame after the configure ack.
    const bool resized = m_boundsTransitionOldContentSize.isEmpty()
            || surface->contentSize() != m_boundsTransitionOldContentSize;
    const auto &ack = committed.acknowledgedConfigure;
    const bool acknowledged = !m_configureCallback
            || (ack && ack->size == m_window->contentBounds().size()
                && ack->stateType == m_window->presentedStateType());
    if (acknowledged && surface->lastCommitAttachedBuffer() && resized) {
        if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_window->node()))
            animation->setBoundsContentReady(true);
    }
    updateChildPopupPositions();
}

////////////////////////////////////////////////////////////////////////////////
// WindowObserver overrides:

void ShellSurface::onWindowBoundsChanged(Window *window, const base::Rect &)
{
    if (window != m_window)
        return;

    if (m_role == Role::Popup) {
        updateFromWindowGeometry();
        updateChildPopupPositions();
        return;
    }

    // In client-area terms: the outer bounds also move when a frame appears or
    // disappears, and that is not a resize as far as the client is concerned.
    const base::Rect newContent = window->contentBounds();
    const base::Rect oldContent = m_configuredContentBounds;
    m_configuredContentBounds = newContent;

    // Without a configure callback (the compositor's own windows) nothing
    // acknowledges a size, so there is no in-flight content to hold in place.
    if (newContent.size() != oldContent.size() && !m_applyingClientBounds && m_configureCallback) {
        // The window manager has already moved and resized the window; hold the
        // client's current contents where they are until it catches up. The
        // offset is handed back at the commit that follows the acknowledgement.
        const base::Point delta = newContent.topLeft() - oldContent.topLeft();
        m_originOffset -= delta;
        m_pendingOriginOffsetAccumulator += delta;
        configure();
    }

    updateFromWindowGeometry();
    updateChildPopupPositions();
}

void ShellSurface::onWindowFrameChanged(Window *window)
{
    if (window != m_window)
        return;
    updateFromWindowGeometry();
    configure();
}

void ShellSurface::onWindowHiddenChanged(Window *window)
{
    if (window != m_window)
        return;
    configure();
}

void ShellSurface::onWindowDeskHiddenChanged(Window *window)
{
    if (window != m_window)
        return;
    configure();
}

void ShellSurface::onWindowStateTypeChanged(Window *window, WindowStateType oldType)
{
    if (window != m_window)
        return;
    LM_DEBUG(lingmoAurora) << "xdg toplevel state:" << m_applicationId << toString(oldType) << "->"
                         << toString(window->stateType()) << window->bounds();
    const base::Size targetSize = window->contentBounds().size();
    const auto targetState = window->presentedStateType();
    // Drag completion repeats the restored state without requiring another frame.
    const bool sameTarget = m_boundsTransitionTarget && m_boundsTransitionTarget->size == targetSize
            && m_boundsTransitionTarget->stateType == targetState;
    if (sameTarget) {
        configure();
        return;
    }
    m_boundsTransitionTarget = ConfigureState {.size = targetSize, .stateType = targetState};
    m_boundsTransitionOldContentSize = window->contentBounds().size() != visibleBounds().size()
                    && window->contentBounds().size() != m_surface->contentSize()
            ? m_surface->contentSize()
            : base::Size();
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(window->node()))
        animation->setBoundsContentReady(!m_configureCallback);
    configure();
}

void ShellSurface::onWindowActivationChanged(Window *window, bool)
{
    if (window != m_window)
        return;
    configure();
}

void ShellSurface::onWindowResizingChanged(Window *window, bool)
{
    if (window != m_window)
        return;
    configure();
}

void ShellSurface::onWindowDestroying(Window *window)
{
    if (window != m_window)
        return;
    m_window = nullptr;
    window->removeObserver(this);
    resetConfigureState();
}

////////////////////////////////////////////////////////////////////////////////
// ShellSurface, private:

void ShellSurface::configure()
{
    // Defer while a commit is in progress, unless the surface is not mapped
    // yet: the initial configure has to go out on its own.
    if (m_window && m_scopedConfigure) {
        m_scopedConfigure->setNeedsConfigure();
        return;
    }
    if (!m_configureCallback)
        return;

    const base::Point originOffset = m_pendingOriginOffsetAccumulator;
    m_pendingOriginOffsetAccumulator = base::Point();

    ConfigureState state;
    ResizeEdge resizeEdge = ResizeEdge::None;

    if (m_window) {
        state.size = m_window->contentBounds().size();
        m_configuredContentBounds = m_window->contentBounds();
        state.stateType = m_window->presentedStateType();
        state.restoreStateType = m_window->state()->restoreStateType();
        state.resizing = m_window->isResizing();
        state.activated = m_window->isActive();
        state.maximizable = m_window->isResizable();
        state.suspended = m_window->stateType() == WindowStateType::Minimized
                || m_window->isHidden() || m_window->isDeskHidden();
        if (state.resizing && m_manager->eventHandler()->draggedWindow() == m_window)
            resizeEdge = m_manager->eventHandler()->draggedEdge();
    } else {
        state.stateType = m_initialStateType;
        state.size = initialSizeForState(state.stateType);
        state.maximizable = computeResizable();
        // Never suspended before mapping: a suspended client may hold back the
        // first buffer that maps it.
    }
    state.bounds = configureBounds();

    // The callback writes to the client and may flush, which can take the
    // protocol object that owns this one with it. Both the liveness token and
    // the copied callable are needed: the callable's captures live inside the
    // std::function member that would be freed along with this object.
    const std::shared_ptr<ShellSurface *> self = m_self;
    const ConfigureCallback callback = m_configureCallback;
    const uint32_t serial = callback(state);
    if (!*self)
        return;

    if (!serial) {
        m_acknowledgedConfigure = state;
        m_pendingOriginOffset += originOffset;
        m_pendingResizeEdge = resizeEdge;
        return;
    }

    m_pendingConfigs.push_back(Config {serial, originOffset, resizeEdge, state});
    if (m_pendingConfigs.size() > 100) {
        LM_WARNING(lingmoAurora) << "Outstanding xdg configure acknowledgements:"
                                << m_pendingConfigs.size();
    }
}

std::function<void()> ShellSurface::onSurfaceCommitRequested(compositor::Surface *surface)
{
    if (surface != m_surface || m_role == Role::None)
        return {};
    CommittedState state {
        .geometry = m_pendingGeometry,
        .minimumSize = m_pendingMinimumSize,
        .maximumSize = m_pendingMaximumSize,
        .originOffset = std::exchange(m_pendingOriginOffset, base::Point()),
        .resizeEdge = m_pendingResizeEdge,
        .acknowledgedConfigure = m_acknowledgedConfigure,
        .decorationRemoval = std::exchange(m_decorationRemovalPending, false),
        .popupBounds = std::exchange(m_pendingPopupBounds, std::nullopt),
    };
    // The surface drops this when the observer goes away.
    return [this, state = std::move(state)] {
        m_committed = state;
    };
}

void ShellSurface::applyCommittedState(const CommittedState &state)
{
    m_geometry = state.geometry;
    m_minimumSize = state.minimumSize;
    m_maximumSize = state.maximumSize;
    m_originOffset += state.originOffset;
    m_resizeEdge = state.resizeEdge;
    if (state.popupBounds)
        m_popupBounds = *state.popupBounds;

    updateSizeConstraints();
}

void ShellSurface::mapToplevel()
{
    const base::Rect visible = visibleBounds();
    WindowInit init;
    init.requestedSize = visible.size();
    init.title = m_title;
    init.applicationId = m_applicationId;
    init.processId = m_processId;
    init.decorated = decorationMode() == DecorationMode::ServerSide;
    init.minimumSize = m_minimumSize.isEmpty() ? base::Size(1, 1) : m_minimumSize;
    init.maximumSize = m_maximumSize;
    init.transientParent = m_transientParentShell ? m_transientParentShell->window() : nullptr;
    init.positionManaged = isPositionManaged();
    init.modal = m_modal;
    init.output = isPositionManaged() ? m_initialOutput : base::Rect();
    m_window = m_manager->createWindow(m_surface, init);
    if (!m_fullscreenOutputName.empty())
        m_manager->setWindowOutput(m_window, m_fullscreenOutputName);
    // A caption button's close asks the client to close itself, over the same
    // path xdg_toplevel.close takes.
    const std::shared_ptr<ShellSurface *> self = m_self;
    m_window->setCloseCallback([self] {
        if (*self)
            (*self)->close();
    });
    m_window->setPingCallback([self] {
        if (*self && (*self)->m_pingCallback)
            (*self)->m_pingCallback();
    });
    m_window->setKillCallback([self] {
        if (*self && (*self)->m_killCallback)
            (*self)->m_killCallback();
    });
    m_window->addObserver(this);
    updateSizeConstraints();
    updateFromWindowGeometry();

    // Shell metadata may arrive after the initial configure that inherited a state.
    const WindowStateType initialStateType = m_initialStateInherited && !isPositionManaged()
            ? WindowStateType::Normal
            : m_initialStateType;
    switch (initialStateType) {
    case WindowStateType::Maximized:
        m_window->maximize();
        break;
    case WindowStateType::Fullscreen:
        m_window->setFullscreen(true);
        break;
    case WindowStateType::Minimized:
        m_window->minimize();
        break;
    case WindowStateType::Normal:
    case WindowStateType::PrimarySnapped:
    case WindowStateType::SecondarySnapped:
        // A client cannot ask to be mapped snapped; only a drag or the shell
        // puts a window there.
        break;
    }

    // Mapped maximized or fullscreen, the window has no normal bounds to come
    // back to; give it those it would have had mapping normal at its own size.
    const base::Size restoreSize = m_surface->restoreSize();
    if (!restoreSize.isEmpty()
        && (m_window->state()->isMaximized() || m_window->state()->isFullscreen())) {
        base::Rect bounds = Placement().place(normalFrameInsets().outset(restoreSize),
                                         m_manager->workAreaFor(m_window));
        if (isPositionManaged())
            bounds = m_manager->boundsForNewWindow(m_window, bounds);
        m_window->state()->setRestoreBounds(bounds);
    }
    // A position asked for places only the next map.
    m_surface->setRequestedPosition(std::nullopt);

    // The capture interface and pickers are never activated; mapping them
    // takes the keyboard.
    if (m_window->role() == WindowRole::Capture || m_window->role() == WindowRole::Picker)
        m_manager->setKeyboardGrab(m_window);
    else if (!m_window->state()->isMinimized())
        m_manager->activation()->activate(m_window);
    m_configuredContentBounds = m_window->contentBounds();
    updateChildPopupPositions();

    LM_DEBUG(lingmoAurora) << "xdg toplevel mapped:" << m_applicationId << m_title
                         << (m_window->isDecorated() ? "ssd" : "csd") << "outer"
                         << m_window->bounds() << "content"
                         << m_window->contentBounds();
}

void ShellSurface::unmapToplevel()
{
    unmapChildPopups();
    // Before the window goes, so the children's windows move to the new parent.
    releaseTransientChildren();
    LM_DEBUG(lingmoAurora) << "xdg toplevel unmapped:" << m_applicationId << m_title;
    Window *window = m_window;
    m_window = nullptr;
    window->removeObserver(this);
    m_manager->destroyWindow(window);
    resetConfigureState();
}

void ShellSurface::mapPopup()
{
    if (!m_parentShell || !m_parentShell->window()) {
        m_surface->setHidden(true);
        return;
    }
    m_popupMapped = true;
    WindowInit init;
    init.type = WindowType::Popup;
    init.origin = absoluteOrigin();
    init.requestedSize = visibleBounds().size();
    init.transientParent = m_parentShell ? m_parentShell->window() : nullptr;
    m_window = m_manager->createWindow(m_surface, init);
    m_window->addObserver(this);
    updatePopupPosition();

    LM_DEBUG(lingmoAurora) << "xdg popup mapped at" << absoluteOrigin() << "grab" << m_hasGrab;
}

void ShellSurface::unmapChildPopups()
{
    for (ShellSurface *child : std::vector<ShellSurface *>(m_childShells)) {
        if (child->m_popupMapped)
            child->unmapPopup();
    }
}

void ShellSurface::unmapPopup()
{
    unmapChildPopups();
    LM_DEBUG(lingmoAurora) << "xdg popup unmapped";
    m_popupMapped = false;
    if (m_window) {
        Window *window = m_window;
        m_window = nullptr;
        window->removeObserver(this);
        m_manager->destroyWindow(window);
    }
    m_manager->removePopupGrab(this);
    m_hasGrab = false;
}

void ShellSurface::updatePopupPosition()
{
    if (m_role != Role::Popup || !m_surface)
        return;
    // The window geometry's origin is where the popup's logical top-left sits,
    // so shift the buffer by it: a client whose buffer carries a shadow around
    // the menu still lands in the right place.
    if (m_window) {
        m_window->setVisible(m_parentShell && m_parentShell->window()
                             && m_parentShell->window()->targetVisibility());
        m_window->setBounds(base::Rect(absoluteOrigin(), visibleBounds().size()));
        updateFromWindowGeometry();
    }
    updateChildPopupPositions();
    if (m_manager->compositor())
        m_manager->compositor()->scheduleRepaint();
}

void ShellSurface::updateChildPopupPositions()
{
    for (ShellSurface *child : std::vector<ShellSurface *>(m_childShells))
        child->updatePopupPosition();
}

void ShellSurface::updateFromWindowGeometry()
{
    updateSurfaceOrigin();
    updateClientExteriorInsets();
}

void ShellSurface::updateSurfaceOrigin()
{
    if (m_window)
        m_window->setSurfaceOrigin(surfaceOriginForWindow());
}

void ShellSurface::updateClientExteriorInsets()
{
    if (m_window) {
        m_window->setClientExteriorInsets(clientExteriorInsets());
        m_window->setClientInputInsets(clientInputInsets());
    }
}

Insets ShellSurface::clientExteriorInsets() const
{
    // xdg_surface.set_window_geometry says which part of the surface is the
    // window proper; whatever the buffer covers outside it is the client's own
    // exterior - a CSD shadow, an invisible resize border, or both. A client
    // that sets no geometry, or sets it to its whole surface, has none. This is
    // protocol geometry only: no application id, toolkit or pixel is consulted.
    if (m_geometry.isEmpty() || !m_surface)
        return Insets();
    const base::Size content = m_surface->contentSize();
    if (content.isEmpty())
        return Insets();
    if (m_window && m_window->isResizing()) {
        const auto exterior = m_window->clientExteriorInsets();
        // Resize geometry can lead or lag the buffer; retain established CSD margins.
        if (!exterior.topLeft().isOrigin() && m_geometry.topLeft() == exterior.topLeft())
            return exterior;
    }
    // A geometry reaching outside the surface is not exterior decoration, so
    // negative slack is clamped away rather than reported as a client shadow.
    return Insets {std::max(0, m_geometry.left()), std::max(0, m_geometry.top()),
                   std::max(0, content.width() - (m_geometry.left() + m_geometry.width())),
                   std::max(0, content.height() - (m_geometry.top() + m_geometry.height()))};
}

Insets ShellSurface::clientInputInsets() const
{
    // No input region means the whole surface takes input.
    if (!m_surface || !m_surface->inputRegion())
        return Insets();
    const base::Size content = m_surface->contentSize();
    const base::Rect input =
            m_surface->inputRegion()->boundingRect().intersected(base::Rect(base::Point(), content));
    if (content.isEmpty() || input.isEmpty())
        return Insets();
    return Insets {input.left(), input.top(), content.width() - (input.left() + input.width()),
                   content.height() - (input.top() + input.height())};
}

base::Point ShellSurface::surfaceOriginForWindow() const
{
    base::Rect visible = visibleBounds();
    // Anchor the committed body, including CSD margins, while geometry catches up.
    if (m_resizeEdge != ResizeEdge::None && m_surface && !m_surface->contentSize().isEmpty())
        visible.setSize(visible.topLeft().isOrigin()
                                ? m_surface->contentSize()
                                : clientExteriorInsets().inset(m_surface->contentSize()));
    const base::Size windowSize = m_window->contentBounds().size();

    // While a resize anchored to a top or left edge is in flight the window has
    // already grown, but the client's contents have not. Pin them to the corner
    // the drag is not moving so the window does not appear to jump.
    base::Point anchor;
    switch (m_resizeEdge) {
    case ResizeEdge::None:
        anchor = m_originOffset;
        break;
    case ResizeEdge::Bottom:
    case ResizeEdge::Right:
    case ResizeEdge::BottomRight:
        break;
    case ResizeEdge::Top:
    case ResizeEdge::TopRight:
        anchor = base::Point(0, windowSize.height() - visible.height());
        break;
    case ResizeEdge::Left:
    case ResizeEdge::BottomLeft:
        anchor = base::Point(windowSize.width() - visible.width(), 0);
        break;
    case ResizeEdge::TopLeft:
        anchor = base::Point(windowSize.width() - visible.width(),
                        windowSize.height() - visible.height());
        break;
    }
    return anchor - visible.topLeft();
}

void ShellSurface::updateWindowBoundsFromClient()
{
    if (!m_window)
        return;
    // Defer while the client still owes an acknowledgement: its contents
    // describe a size the compositor has already asked it to leave.
    if (!m_pendingConfigs.empty())
        return;
    // An ack followed by a state-only commit still describes the old buffer.
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(m_window->node())) {
        if (animation->isWaitingForBoundsContent())
            return;
    }
    // Maximized, fullscreen and interactively resized bounds belong to the
    // window manager.
    if (!m_window->state()->isNormal() || m_window->isResizing())
        return;

    const base::Size size = visibleBounds().size();
    if (size.isEmpty() || size == m_window->contentBounds().size())
        return;

    base::Rect bounds = m_window->bounds();
    const base::Size oldSize = bounds.size();
    bounds.setSize(m_window->frameInsets().outset(size));
    if (m_surface->resizeAnchor() == compositor::ResizeAnchor::Center) {
        // Halves of the difference rather than moveCenter(): an integer centre
        // would drift a pixel on every odd-sized step.
        bounds.translate((oldSize.width() - bounds.width()) / 2,
                         (oldSize.height() - bounds.height()) / 2);
        const base::Rect workArea = m_manager->workAreaFor(m_window);
        if (!workArea.isEmpty()) {
            bounds.moveLeft(std::max(workArea.left(),
                                     std::min(bounds.left(), workArea.right() - bounds.width())));
            bounds.moveTop(std::max(workArea.top(),
                                    std::min(bounds.top(), workArea.bottom() - bounds.height())));
        }
    }
    m_applyingClientBounds = true;
    m_window->setBounds(bounds);
    m_applyingClientBounds = false;
}

void ShellSurface::updateSizeConstraints()
{
    if (!m_window)
        return;
    m_window->setMinimumSize(m_minimumSize.isEmpty() ? base::Size(1, 1) : m_minimumSize);
    m_window->setMaximumSize(m_maximumSize);
    m_window->setResizable(computeResizable());
    // Only a configure carries a change of wm_capabilities to the client.
    const std::optional<ConfigureState> &last =
            m_pendingConfigs.empty() ? m_acknowledgedConfigure : m_pendingConfigs.back().state;
    if (last && last->maximizable != m_window->isResizable())
        configure();
}

base::Size ShellSurface::configureBounds() const
{
    // Shell surfaces size themselves over the whole output. Qt clamps a
    // client-chosen size to the bounds, which would lift the desktop off the
    // dock's strip.
    if (m_manager->shellMetadata(m_surface) || (m_window && m_window->isShellWindow()))
        return base::Size(0, 0);
    const Insets insets = normalFrameInsets();
    base::Rect workArea = m_window ? m_manager->workAreaFor(m_window)
                              : m_manager->placementWorkAreaForOutput(m_initialOutput);
    if (workArea.isEmpty())
        workArea = m_manager->workAreaFor(nullptr);
    return insets.inset(workArea.size());
}

Insets ShellSurface::normalFrameInsets() const
{
    if (decorationMode() == DecorationMode::ServerSide && m_manager->frameProvider())
        return m_manager->frameProvider()->defaultExtents();
    return Insets();
}

bool ShellSurface::computeResizable() const
{
    // Upstream ShellSurface::GetCanResizeFromSizeConstraints: both defaults are
    // empty, and a window with no constraints must stay resizable.
    return m_minimumSize.isEmpty() || m_minimumSize != m_maximumSize;
}

base::Size ShellSurface::initialSizeForState(WindowStateType stateType) const
{
    // No window yet, so no frame either: ask what one would cost.
    const Insets insets = normalFrameInsets();

    switch (stateType) {
    case WindowStateType::Maximized: {
        base::Rect workArea = m_manager->workAreaForOutput(m_initialOutput);
        if (workArea.isEmpty())
            workArea = m_manager->workAreaFor(nullptr);
        return insets.inset(workArea.size());
    }
    case WindowStateType::Fullscreen:
        return m_manager->outputBoundsFor(m_window).size();
    case WindowStateType::Normal:
    case WindowStateType::Minimized:
    case WindowStateType::PrimarySnapped:
    case WindowStateType::SecondarySnapped:
        // Zero lets the client pick its own size, which is what xdg-shell asks
        // a compositor with no opinion to do.
        return base::Size(0, 0);
    }
    return base::Size(0, 0);
}

void ShellSurface::resetConfigureState()
{
    // A remap starts the configure sequence over from the beginning.
    m_pendingConfigs.clear();
    m_acknowledgedConfigure.reset();
    m_boundsTransitionTarget.reset();
    m_initialConfigureSent = false;
    m_originOffset = base::Point();
    m_pendingOriginOffset = base::Point();
    m_pendingOriginOffsetAccumulator = base::Point();
    m_resizeEdge = ResizeEdge::None;
    m_pendingResizeEdge = ResizeEdge::None;
    m_pendingPopupBounds.reset();
    m_configuredContentBounds = base::Rect();
    m_boundsTransitionOldContentSize = base::Size();
}

bool ShellSurface::hasPopupDescendant(const compositor::Surface *root) const
{
    for (const ShellSurface *child : m_childShells) {
        if (child->rootSurface() == root || child->hasPopupDescendant(root))
            return true;
    }
    return false;
}

void ShellSurface::addChildShell(ShellSurface *child)
{
    if (std::find(m_childShells.begin(), m_childShells.end(), child) == m_childShells.end())
        m_childShells.push_back(child);
}

void ShellSurface::removeChildShell(ShellSurface *child)
{
    m_childShells.erase(std::remove(m_childShells.begin(), m_childShells.end(), child),
                        m_childShells.end());
}

} // namespace Lingmo::WM
