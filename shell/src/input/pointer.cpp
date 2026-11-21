// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/pointer.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "input/pointer.h"

#include <cmath>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "input/hit_test.h"
#include "input/pointer_delegate.h"
#include "input/relative_pointer_delegate.h"
#include "input/seat.h"
#include "scene/node.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {

namespace {

// Exo's kLocatedEventEpsilonSquared, in the same units: locations closer than
// this are the same location as far as the client is concerned.
constexpr double kLocatedEventEpsilonSquared = 1.0 / (2000.0 * 2000.0);

bool sameLocation(const base::PointF &a, const base::PointF &b)
{
    const base::PointF offset = a - b;
    const double lengthSquared = offset.x() * offset.x() + offset.y() * offset.y();
    return lengthSquared < (2 * kLocatedEventEpsilonSquared);
}

} // namespace

Pointer::Pointer(PointerDelegate *delegate, Seat *seat)
    : m_delegate(delegate)
    , m_seat(seat)
{
    m_seat->addPointer(this);
}

Pointer::~Pointer()
{
    *m_alive = false;
    m_seat->removePointer(this);
    updatePointerSurface(nullptr);
    m_seat->notifyCursorChanged();

    for (RelativePointerDelegate *delegate : std::exchange(m_relativeDelegates, {}))
        delegate->onPointerDestroying(this);
    for (PointerGestureDelegate *delegate : std::exchange(m_gestureDelegates, {}))
        delegate->onPointerDestroying(this);

    if (m_delegate) {
        PointerDelegate *delegate = m_delegate;
        // The delegate may delete itself here, as the Wayland one does.
        m_delegate = nullptr;
        delegate->onPointerDestroying(this);
    }
    if (m_focusSurface)
        m_focusSurface->removeSurfaceObserver(this);
}

void Pointer::addRelativePointerDelegate(RelativePointerDelegate *delegate)
{
    m_relativeDelegates.push_back(delegate);
}

void Pointer::removeRelativePointerDelegate(RelativePointerDelegate *delegate)
{
    std::erase(m_relativeDelegates, delegate);
}

void Pointer::addGestureDelegate(PointerGestureDelegate *delegate)
{
    m_gestureDelegates.push_back(delegate);
}

void Pointer::removeGestureDelegate(PointerGestureDelegate *delegate)
{
    std::erase(m_gestureDelegates, delegate);
}

void Pointer::setCursor(compositor::Surface *surface, const base::Point &hotspot)
{
    LM_DEBUG(lingmoCursorTransitions) << "set-cursor" << this << "requested" << surface
                                 << "explicit-hide" << (surface == nullptr) << "hotspot"
                                 << hotspot
                                 << "focus" << m_focusSurface;
    // The protocol only defines set_cursor while the pointer has focus.
    if (!m_focusSurface)
        return;

    bool cursorChanged = !m_hasCursorRequest || m_cursorShape.has_value();

    if (surface != m_cursorSurface) {
        if (surface && surface->hasRole()) {
            LM_WARNING(lingmoInput) << "wl_pointer.set_cursor: surface already has a role";
            return;
        }
        updatePointerSurface(surface);
        cursorChanged = true;
    }

    if (hotspot != m_hotspot) {
        m_hotspot = hotspot;
        cursorChanged = true;
    }

    m_cursorShape.reset();
    m_hasCursorRequest = true;
    if (cursorChanged)
        updateCursor();
    traceCursor("set-cursor-result");
    if (cursorChanged)
        m_seat->notifyCursorChanged();
}

// Adapted from Exo Pointer::SetCursorType: replace any surface cursor and
// ignore requests outside pointer focus. Protocol serial validation is external.
void Pointer::setCursorShape(CursorShape shape)
{
    if (!m_focusSurface || (m_hasCursorRequest && m_cursorShape == shape))
        return;
    updatePointerSurface(nullptr);
    m_cursorShape = shape;
    m_hasCursorRequest = true;
    m_seat->notifyCursorChanged();
}

