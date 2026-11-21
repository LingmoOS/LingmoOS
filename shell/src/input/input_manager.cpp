// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/extended_mouse_warp_controller.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: project relative pointer motion onto active logical
// rectangles without introducing Ash root windows.

#include "input/input_manager.h"
#include "display/display_manager.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <linux/input-event-codes.h>
#include <utility>

#include "core/logging.h"
#include "input/event_sink.h"
#include "input/pointer_constraints.h"
#include "input/relative_pointer_delegate.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "base/event_loop.h"
#include "scene/node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {

InputManager::InputManager(Seat *seat)
    : m_seat(seat)
{
    m_seat->addObserver(this);
    m_seat->pointerConstraints()->warp = [this](const base::PointF &position) {
        m_pointerPosition = clamp(position);
        dispatchMotion(std::chrono::steady_clock::now());
    };
}

InputManager::~InputManager()
{
    m_seat->pointerConstraints()->warp = nullptr;
    m_seat->removeObserver(this);
}

void InputManager::setBounds(const base::Size &bounds)
{
    m_topology.clear();
    m_bounds = bounds;
    warpPointer(m_pointerPosition);
}

void InputManager::setTopology(const std::vector<base::Rect> &displays, const base::Rect &primary)
{
    m_topology.clear();
    for (const base::Rect &display : displays) {
        if (display.width() > 0 && display.height() > 0)
            m_topology.push_back(display);
    }
    base::Rect unionBounds;
    for (const base::Rect &display : m_topology)
        unionBounds = unionBounds.united(display);
    m_bounds = unionBounds.size();
    if (m_topology.empty()) {
        warpPointer(m_pointerPosition);
        return;
    }
    const std::vector<base::Rect> previous = std::exchange(m_pointerLayout, m_topology);
    if (!m_pointerPlaced) {
        m_pointerPlaced = true;
        const base::Rect start = primary.isEmpty() ? m_topology.front() : primary;
        warpPointer(base::RectF(start).center());
        return;
    }
    // A display that changes scale or mode keeps the pointer at the same spot, as
    // DRM's cursor keeps its native position (ash UpdateMouseLocationAfterDisplayChange).
    if (previous.size() == m_topology.size()) {
        for (size_t index = 0; index < previous.size(); ++index) {
            const base::RectF from = previous[index];
            if (m_pointerPosition.x() < from.left() || m_pointerPosition.y() < from.top()
                || m_pointerPosition.x() >= from.left() + from.width()
                || m_pointerPosition.y() >= from.top() + from.height())
                continue;
            const base::RectF to = m_topology[index];
            warpPointer(base::PointF(
                    to.left() + (m_pointerPosition.x() - from.left()) * to.width() / from.width(),
                    to.top() + (m_pointerPosition.y() - from.top()) * to.height() / from.height()));
            return;
        }
    }
    warpPointer(m_pointerPosition);
}

base::PointF InputManager::clamp(const base::PointF &position) const
{
    if (!m_topology.empty()) {
        for (const base::Rect &display : m_topology) {
            if (position.x() >= display.x() && position.y() >= display.y()
                && position.x() < display.x() + display.width()
                && position.y() < display.y() + display.height()) {
                return position;
            }
        }

        base::PointF nearest;
        double nearestDistance = std::numeric_limits<double>::max();
        for (const base::Rect &display : m_topology) {
            const base::PointF candidate(std::clamp(position.x(), double(display.x()),
                                               double(display.x() + display.width() - 1)),
                                    std::clamp(position.y(), double(display.y()),
                                               double(display.y() + display.height() - 1)));
            const base::PointF delta = position - candidate;
            const double distance = delta.x() * delta.x() + delta.y() * delta.y();
            if (distance < nearestDistance) {
                nearest = candidate;
                nearestDistance = distance;
            }
        }
        return nearest;
    }
    if (m_bounds.isEmpty())
        return position;

    if (!m_deviceBounds.isEmpty())
        return base::PointF(
                std::clamp(position.x(), 0.0, (m_deviceBounds.width() - 1) / m_deviceScale),
                std::clamp(position.y(), 0.0, (m_deviceBounds.height() - 1) / m_deviceScale));
    // The pointer may sit on the last pixel but not past it.
    return base::PointF(std::clamp(position.x(), 0.0, static_cast<double>(m_bounds.width() - 1)),
                   std::clamp(position.y(), 0.0, static_cast<double>(m_bounds.height() - 1)));
}

void InputManager::warpPointer(const base::PointF &position)
{
    m_pointerPosition = clamp(position);
    m_seat->updateCursorPosition(m_pointerPosition);
    updateFallbackCursor();
}

void InputManager::synthesizeMotion()
{
    if (std::exchange(m_synthesizePending, true))
        return;
    base::EventLoop::current()->post([weak = m_weakFactory.getWeakPtr()] {
        InputManager *self = weak.get();
        if (!self)
            return;
        self->synthesizePendingMotion();
    });
}

void InputManager::synthesizePendingMotion()
{
    m_synthesizePending = false;
    // A held button or a drag keeps the surface it started on, and a
    // cursor hidden by touch hovers nothing.
    if (m_seat->wasShutdown() || m_seat->pressedButtons() != NoButton
        || m_seat->drag()->grabsInput() || m_seat->cursorHiddenByTouch())
        return;
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput())
        return;
    dispatchMotion(std::chrono::steady_clock::now());
}

void InputManager::setFallbackCursor(scene::Node *node, const base::Point &hotspot)
{
    m_fallbackCursor = node;
    m_fallbackCursorHotspot = hotspot;
    updateFallbackCursor();
}

void InputManager::updateFallbackCursor()
{
    if (!m_fallbackCursor)
        return;

    m_fallbackCursor->setPosition(
            base::Point(static_cast<int>(std::lround(m_pointerPosition.x())),
                        static_cast<int>(std::lround(m_pointerPosition.y())))
            - m_fallbackCursorHotspot);
    const bool visible = !m_seat->hasClientCursorRequest() && !m_seat->cursorHiddenByTouch();
    if (m_fallbackCursor->isVisible() != visible)
        LM_DEBUG(lingmoCursorTransitions) << "fallback visible" << visible;
    m_fallbackCursor->setVisible(visible);
}

void InputManager::dispatchMotion(Timestamp timestamp, const RelativeMotion *relative)
{
    m_seat->setCursorHiddenByTouch(false);
    m_seat->updateCursorPosition(m_pointerPosition);
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        if (auto *sink = gate->secureInput())
            sink->onPointerMotion(timestamp, m_pointerPosition);
        updateFallbackCursor();
        return;
    }
    pointerMoved(m_pointerPosition);
    // The cursor tracks the physical pointer even while the sink consumes the
    // event; only client dispatch is suppressed.
    m_seat->beginPointerMotion();
    const bool consumed = !m_seat->drag()->grabsInput() && m_eventSink
            && m_eventSink->onPointerMotion(timestamp, m_pointerPosition);
    if (!consumed)
        m_seat->onPointerMotion(timestamp, m_pointerPosition, relative);
    m_seat->endPointerMotion();
    updateFallbackCursor();
    if (consumed != m_motionConsumed) {
        m_motionConsumed = consumed;
        m_seat->traceCursors(consumed ? "motion-owner-sink" : "motion-owner-client");
    }
}

void InputManager::movePointer(Timestamp timestamp, base::PointF target, const RelativeMotion &relative)
{
    const PointerConstraints *constraints = m_seat->pointerConstraints();
    if (constraints->isLocked())
        target = m_pointerPosition;
    else
        target = constraints->confine(m_pointerPosition, target);
    m_pointerPosition = clamp(target);
    dispatchMotion(timestamp, &relative);
}

void InputManager::onPointerMotion(Timestamp timestamp, const base::PointF &delta,
                                   const base::PointF &unaccelerated)
{
    userActivity();
    const base::PointF logicalDelta = m_topology.empty() ? delta / m_deviceScale : delta;
    movePointer(timestamp, m_pointerPosition + logicalDelta, {logicalDelta, unaccelerated});
}

void InputManager::moveAbsolute(Timestamp timestamp, const base::PointF &target)
{
    // Exo derives relative motion from the location, which is all an absolute
    // device has.
    const base::PointF delta = m_absolutePosition ? target - *m_absolutePosition : base::PointF();
    m_absolutePosition = target;
    movePointer(timestamp, target, {delta, delta});
}

void InputManager::onPointerMotionAbsolute(Timestamp timestamp, const base::PointF &position)
{
    userActivity();
    moveAbsolute(timestamp, position / m_deviceScale);
}

void InputManager::onPointerMotionLogical(Timestamp timestamp, const base::PointF &position)
{
    userActivity();
    const base::PointF delta = position - m_pointerPosition;
    movePointer(timestamp, position, {delta, delta});
}