void Pointer::updatePointerSurface(compositor::Surface *surface)
{
    if (m_cursorSurface) {
        compositor::Surface *old = m_cursorSurface;
        // Detached rather than returned to the surface container: a surface
        // that was once a cursor is never a window.
        m_cursorSurface = nullptr;
        old->reparentNode(nullptr);
        old->setHasRole(false);
        maybeRemoveSurfaceObserver(old);
    }

    if (surface) {
        surface->setHasRole(true);
        surface->addSurfaceObserver(this);
        surface->reparentNode(m_seat->cursorContainer());
        surface->setHidden(false);
        m_cursorSurface = surface;
    }
}

void Pointer::updateCursor()
{
    if (!m_cursorSurface)
        return;

    // The hotspot is the point of the cursor image that sits under the pointer.
    const base::PointF topLeft = m_cursorPosition - base::PointF(m_hotspot);
    const base::Point position(static_cast<int>(std::lround(topLeft.x())),
                          static_cast<int>(std::lround(topLeft.y())));
    if (m_cursorSurface->position() == position)
        return;
    m_cursorSurface->setPosition(position);
    if (compositor::Compositor *compositor = m_seat->compositor())
        compositor->scheduleRepaint();
}

void Pointer::setCursorPosition(const base::PointF &position)
{
    m_cursorPosition = position;
    updateCursor();
}

bool Pointer::shouldObserveSurface(compositor::Surface *surface) const
{
    return surface && (surface == m_focusSurface || surface == m_cursorSurface);
}

void Pointer::maybeRemoveSurfaceObserver(compositor::Surface *surface)
{
    if (!shouldObserveSurface(surface))
        surface->removeSurfaceObserver(this);
}

compositor::Surface *Pointer::effectiveTargetForLocation(const base::PointF &rootLocation,
                                                         base::PointF *locationInTarget) const
{
    // A press holds the pointer for the surface it was delivered to, so motion
    // that leaves the surface still reports locations in that surface - which
    // is how a drag started inside a client keeps working outside it.
    if (compositor::Surface *grab = m_seat->pointerGrabSurface()) {
        if (!grab->isMapped() || !m_delegate || !m_delegate->canAcceptPointerEventsForSurface(grab)
            || !m_seat->allowsFocus(grab)) {
            return nullptr;
        }
        *locationInTarget = locationInSurface(grab, rootLocation);
        return grab;
    }

    // The search itself is confined, not just its result: a blocked session
    // never even walks the session containers.
    scene::Node *root = inputRootFor(m_seat->compositor(), m_seat->sessionGate());
    compositor::Surface *target = m_seat->pointerTargetAt(root, rootLocation, locationInTarget);
    if (!target || !m_delegate || !m_delegate->canAcceptPointerEventsForSurface(target)
        || !m_seat->allowsFocus(target)) {
        return nullptr;
    }
    return target;
}

void Pointer::setFocus(compositor::Surface *surface, const base::PointF &rootLocation,
                       const base::PointF &surfaceLocation)
{
    LM_DEBUG(lingmoCursorTransitions) << "focus" << this << "old" << m_focusSurface << "new" << surface;
    cancelGesture();
    const bool cursorOwned = m_hasCursorRequest;
    m_deliveredButtons = NoButton;
    // First a leave for the surface losing focus.
    if (m_focusSurface) {
        if (m_delegate) {
            m_delegate->onPointerLeave(m_focusSurface);
            m_delegate->onPointerFrame();
        }
        // The cursor is per-focus: the client must call set_cursor again in
        // response to the next enter.
        updatePointerSurface(nullptr);
        m_hasCursorRequest = false;
        m_cursorShape.reset();

        compositor::Surface *old = m_focusSurface;
        m_focusSurface = nullptr;
        maybeRemoveSurfaceObserver(old);
    }

    // Then an enter for the surface gaining it.
    if (surface) {
        if (m_delegate) {
            m_delegate->onPointerEnter(surface, surfaceLocation, m_seat->pressedButtons());
            m_delegate->onPointerFrame();
        }
        m_cursorPosition = rootLocation;
        m_locationInRoot = rootLocation;
        m_locationInSurface = surfaceLocation;
        m_hasLocation = true;
        m_focusSurface = surface;
        m_focusSurface->addSurfaceObserver(this);
    }
    if (cursorOwned)
        m_seat->notifyCursorChanged();
}