void InputManager::onPointerMotionNormalized(Timestamp timestamp, const base::PointF &position)
{
    userActivity();
    if (m_displayManager) {
        if (m_displayManager->isPublishing() || m_displayManager->desktopBounds().isEmpty())
            return;
        moveAbsolute(timestamp, m_displayManager->normalizedToGlobal(position));
        return;
    }
    base::Rect desktop(base::Point(), m_bounds);
    if (!m_topology.empty()) {
        desktop = {};
        for (const auto &display : m_topology)
            desktop = desktop.united(display);
    }
    if (desktop.isEmpty())
        return;
    // Absolute devices report a fraction of the current logical desktop, not the startup mode.
    moveAbsolute(timestamp, base::PointF(desktop.x() + position.x() * desktop.width(),
                                    desktop.y() + position.y() * desktop.height()));
}

void InputManager::onPointerButton(Timestamp timestamp, uint32_t evdevButton, bool pressed)
{
    userActivity();
    if (pressed)
        m_heldButtons.insert(evdevButton);
    else
        m_heldButtons.erase(evdevButton);
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        if (auto *sink = gate->secureInput())
            sink->onPointerButton(timestamp, m_pointerPosition, evdevButton, pressed);
        // A button held across the lock still ends in the translator and the
        // seat, as a key does: a Control+click would stay a secondary click.
        if (!pressed) {
            const uint32_t translated =
                    m_seat->keyTranslator()->translateButton(evdevButton, false);
            m_seat->updatePointerButtonState(translated, false);
            m_seat->endPointerGrabIfReleased();
        }
        return;
    }
    if (pressed && m_displayManager && m_displayManager->desktopBounds().isEmpty())
        return;
    evdevButton = m_seat->keyTranslator()->translateButton(evdevButton, pressed);
    // A click or a wheel brings the cursor back as a move does; the event
    // itself enters the surface under it again.
    if (pressed) {
        m_seat->setCursorHiddenByTouch(false);
        updateFallbackCursor();
    }
    m_seat->updateCursorPosition(m_pointerPosition);
    // WM owns drag completion; the client still needs the release paired with
    // its press. Do not hit-test a new recipient for a consumed release.
    const auto before = m_seat->pressedButtons();
    // Drag completion callbacks may re-enter request validation.
    if (!pressed)
        m_seat->updatePointerButtonState(evdevButton, false);
    const bool consumed = !m_seat->drag()->grabsInput() && m_eventSink
            && m_eventSink->onPointerButton(timestamp, m_pointerPosition, evdevButton, pressed);
    if (!consumed)
        m_seat->onPointerButton(timestamp, m_pointerPosition, evdevButton, pressed);
    else if (!pressed)
        m_seat->releasePointerButton(timestamp, evdevButton);
    LM_DEBUG(lingmoInputTransitions) << "input code" << evdevButton << (pressed ? "press" : "release")
                                << "position" << m_pointerPosition << "seat-before" << before
                                << "seat-after" << m_seat->pressedButtons() << "consumer"
                                << (consumed ? "event-sink" : "seat/client");
    m_seat->traceCursors(pressed ? "button-press" : "button-release");
    // A click is where a client usually claims the pointer image.
    updateFallbackCursor();
}

std::optional<base::PointF> InputManager::touchLocation(const base::PointF &normalized) const
{
    if (m_displayManager) {
        if (m_displayManager->isPublishing())
            return std::nullopt;
        return m_displayManager->touchscreenToGlobal(normalized);
    }
    // Without display configuration the panel spans the desktop.
    base::Rect desktop(base::Point(), m_bounds);
    for (const base::Rect &display : m_topology)
        desktop = desktop.isEmpty() ? display : desktop.united(display);
    if (desktop.isEmpty())
        return std::nullopt;
    return base::PointF(desktop.x() + normalized.x() * desktop.width(),
                   desktop.y() + normalized.y() * desktop.height());
}

void InputManager::onTouchDown(Timestamp timestamp, int32_t id, const base::PointF &normalized)
{
    userActivity();
    const auto location = touchLocation(normalized);
    if (!location)
        return;
    m_touchLocations[id] = *location;
    // Not while the mouse holds a button, as aura's CompoundEventFilter: the
    // surface it pressed on keeps the pointer.
    if (m_seat->pressedButtons() == NoButton && !m_seat->drag()->grabsInput())
        m_seat->setCursorHiddenByTouch(true);
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        // Whatever a client had of the session's touches ends with it.
        m_seat->onTouchCancel();
        auto *sink = gate->secureInput();
        if (!sink || m_secureTouch)
            return;
        m_secureTouch = id;
        sink->onPointerMotion(timestamp, *location);
        sink->onPointerButton(timestamp, *location, BTN_LEFT, true);
        return;
    }
    if (m_eventSink && !m_seat->drag()->grabsInput()
        && m_eventSink->onTouchDown(timestamp, id, *location)) {
        m_consumedTouches.insert(id);
        return;
    }
    m_seat->onTouchDown(timestamp, id, *location);
}

void InputManager::onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF &normalized)
{
    const auto location = touchLocation(normalized);
    if (!location || !m_touchLocations.contains(id))
        return;
    m_touchLocations[id] = *location;
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        m_seat->onTouchCancel();
        if (auto *sink = gate->secureInput(); sink && m_secureTouch == id)
            sink->onPointerMotion(timestamp, *location);
        return;
    }
    if (m_consumedTouches.contains(id)) {
        if (m_eventSink)
            m_eventSink->onTouchMotion(timestamp, id, *location);
        return;
    }
    if (m_eventSink && m_eventSink->onTouchMotion(timestamp, id, *location)) {
        m_consumedTouches.insert(id);
        return;
    }
    m_seat->onTouchMotion(timestamp, id, *location);
}

void InputManager::onTouchUp(Timestamp timestamp, int32_t id)
{
    const auto it = m_touchLocations.find(id);
    if (it == m_touchLocations.end())
        return;
    const base::PointF location = it->second;
    m_touchLocations.erase(it);
    if (m_secureTouch == id) {
        m_secureTouch.reset();
        if (auto *gate = m_seat->sessionGate(); gate && gate->secureInput())
            gate->secureInput()->onPointerButton(timestamp, location, BTN_LEFT, false);
        return;
    }
    if (m_consumedTouches.erase(id)) {
        if (m_eventSink)
            m_eventSink->onTouchUp(timestamp, id);
        return;
    }
    if (m_eventSink && m_eventSink->onTouchUp(timestamp, id))
        return;
    m_seat->onTouchUp(timestamp, id);
}

void InputManager::onTouchCancel(Timestamp timestamp)
{
    if (m_secureTouch) {
        const base::PointF location = m_touchLocations[*m_secureTouch];
        m_secureTouch.reset();
        if (auto *gate = m_seat->sessionGate(); gate && gate->secureInput())
            gate->secureInput()->onPointerButton(timestamp, location, BTN_LEFT, false);
    }
    // The sink may hold a point it has not been handed an event of yet.
    if (m_eventSink)
        m_eventSink->onTouchCancel(timestamp);
    m_consumedTouches.clear();
    m_touchLocations.clear();
    m_seat->onTouchCancel();
}

void InputManager::onTouchFrame(Timestamp)
{
    m_seat->onTouchFrame();
}

void InputManager::onPointerAxis(Timestamp timestamp, const base::PointF &offset, bool discrete)
{
    userActivity();
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        if (auto *sink = gate->secureInput())
            sink->onPointerAxis(timestamp, m_pointerPosition, offset, discrete);
        return;
    }
    if (m_seat->drag()->grabsInput())
        return;
    m_seat->setCursorHiddenByTouch(false);
    updateFallbackCursor();
    if (!(m_eventSink
          && m_eventSink->onPointerAxis(timestamp, m_pointerPosition, offset, discrete)))
        m_seat->onPointerAxis(timestamp, m_pointerPosition, offset, discrete);
}

void InputManager::onPointerAxisStop(Timestamp timestamp)
{
    userActivity();
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        if (auto *sink = gate->secureInput())
            sink->onPointerAxisStop(timestamp, m_pointerPosition);
        return;
    }
    if (m_seat->drag()->grabsInput())
        return;
    if (!(m_eventSink && m_eventSink->onPointerAxisStop(timestamp, m_pointerPosition)))
        m_seat->onPointerAxisStop(timestamp, m_pointerPosition);
}

void InputManager::onPointerGestureBegin(Timestamp timestamp, GestureType type, uint32_t fingers)
{
    userActivity();
    // Only the begin is gated: a gesture that began is ended by its own pointer
    // when focus leaves its surface, lock screen included.
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput())
        return;
    m_gestureCancelled = false;
    m_gestureClaimed = m_gestureFilter && m_gestureFilter->claimGesture(timestamp, type, fingers);
    if (m_gestureClaimed)
        return;
    m_seat->onPointerGestureBegin(timestamp, m_pointerPosition, type, fingers);
}