void Pointer::clearFocus()
{
    if (m_focusSurface)
        setFocus(nullptr, m_locationInRoot, base::PointF());
}

compositor::Surface *Pointer::updateFocus(const base::PointF &rootLocation,
                                          base::PointF *locationInTarget)
{
    *locationInTarget = base::PointF();
    compositor::Surface *target = effectiveTargetForLocation(rootLocation, locationInTarget);
    if (target != m_focusSurface)
        setFocus(target, rootLocation, *locationInTarget);
    return m_focusSurface;
}

bool Pointer::checkIfSameLocation(const base::PointF &locationInRoot,
                                  const base::PointF &locationInTarget) const
{
    // A surface can move under a stationary pointer, so the surface-local
    // location has to be compared as well as the root one.
    if (!m_hasLocation)
        return false;
    return sameLocation(locationInRoot, m_locationInRoot)
            && sameLocation(locationInTarget, m_locationInSurface);
}

void Pointer::onPointerMotion(Timestamp timestamp, const base::PointF &rootLocation,
                              const RelativeMotion *relative)
{
    if (m_seat->wasShutdown())
        return;

    base::PointF locationInTarget;
    compositor::Surface *focus = updateFocus(rootLocation, &locationInTarget);

    // The enter event already carried this location, so an immediately
    // following motion for the same point is suppressed. Sampled before
    // m_locationInRoot is advanced.
    const bool sameLocation = checkIfSameLocation(rootLocation, locationInTarget);

    // The cursor follows the physical pointer even while it is over nothing
    // this client owns; only ownership of the image is tied to focus.
    m_locationInRoot = rootLocation;
    setCursorPosition(rootLocation);

    if (!focus || !m_delegate)
        return;

    // Relative motion goes on at the edge of the screen and under a lock,
    // where the location does not change.
    bool needsFrame = false;
    if (relative && !(relative->delta.isOrigin() && relative->unaccelerated.isOrigin())) {
        for (RelativePointerDelegate *delegate : m_relativeDelegates) {
            delegate->onPointerRelativeMotion(timestamp, *relative);
            needsFrame = true;
        }
    }
    if (!sameLocation) {
        m_delegate->onPointerMotion(timestamp, locationInTarget);
        m_locationInSurface = locationInTarget;
        m_hasLocation = true;
        needsFrame = true;
    }
    if (needsFrame)
        m_delegate->onPointerFrame();
}

void Pointer::onPointerButton(Timestamp timestamp, const base::PointF &rootLocation,
                              ButtonFlags changedButtonFlags, bool pressed)
{
    if (m_seat->wasShutdown())
        return;

    base::PointF locationInTarget;
    compositor::Surface *focus = updateFocus(rootLocation, &locationInTarget);
    if (!focus || !m_delegate)
        return;

    if (pressed)
        m_deliveredButtons |= changedButtonFlags;
    else
        m_deliveredButtons &= ~changedButtonFlags;
    m_delegate->onPointerButton(timestamp, changedButtonFlags, pressed);
    m_delegate->onPointerFrame();
}

void Pointer::releasePressedButtons(Timestamp timestamp, ButtonFlags buttons)
{
    const auto released = m_deliveredButtons & buttons;
    m_deliveredButtons &= ~released;
    if (!released || !m_focusSurface || !m_delegate)
        return;
    const auto alive = m_alive;
    auto *delegate = m_delegate;
    delegate->onPointerButton(timestamp, released, false);
    if (*alive)
        delegate->onPointerFrame();
}

void Pointer::onPointerAxis(Timestamp timestamp, const base::PointF &rootLocation, const base::PointF &offset,
                            bool discrete)
{
    if (m_seat->wasShutdown())
        return;

    base::PointF locationInTarget;
    compositor::Surface *focus = updateFocus(rootLocation, &locationInTarget);
    if (!focus || !m_delegate)
        return;

    m_delegate->onPointerScroll(timestamp, offset, discrete);
    m_delegate->onPointerFrame();
}

void Pointer::onPointerAxisStop(Timestamp timestamp, const base::PointF &rootLocation)
{
    if (m_seat->wasShutdown())
        return;

    base::PointF locationInTarget;
    compositor::Surface *focus = updateFocus(rootLocation, &locationInTarget);
    if (!focus || !m_delegate)
        return;

    m_delegate->onFingerScrollStop(timestamp);
    m_delegate->onPointerFrame();
}