void InputManager::onPointerGestureUpdate(Timestamp timestamp, GestureType type,
                                          const base::PointF &delta, const base::PointF &unacceleratedDelta,
                                          double scale, double rotation)
{
    userActivity();
    const base::PointF logicalDelta = m_topology.empty() ? delta / m_deviceScale : delta;
    const GestureUpdate update {logicalDelta, unacceleratedDelta, scale, rotation};
    if (m_gestureClaimed) {
        if (m_gestureCancelled) return;
        if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
            m_gestureCancelled = true;
            if (m_gestureFilter) m_gestureFilter->onGestureEnd(timestamp, true);
            return;
        }
        if (m_gestureFilter)
            m_gestureFilter->onGestureUpdate(timestamp, update);
        return;
    }
    m_seat->onPointerGestureUpdate(timestamp, type, update);
}

void InputManager::onPointerGestureEnd(Timestamp timestamp, GestureType type, bool cancelled)
{
    userActivity();
    if (std::exchange(m_gestureClaimed, false)) {
        if (std::exchange(m_gestureCancelled, false)) return;
        if (m_gestureFilter)
            m_gestureFilter->onGestureEnd(timestamp, cancelled);
        return;
    }
    m_seat->onPointerGestureEnd(timestamp, type, cancelled);
}

uint32_t InputManager::positionalKey(uint32_t evdevCode, KeyboardModifierLayout layout) const
{
    // The PC convention is plain Linux: every keyboard as it reports itself.
    if (m_seat->shortcutConvention() != ShortcutConvention::Command
        || layout == KeyboardModifierLayout::Apple)
        return evdevCode;
    switch (evdevCode) {
    case KEY_LEFTALT:
        return KEY_LEFTMETA;
    case KEY_LEFTMETA:
        return KEY_LEFTALT;
    case KEY_RIGHTALT:
    case KEY_RIGHTMETA:
        // Right Alt stays where it is while the layout types with it (AltGr).
        if (m_seat->xkbTracker()->baseKeysym(KEY_RIGHTALT) == XKB_KEY_ISO_Level3_Shift)
            return evdevCode;
        return evdevCode == KEY_RIGHTALT ? KEY_RIGHTMETA : KEY_RIGHTALT;
    default:
        return evdevCode;
    }
}

void InputManager::onKey(Timestamp timestamp, uint32_t physicalCode, bool pressed,
                         KeyboardModifierLayout layout)
{
    userActivity();
    uint32_t evdevCode = physicalCode;
    if (pressed) {
        evdevCode = m_positionalKeys.try_emplace(physicalCode, positionalKey(physicalCode, layout))
                            .first->second;
    } else if (auto it = m_positionalKeys.find(physicalCode); it != m_positionalKeys.end()) {
        evdevCode = it->second;
        m_positionalKeys.erase(it);
    }
    const bool suppressed = m_seat->keySuppressed(evdevCode);
    if (!m_seat->updateKeyState(evdevCode, pressed))
        return;
    if (auto *gate = m_seat->sessionGate(); gate && gate->restrictsInput()) {
        if ((!suppressed || m_secureKeys.contains(evdevCode))
            && !(m_eventSink && m_eventSink->onSecureKey(timestamp, evdevCode, pressed))
            && gate->secureInput()) {
            gate->secureInput()->onKey(timestamp, evdevCode, pressed);
        }
        if (pressed) {
            m_secureKeys.insert(evdevCode);
            m_seat->suppressKey(evdevCode);
        } else {
            m_secureKeys.erase(evdevCode);
            // A key held across the lock still ends in the translator, or it
            // stays held there: a Meta held for the lock shortcut would reach
            // every client with the next key. Keys pressed while locked are
            // suppressed and never reached it.
            if (!suppressed)
                m_seat->dispatchKey(timestamp, evdevCode, false);
        }
        m_seat->syncKeyboardLocks();
        return;
    }
    m_secureKeys.erase(evdevCode);
    if (m_seat->drag()->key(evdevCode, pressed)) {
        if (pressed)
            m_seat->suppressKey(evdevCode);
        else if (!suppressed)
            m_seat->dispatchKey(timestamp, evdevCode, false);
        m_seat->syncKeyboardLocks();
        return;
    }
    if (m_eventSink && m_eventSink->onKey(timestamp, evdevCode, pressed)) {
        if (pressed)
            m_seat->suppressKey(evdevCode);
        // Exo releases every key the client saw pressed, whatever handled the
        // release: a press without its release would leave the key stuck.
        else if (!suppressed)
            m_seat->dispatchKey(timestamp, evdevCode, false);
        m_seat->syncKeyboardLocks();
        return;
    }
    if (!suppressed)
        m_seat->dispatchKey(timestamp, evdevCode, pressed);
}

} // namespace Lingmo::Input