void Pointer::onPointerGestureBegin(Timestamp timestamp, const base::PointF &rootLocation,
                                    GestureType type, uint32_t fingers)
{
    if (m_seat->wasShutdown())
        return;

    cancelGesture();
    base::PointF locationInTarget;
    compositor::Surface *focus = updateFocus(rootLocation, &locationInTarget);
    if (!focus || !m_delegate)
        return;

    m_gesture = type;
    for (PointerGestureDelegate *delegate : m_gestureDelegates) {
        if (delegate->gestureType() == type)
            delegate->onPointerGestureBegin(timestamp, focus, fingers);
    }
    m_delegate->onPointerFrame();
}

void Pointer::onPointerGestureUpdate(Timestamp timestamp, GestureType type,
                                     const GestureUpdate &update)
{
    if (m_gesture != type || !m_delegate)
        return;

    for (PointerGestureDelegate *delegate : m_gestureDelegates) {
        if (delegate->gestureType() == type)
            delegate->onPointerGestureUpdate(timestamp, update);
    }
    m_delegate->onPointerFrame();
}

void Pointer::onPointerGestureEnd(Timestamp timestamp, GestureType type, bool cancelled)
{
    if (m_gesture != type || !m_delegate)
        return;

    m_gesture.reset();
    for (PointerGestureDelegate *delegate : m_gestureDelegates) {
        if (delegate->gestureType() == type)
            delegate->onPointerGestureEnd(timestamp, cancelled);
    }
    m_delegate->onPointerFrame();
}

void Pointer::cancelGesture()
{
    if (m_gesture)
        onPointerGestureEnd(std::chrono::steady_clock::now(), *m_gesture, true);
}

void Pointer::onSurfaceDestroying(compositor::Surface *surface)
{
    traceCursor("surface-destroying");
    const bool affected = surface == m_cursorSurface || surface == m_focusSurface;
    // The dying surface removes its observers. Clear discoverable cursor state
    // before notifying presentation observers, which may re-enter input code.
    if (surface == m_cursorSurface) {
        m_cursorSurface = nullptr;
        m_hasCursorRequest = false;
        m_cursorShape.reset();
    }

    if (surface == m_focusSurface) {
        cancelGesture();
        m_focusSurface = nullptr;
        m_deliveredButtons = NoButton;
        m_hasLocation = false;
        updatePointerSurface(nullptr);
        m_hasCursorRequest = false;
        m_cursorShape.reset();
        if (m_delegate) {
            m_delegate->onPointerLeave(surface);
            m_delegate->onPointerFrame();
        }
    }
    if (affected)
        m_seat->notifyCursorChanged();
}

void Pointer::traceCursor(const char *transition) const
{
    if (!lingmoCursorTransitions.isEnabled(base::LogLevel::Debug))
        return;
    auto *node = m_cursorSurface ? m_cursorSurface->node() : nullptr;
    LM_DEBUG(lingmoCursorTransitions) << transition << this << "focus" << m_focusSurface << "cursor"
                                 << m_cursorSurface << "request-owned" << m_hasCursorRequest
                                 << "contents"
                                 << (m_cursorSurface && m_cursorSurface->hasContents()) << "buffer"
                                 << (m_cursorSurface ? m_cursorSurface->buffer().get() : nullptr)
                                 << "mapped" << (m_cursorSurface && m_cursorSurface->isMapped())
                                 << "node" << node << "visible"
                                 << (node && node->isEffectivelyVisible()) << "parent"
                                 << (node ? node->parent() : nullptr) << "cursor-layer"
                                 << m_seat->cursorContainer() << "texture"
                                 << (node ? node->texture() : nullptr) << "geometry"
                                 << (node ? node->absoluteGeometry() : base::Rect()) << "hotspot"
                                 << m_hotspot;
}

void Pointer::onSurfaceCommit(compositor::Surface *surface)
{
    if (surface == m_cursorSurface)
        traceCursor("cursor-commit");
}

} // namespace Lingmo::Input
